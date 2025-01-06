/**
 * @brief Smart Home System
*/
//C/C++: Disable Error Squiggles
//C/C++: Enable Error Squiggles

#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"
#include "misc.h"

#include "core_cm3.h"
#include "stm32f10x_adc.h"
#include "lcd.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "touch.h"
#include "stdint.h"

void RCC_Configure(void);
void GPIO_Configure(void);
void EXTI_Configure(void);
void USART1_Init(void);
void NVIC_Configure(void);
void EXTI15_10_IRQHandler(void);
void EXTI4_IRQHandler(void);
void Delay(void);
void sendDataUART1(uint16_t data);
void MotorOPChange(int num);

int color[12] = {WHITE,CYAN,BLUE,RED,MAGENTA,LGRAY,GREEN,YELLOW,BROWN,BRRED,GRAY};
int motor_num = 100;
bool user_mode = false;
bool working_state = false;
volatile uint32_t adc_values[3];    // 0: 외부조도, 1: 내부조도, 2: 온도센서
/*
uint16_t illuminance_value;         // 조도 센서값
uint16_t humidity_value;            // 습도 센서값
uint16_t adc_conversion_index;
volatile bool is_interrupted = false;
*/

void RCC_Configure(void) {
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE); 
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE); 
   
   //RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); 
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}

void GPIO_Configure(void) {

    GPIO_InitTypeDef GPIO_InitStructure; 
   
    /* Button - PC4 / PB10 / PC13 / PA0 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_10 | GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    // Libraries\STM32F10x_StdPeriph_Driver_v3.5\src\stm32f10x_gpio.c
    GPIO_Init(GPIOC, &GPIO_InitStructure);  
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* LED - PD2 / PD3 / PD4 / PD7 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* ADC - PB0, PB1, PB2 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;       // 조도센서 입력모드
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;       // 조도센서 입력모드
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;       // 온도센서 입력모드
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* UART2 - PD5(TX) / PD6(RX) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure); 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure); 

    /* DC Motor */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void DMA_Configure(void) {
    DMA_InitTypeDef DMA_InitStructure;
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&adc_values[0];
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 3; 
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; 
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word; 
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    
    DMA_Cmd(DMA1_Channel1, ENABLE); // DMA 활성화
}

/* 
ADC2는 ADC1과 동일한 핀을 공유하므로 충돌 일어남으로 못 씀
그래서 ADC1에서 조도, 습도 채널을 따로 만들고 타이밍 제어함
조도 PB0 -> ADC_Channel_8, 습도 PB1 -> ADC_Channel_9 
인터럽트 방식을 쓰면 채널간 번갈아가며 받아야함 -> DMA가 낫다
*/ 
void ADC_Configure(void) { 
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_NbrOfChannel = 3; // 1->2 edited
    ADC_InitStructure.ADC_ScanConvMode = ENABLE; 

    // 조도1 센서 채널 설정 (PB0 - ADC_Channel_8)
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5);
    // 조도2 센서 채널 설정 (PB1 - ADC_Channel_9)
    ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 2, ADC_SampleTime_239Cycles5);
    // 온도 센서 채널 설정 (PB1 - ADC_Channel_9)
    ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 3, ADC_SampleTime_239Cycles5);
    

    //ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE); 이거 대신 ADC_DMACmd() 써야함
    //DMA 모드를 활성화하여 변환된 데이터를 DMA를 통해 직접 메모리로 전송
    ADC_DMACmd(ADC1, ENABLE); 
    
    ADC_Cmd(ADC1,ENABLE);       // ADC1 활성화
    ADC_ResetCalibration(ADC1); // ADC 캘리브레이션
    while(ADC_GetResetCalibrationStatus(ADC1)!=RESET) ;
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1)) ;

    ADC_SoftwareStartConvCmd(ADC1, ENABLE); 
    //ADC 변환이 완료되면, DMA가 다시 첫 번째 채널로 돌아가 순환적으로 값을 저장
}

void EXTI_Configure(void) {
    EXTI_InitTypeDef EXTI_InitStructure;

    // Button 1 (PC4)
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource4);
    EXTI_InitStructure.EXTI_Line = EXTI_Line4;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // Button 2 (PB10)
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource10);
    EXTI_InitStructure.EXTI_Line = EXTI_Line10;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // Button 3 (PC13)
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource13);
    EXTI_InitStructure.EXTI_Line = EXTI_Line13;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // Button 4 (PA0)
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
}

void USART2_Init(void) {
    USART_InitTypeDef USART2_InitStructure;
    USART_Cmd(USART2, ENABLE);

    USART2_InitStructure.USART_BaudRate = 9600;
    USART2_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART2_InitStructure.USART_StopBits = USART_StopBits_1;
    USART2_InitStructure.USART_Parity = USART_Parity_No;
    USART2_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART2_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART2, &USART2_InitStructure);
    
    GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE); // Remap 활성화
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
}

void NVIC_Configure(void) {

    NVIC_InitTypeDef NVIC_InitStructure; 
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
    //Preemption Priority를 6비트로 설정하고 Sub Priority를 2비트로 설정 
    //(Preemption이 중요하고 Sub Priority도 적당히 중요할 때)

    // Button 1
    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn; // for PC4
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // Button 2, 3
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn; // for PB10, PC13
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // Button 4 (added - EXTI0_IRQHandler; EXTI Line 0)
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn; // for PA0
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // UART2
    NVIC_EnableIRQ(USART2_IRQn);
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /*
    // ADC (edit - NVIC_InitStructure1 이었는데 수정함)
    NVIC_EnableIRQ(ADC1_2_IRQn);
    NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);
    */
}

// ---------------USART_SendData(USART2, msg[i]);
void USART2_IRQHandler(void) {
    uint16_t word;

    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        word = USART_ReceiveData(USART2);

        switch (word) {
            case 'a':
                motor_num = 1;
                MotorOPChange(1); // 모터 1 작동
                user_mode = true;
                working_state = true; 
                break;
            case 'b':
                motor_num = 2;
                MotorOPChange(2); // 모터 2 작동
                user_mode = true;
                working_state = true; 
                break;
            case 'c':
                motor_num = 3;
                MotorOPChange(3); // 모터 3 작동
                user_mode = true;
                working_state = true; 
                break;
            case 'r':
                motor_num = 0;
                MotorOPChange(0);
                user_mode = false; // 사용자 모드 해제
                working_state = true; 
                break;
            default:
                break;
        }

        USART_ClearITPendingBit(USART2, USART_IT_RXNE); 
    }
}

// ------------------------button 1
void EXTI4_IRQHandler(void) {

    if (EXTI_GetITStatus(EXTI_Line4) != RESET) {
        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == Bit_RESET) {
            //is_interrupted = true;
            working_state = true;
            user_mode = true;
            MotorOPChange(1);
        }
        EXTI_ClearITPendingBit(EXTI_Line4); 
    }
}

// ----------------------button 2, 3
void EXTI15_10_IRQHandler(void) { 

    if (EXTI_GetITStatus(EXTI_Line10) != RESET) {
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == Bit_RESET) { 
            //is_interrupted = true;
            working_state = true;
            user_mode = true;
            MotorOPChange(2);
        }
        EXTI_ClearITPendingBit(EXTI_Line10);
    }

    if (EXTI_GetITStatus(EXTI_Line13) != RESET) {
        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == Bit_RESET) {
            //is_interrupted = true;
            working_state = true;
            user_mode = true;
            MotorOPChange(3);
        }
        EXTI_ClearITPendingBit(EXTI_Line13);
    }
}

// ------------------------button 4
// Libraries\CMSIS\DeviceSupport\Startup\startup_stm32f10x_cl.s
void EXTI0_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET) {
            //is_interrupted = false;
            working_state = true;
            user_mode = false;
        }
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

void Delay(void) {
   int i;
   for (i = 0; i < 1000000; i++) {}
}

void sendDataUART2() {
    char msg[]= "dry done\r\n"; 
    for(int i=0; i <= 18; i++) {
        while ((USART2->SR & USART_SR_TC) == 0);
        USART_SendData(USART2, msg[i]);
    }
    //USART_ClearITPendingBit(USART1, USART_IT_RXNE);
}

void MotorConfigure() {
    GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_RESET);    // 모터 1
    GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_RESET);    // 모터 2
    GPIO_WriteBit(GPIOA, GPIO_Pin_11, Bit_RESET);   // 모터 3
}

void MotorOPChange(int num) {
    if (num == 1) { // 반대일 수도 있음
        motor_num = 1;
        GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_SET);
        GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_RESET);
        GPIO_WriteBit(GPIOA, GPIO_Pin_11, Bit_RESET);
    } else if (num == 2) {
        motor_num = 2; 
        GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_SET);
        GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_SET);
        GPIO_WriteBit(GPIOA, GPIO_Pin_11, Bit_RESET);
    } else if(num == 3) {
        motor_num = 3; 
        GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_SET);
        GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_SET);
        GPIO_WriteBit(GPIOA, GPIO_Pin_11, Bit_SET);
    } else if(num == 0) {
        GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_RESET);
        GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_RESET);
        GPIO_WriteBit(GPIOA, GPIO_Pin_11, Bit_RESET);
    }
}


//LCD 초기화 설정
void LCD_DisplayStatus() {
    char buffer[32];
    
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "outdoor : %d%%", adc_values[1]);
    LCD_ShowString(10, 10, buffer, BLACK, WHITE);

    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "indoor : %d", adc_values[0]);
    LCD_ShowString(10, 30, buffer, BLACK, WHITE);

    
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "temper : %d", adc_values[0]);
    LCD_ShowString(10, 50, buffer, BLACK, WHITE);

/*
    // 현재 동작 상태 표시
    if (working_state) {
        LCD_ShowString(10, 50, "Status: Working", GREEN, WHITE);
    } else {
        LCD_ShowString(10, 50, "Status: Stop", RED, WHITE);
    }*/
}

int main(void) {

    SystemInit();
    RCC_Configure();
    GPIO_Configure();
    DMA_Configure();
    ADC_Configure();
    NVIC_Configure();
    EXTI_Configure();
    MotorConfigure();
    USART2_Init();
    //LCD 설정
    LCD_Init();
    Touch_Configuration();
    //Touch_Adjust();
    LCD_Clear(WHITE);

    // 시스템 제어 모드 안에서 인터럽트가 발생했을때 동시성 문제가 존재함.
    while (1) {
        while (DMA_GetFlagStatus(DMA1_FLAG_TC1) == RESET) ; // DMA 전송 완료까지 대기
        DMA_ClearFlag(DMA1_FLAG_TC1); 
        LCD_DisplayStatus();    // LCD 상태 갱신
        Delay();
        // 시스템 제어 모드 (습하면 값이 크다)
        if(!user_mode) {
            if(adc_values[1] > 60 && adc_values[0] > 100) {// 낮이고 습하면 건조 시작
                working_state = true;// 밤이되더라도 이미 한번 작동한 working_state는 true이니까밑에 조건들은 다 수행 될 수 있음.
            }
            
            
            __disable_irq(); 
            if(adc_values[1] >= 80 && working_state) {
                // LED도 MotorOPChange 처럼 해주면 됨
                if(motor_num != 3) MotorOPChange(3);    // motor_num 필요한지 생각
                GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET);   // LED1만 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_RESET); // LED2 OFF
                GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_RESET); // LED3 OFF
                GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_RESET); // LED4 OFF
            }     
            else if(adc_values[1] >= 70 && working_state) { 
                if(motor_num != 2) MotorOPChange(2);
                GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET);   // LED1 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_SET);   // LED2 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_RESET); // LED3 OFF
                GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_RESET); // LED4 OFF
            } 
            else if(adc_values[1] >= 60 && working_state) {
                if(motor_num != 1) MotorOPChange(1);
                GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET);   // LED1 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_SET);   // LED2 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_SET);   // LED3 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_RESET); // LED4 OFF
            } 
            
            if (adc_values[1] < 60 && working_state) { // 건조 완료 (습도 60 미만)
                working_state = false;
                MotorOPChange(0);
                GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET);  // LED1 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_SET);  // LED2 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_SET);  // LED3 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_SET);  // LED4 ON
                sendDataUART2();
            }
            __enable_irq(); // pending 시키는거니 알림은 제대로 갈듯
        }
        // 사용자 모드
        else {
            // Button 4 Interrupt에서 user_state를 false로 고쳐줘야함
            // Button들은 working_state를 항상 true로 유지시켜줘야함
            // 우산을 넣기 전 사용자 모드를 먼저 실행시키는 케이스도 고려
        }
    }
    
    return 0;
}

