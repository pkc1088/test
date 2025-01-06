/**
 * @brief Term Project main.c
*/
//C/C++: Disable Error Squiggles
//C/C++: Enable Error Squiggles

// 일단 되는 코드. 

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
uint16_t Direction = 1;
int motor_num = 100;
bool user_mode = false;
bool working_state = false;
int adc_values[2]={0,0};    // 0: 조도, 1: 습도
int value=0;


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
    GPIO_Init(GPIOC, &GPIO_InitStructure);  
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* LED - PD2 / PD3 / PD4 / PD7 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // **초기 상태 설정 (모두 OFF)**
    GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET);
    GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_SET);
    GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_SET);
    GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_SET);
   
   
    /* ADC - PB0, PB1 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;       // 조도센서 입력모드
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;       // 습도센서 입력모드
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
    DMA_InitStructure.DMA_BufferSize =sizeof(adc_values) / sizeof(adc_values[0]); 
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; //Half 
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word; //Half 
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    
    DMA_Cmd(DMA1_Channel1, ENABLE); // DMA 활성화
}

void ADC_Configure(void) { 
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_NbrOfChannel = 2; // 1->2 edited
    ADC_InitStructure.ADC_ScanConvMode = ENABLE; 

    // 조도 센서 채널 설정 (PB0 - ADC_Channel_8)
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5);
    // 습도 센서 채널 설정 (PB1 - ADC_Channel_9)
    ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 2, ADC_SampleTime_239Cycles5);
    ADC_Init(ADC1, &ADC_InitStructure); 

    //DMA 모드를 활성화하여 변환된 데이터를 DMA를 통해 직접 메모리로 전송
    ADC_DMACmd(ADC1, ENABLE); 
    
    ADC_Cmd(ADC1,ENABLE);       // ADC1 활성화
    ADC_ResetCalibration(ADC1); // ADC 캘리브레이션
    while(ADC_GetResetCalibrationStatus(ADC1)!=RESET) ;
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1)) ; //!= RESET 없음

    ADC_SoftwareStartConvCmd(ADC1,ENABLE); 
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
}

/*
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
*/

// button 1
void EXTI4_IRQHandler(void) {

    if (EXTI_GetITStatus(EXTI_Line4) != RESET) {
        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == Bit_RESET) {
            working_state = true;
            user_mode = true;
            MotorOPChange(1);
        }
        EXTI_ClearITPendingBit(EXTI_Line4); 
    }
}

// button 2, 3
void EXTI15_10_IRQHandler(void) { 

    if (EXTI_GetITStatus(EXTI_Line10) != RESET) {
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == Bit_RESET) { 
            working_state = true;
            user_mode = true;
            MotorOPChange(2);
        }
        EXTI_ClearITPendingBit(EXTI_Line10);
    }

    if (EXTI_GetITStatus(EXTI_Line13) != RESET) {
        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == Bit_RESET) {
            working_state = true;
            user_mode = true;
            MotorOPChange(3);
        }
        EXTI_ClearITPendingBit(EXTI_Line13);
    }
}

// button 4
void EXTI0_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET) {
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
}

void MotorConfigure() {
    GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_RESET);    // 모터 1
    GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_RESET);    // 모터 2
    GPIO_WriteBit(GPIOA, GPIO_Pin_11, Bit_RESET);   // 모터 3
}

void MotorOPChange(int num) {
    if (num == 1) { // 반대일 수도 있음
        motor_num = 1;
      GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_RESET);  // LED1 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_SET);  // LED2 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_RESET);  // LED3 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_SET);  // LED4 ON
         } else if (num == 2) {
        motor_num = 2; GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET);  // LED1 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_RESET);  // LED2 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_SET);  // LED3 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_RESET);  // LED4 ON
    } else if(num == 3) {
        motor_num = 3; GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_RESET);  // LED1 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_RESET);  // LED2 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_RESET);  // LED3 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_RESET);  // LED4 ON
    } else if(num == 0) {GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET);  // LED1 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_SET);  // LED2 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_SET);  // LED3 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_SET);  // LED4 ON
    }
}

void LCD_DisplayStatus() {
   
   char buffer1[64]=0;
   memset(buffer1, 0, sizeof(buffer1));
   sprintf(buffer1, "jodo first : ");
   //printf("jodo 1 : %d\n", adc_values[0]);
   LCD_ShowString(10,70, buffer1, BLACK, WHITE);
   LCD_ShowNum(120, 70, adc_values[0], 4, BLACK, WHITE);
   // 이건 4자리 맞음
   
   char buffer2[64]=0;
   memset(buffer2, 0, sizeof(buffer2));
   sprintf(buffer2, "humid second : ");
   //printf("jodo 2 : %d\n", adc_values[1]);
   LCD_ShowString(10,100, buffer2, BLACK, WHITE);
   LCD_ShowNum(120, 100, adc_values[1], 2, BLACK, WHITE);

    if (working_state) { LCD_ShowString(10, 50, "Status: Working", GREEN, WHITE); } 
    else { LCD_ShowString(10, 50, "Status: Stop", RED, WHITE); }
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
    
    LCD_Init();
    Touch_Configuration();
    //Touch_Adjust();
    LCD_Clear(WHITE);

    
   while (1) {
        
      working_state = true;

    
      while (DMA_GetFlagStatus(DMA1_FLAG_TC1) == RESET) ; 
      DMA_ClearFlag(DMA1_FLAG_TC1); 
      LCD_DisplayStatus(); // 습도1 / 조도0
          Delay();
      
        if(!user_mode) {
            if(adc_values[1] > 60 && adc_values[0] > 100) {
                working_state = true;
            }

            __disable_irq(); 
            if(adc_values[1] >= 80 && working_state) {
                //if(motor_num != 3) MotorOPChange(3);
            
                GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_RESET);   // LED1 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_SET); // LED2 OFF
                GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_SET); // LED3 OFF
                GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_SET); // LED4 OFF
            }     
            else if(adc_values[1] >= 70 && working_state) { 
                //if(motor_num != 2) MotorOPChange(2);
                GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_RESET);   // LED1 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_RESET);   // LED2 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_SET); // LED3 OFF
                GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_SET); // LED4 OFF
            } 
            else if(adc_values[1] >= 60 && working_state) {
                //if(motor_num != 1) MotorOPChange(1);
                GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_RESET);   // LED1 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_RESET);   // LED2 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_RESET);   // LED3 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_SET); // LED4 OFF
            } 
            
            if (adc_values[1] < 60 && working_state) { // 건조 완료 (습도 60 미만)
                //working_state = false;
                //MotorOPChange(0);
                GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_RESET);  // LED1 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_RESET);  // LED2 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_RESET);  // LED3 ON
                GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_RESET);  // LED4 ON
                //sendDataUART2();
            }
            __enable_irq(); 
        }
  
    }// end of while
     
   return 0;
}



