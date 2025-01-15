/**
 * @brief REAL FINAL
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

#define ADC_MIN 0          // ADC 최소값
#define ADC_MAX 4095       // ADC 최대값 (12-bit ADC 기준)
#define OUTPUT_MIN 0       // 매핑된 최소 출력값
#define OUTPUT_MAX 100     // 매핑된 최대 출력값

void RCC_Configure(void);
void GPIO_Configure(void);
void EXTI_Configure(void);
void USART2_Init(void);
void NVIC_Configure(void);
void Timer_Configuration(void);
void TIM2_IRQHandler(void);
void EXTI15_10_IRQHandler(void);
void EXTI4_IRQHandler(void);
void EXTI0_IRQHandler(void);
void Motor_Control(int motor, int enable);
void TriggerPulse(void);
void personCheck(void);
void Delay(void);
void sendDataUART2(void);

void USART1_Init(void);

int color[12] = {WHITE,CYAN,BLUE,RED,MAGENTA,LGRAY,GREEN,YELLOW,BROWN,BRRED,GRAY};
int mood_value, jodo;
bool user_mode = false;
bool quitFlag = false;
bool onoff = false;
int adc_values[2] = {0, 0}; // 조도1, 조도2
volatile uint32_t pulseWidth = 0;  // Echo 핀의 HIGH 시간
volatile uint8_t echoState = 0;    // Echo 핀 상태 (0: LOW, 1: HIGH)

void RCC_Configure(void) {
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE); 
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE); 
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
     RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); 
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}
// temp message
void GPIO_Configure(void) {

    GPIO_InitTypeDef GPIO_InitStructure; 

    /* Button - PC4 / PB10 / PC13 / PA0 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_13; /*GPIO_Pin_10 |*/ 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOC, &GPIO_InitStructure);  
    //GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* LED - PD2 / PD3 / PD4 / PD7 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

     /* Trigger - PA11 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Echo - PA12 로 테스트 *///////////////////////////////////////////
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;  
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Motor1 - EN1 (PB6), IN1 (PB7)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // Motor2 - EN2 (PC0), IN3 (PC1)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
   
    /* ADC - PB0, PB1 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;       // 조도센서1 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;       // 조도센서2 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* LED - PA1, PC12 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* UART2 - PD5(TX) / PD6(RX) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure); 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure); 

    
    // USART1 
    // TX
    // TX를 PA9에 연결
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    // RX
    // RX를 PA10에 연결
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);


    /* 초기 상태 설정 (모두 OFF) */
    Motor_Control(1, 0);
    Motor_Control(2, 0);
    GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET);
    GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_SET);
    GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_SET);
    GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_SET);
}

void DMA_Configure(void) {
    DMA_InitTypeDef DMA_InitStructure;
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&adc_values[0];
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = sizeof(adc_values) / sizeof(adc_values[0]); 
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; //Half 
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word; //Half 
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    
    DMA_Cmd(DMA1_Channel1, ENABLE);
}

void ADC_Configure(void) { 
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_NbrOfChannel = 2; 
    ADC_InitStructure.ADC_ScanConvMode = ENABLE; 

    // 조도센서1 채널 설정 (PB0 - ADC_Channel_8)
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5);
    // 조도센서2 채널 설정 (PB1 - ADC_Channel_9)
    ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 2, ADC_SampleTime_239Cycles5);
    ADC_Init(ADC1, &ADC_InitStructure); //ADC_ITConfig 대신 사용

    ADC_DMACmd(ADC1, ENABLE); 
    
    ADC_Cmd(ADC1,ENABLE);       
    ADC_ResetCalibration(ADC1); 
    while(ADC_GetResetCalibrationStatus(ADC1) != RESET) ;
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1)) ;

    ADC_SoftwareStartConvCmd(ADC1, ENABLE); 
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
    
    /*
    // Button 2 (PB10)
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource10);
    EXTI_InitStructure.EXTI_Line = EXTI_Line10;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    */

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

    // Echo (PA12) /////////////////////////////////////////////////////////////
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource12);
    EXTI_InitStructure.EXTI_Line = EXTI_Line12;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; 
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
}

void USART1_Init(void) {
    USART_InitTypeDef USART1_InitStructure;

    // Enable the USART1 peripheral
    USART_Cmd(USART1, ENABLE);
    
    // TODO : Initialize the USART using the structure 'USART_InitTypeDef' and the function 'USART_Init'
    // UART1의 기초설정 BaudRate는 9600으로 설정
    USART1_InitStructure.USART_BaudRate = 9600;
    USART1_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART1_InitStructure.USART_StopBits = USART_StopBits_1;
    USART1_InitStructure.USART_Parity = USART_Parity_No;
    USART1_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART1_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &USART1_InitStructure);
    
    // TODO : Enable the USART1 RX interrupts using the function 'USART_ITConfig'
    // 설정을 통해 UART1 활성화
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
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
    
    // Button 4 
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn; // for PA0
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // USART1
    // 'NVIC_EnableIRQ' is only required for USART setting
    // UART1 설정 우선순위를 0으로 설정.
    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //TODO
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; //TODO
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // UART2
    NVIC_EnableIRQ(USART2_IRQn);
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // TIM2 
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void Timer_Configuration(void) {

    // 얘가 초음파
    TIM_TimeBaseInitTypeDef TIM_InitStructure;
    TIM_InitStructure.TIM_Prescaler = 72 - 1;  // 1μs 타이머 주기 설정 (72MHz 클럭)
    TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_InitStructure.TIM_Period = 0xFFFF;  // 최대 타이머 값 설정
    TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM1, &TIM_InitStructure);
    TIM_Cmd(TIM1, ENABLE);

    // 얘가 LED
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef OutputChannel;
    /* TIM2 Initialize */
    TIM_TimeBaseStructure.TIM_Period = 100 - 1;  // 100kHz
    TIM_TimeBaseStructure.TIM_Prescaler = 24 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    /* TIM2 PWM Initialize */
    OutputChannel.TIM_OCMode = TIM_OCMode_PWM1;
    OutputChannel.TIM_OutputState = TIM_OutputState_Enable;
    OutputChannel.TIM_OutputNState = TIM_OutputNState_Enable;
    OutputChannel.TIM_Pulse = 50 - 1;  // 50% duty ratio
    OutputChannel.TIM_OCPolarity = TIM_OCPolarity_Low;
    OutputChannel.TIM_OCNPolarity = TIM_OCNPolarity_High;
    OutputChannel.TIM_OCIdleState = TIM_OCIdleState_Set;
    OutputChannel.TIM_OCNIdleState = TIM_OCIdleState_Reset;
    TIM_OC2Init(TIM2, &OutputChannel);
    /* TIM2 Enable */
    TIM_Cmd(TIM2, ENABLE);
    TIM_ITConfig(TIM2, TIM_IT_Update | TIM_IT_CC2, ENABLE); 
}

void USART1_IRQHandler() {
    uint16_t word;
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        // the most recent received data by the USART1 peripheral
        word = USART_ReceiveData(USART1);
        // TODO: Implement your code to handle received data
        // 받은 데이터를 UART2로 전달
        USART_SendData(USART2, word);
        // clear 'Read data register not empty' flag
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

void USART2_IRQHandler(void) {
    uint16_t word;

    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        word = USART_ReceiveData(USART2);
        USART_SendData(USART1, word); // 추가 함 ///////////////////
        switch (word) {
            case 'a':
                Motor_Control(1, 1);
                Motor_Control(2, 1);
                break;
            case 'b':
                Motor_Control(1, 1);
                Motor_Control(2, 0);
                user_mode = true;
                break;
            case 'c':
                Motor_Control(1, 0);
                Motor_Control(2, 0);
                user_mode = true;
                break;
            case 'r':
                user_mode = false; // 사용자 모드 해제
                break;
            case 'q':
                quitFlag = true;
                break;
            default:
                break;
        }

        USART_ClearITPendingBit(USART2, USART_IT_RXNE); 
    }
}

// LED TIM2 - 타이머의 overflow 이벤트 발생 시 호출 
void TIM2_IRQHandler(void) {
    //TIM2의 주기(Period)와 비교 이벤트(CC2)에 따라 PC12 핀이 켜졌다 꺼지며, 
    //LED가 깜빡이는 동작을 수행합니다.
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  // Clear the interrupt flag
        GPIOC->BRR = GPIO_Pin_12; 
    }
    if (TIM_GetITStatus(TIM2, TIM_IT_CC2) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);
        GPIOC->BSRR = GPIO_Pin_12; 
    }
}

// button 1 
void EXTI4_IRQHandler(void) {
    // motor1, 2 on/off
    if (EXTI_GetITStatus(EXTI_Line4) != RESET) {
        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == Bit_RESET) {
            user_mode = true;
            BitAction motor1_state = GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_6);
            if (motor1_state == Bit_RESET) {
                Motor_Control(1, 1);
                Motor_Control(2, 1);
            } else {
                Motor_Control(1, 0);
                Motor_Control(2, 0);
            }
        }
        EXTI_ClearITPendingBit(EXTI_Line4); 
    }
}

// ECHO, button 3
void EXTI15_10_IRQHandler(void) { 
    
    // ECHO - TIM1 PA10
    if (EXTI_GetITStatus(EXTI_Line12) != RESET) {
        
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12) == Bit_SET) {
            // Echo가 HIGH로 변했을 때, 타이머를 리셋하여 시간 측정 시작
            // 타이머 카운터 초기화 // Echo 신호가 HIGH
            TIM_SetCounter(TIM1, 0);            
            echoState = 1;                      
        } else {
            // Echo가 LOW로 변했을 때, 타이머 값으로 시간 측정 완료
            // 타이머 값 읽기 // Echo 신호가 LOW
            pulseWidth = TIM_GetCounter(TIM1);  
            echoState = 0;                      
        }
        EXTI_ClearITPendingBit(EXTI_Line12);
    }

    // button 3 - LED on/off
    if (EXTI_GetITStatus(EXTI_Line13) != RESET) {
        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == Bit_RESET) {
            user_mode = true;
            if(onoff) {TIM2->CCR2 = 0; onoff = false;}
            else {TIM2->CCR2 = 100; onoff = true;}
        }
        EXTI_ClearITPendingBit(EXTI_Line13);
    }
}

// button 4 - System Mode
void EXTI0_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET) {
            user_mode = false;
        }
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

void Delay(void) {
   int i;
   for (i = 0; i < 1000000; i++) {}
}

void sendDataUART2(void) {
    char msg[] = "User Entered\r\n";
    int i = 0;
    while(msg[i] != '\0') {
        while ((USART2->SR & USART_SR_TC) == 0);
        USART_SendData(USART2, msg[i]);
        i++;
    }
}

void Motor_Control(int motor, int enable) {
    if (motor == 1) {  // 모터 1 제어
        if (enable) {
            GPIO_WriteBit(GPIOB, GPIO_Pin_6, Bit_SET); // EN1 핀 HIGH (모터 활성화)
            GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_SET); // IN1 핀 HIGH (정방향은 고정)
        } else {
            GPIO_WriteBit(GPIOB, GPIO_Pin_6, Bit_RESET); // EN1 핀 LOW (모터 비활성화)
        }
    } else if (motor == 2) {  // 모터 2 제어
        if (enable) {
            GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_SET); // EN2 핀 HIGH (모터 활성화)
            GPIO_WriteBit(GPIOC, GPIO_Pin_1, Bit_SET); // IN3 핀 HIGH (정방향은 고정)
        } else {
            GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_RESET); // EN2 핀 LOW (모터 비활성화)
        }
    }
}

void TriggerPulse(void) {
    // Trigger 핀에 짧은 펄스 전송 (10μs)
    GPIO_WriteBit(GPIOA, GPIO_Pin_11, Bit_SET);
    for (volatile int i = 0; i < 720; i++);  // 약 10μs 대기 (72MHz 클럭 기준)
    GPIO_WriteBit(GPIOA, GPIO_Pin_11, Bit_RESET);
}

void personCheck(void) {
    while (1) {
        TriggerPulse();  // 초음파 송신을 위해 Trigger 핀에 펄스 보내기
        for (volatile int i = 0; i < 1000000; i++) ; 

        float distance = (pulseWidth * 0.0343) / 2; 
        if((int)distance < 10) {
            return;
        }
    }
    return;
}

void LCD_DisplayStatus(void) {
    
    char buffer1[64];
    memset(buffer1, 0, sizeof(buffer1));
    sprintf(buffer1, "jodo1 first : ");
    LCD_ShowString(10, 70, buffer1, BLACK, WHITE);
    LCD_ShowNum(140, 70, adc_values[0], 4, BLACK, WHITE);
    
    char buffer2[64];
    memset(buffer2, 0, sizeof(buffer2));
    sprintf(buffer2, "jodo2 second : ");
    LCD_ShowString(10, 100, buffer2, BLACK, WHITE);
    LCD_ShowNum(140, 100, adc_values[1], 4, BLACK, WHITE);
    
    char buffer3[64];
    memset(buffer3, 0, sizeof(buffer3));
    sprintf(buffer3, "mood_value : ");
    LCD_ShowString(10, 130, buffer3, BLACK, WHITE);
    LCD_ShowNum(140, 130, mood_value, 4, BLACK, WHITE); // 3 or 9
}

int main(void) {
    Start :
    SystemInit();
    RCC_Configure();
    GPIO_Configure();
    DMA_Configure();
    ADC_Configure();
    NVIC_Configure();
    EXTI_Configure();
    Timer_Configuration();
    USART1_Init(); ///////////////
    USART2_Init();
    LCD_Init();
    Touch_Configuration();
    //Touch_Adjust();
    LCD_Clear(WHITE);

    personCheck(); // 초음파 무한 대기
    sendDataUART2();

    while (1) {
        if(quitFlag) {
            quitFlag = false;
            goto Start;
        }

        while (DMA_GetFlagStatus(DMA1_FLAG_TC1) == RESET) ; 
        DMA_ClearFlag(DMA1_FLAG_TC1); 
        LCD_DisplayStatus();
        Delay();
        if(!user_mode) {
            __disable_irq(); 
            // 가리면(어두워지면) => adc값 커짐 => jodo값 커짐 => mode_value값 커짐 
            // => 100- mode_value 값, 즉 최종 출력 mode값이 작아짐 
            jodo = (adc_values[0] + adc_values[1]) / 2;
            mood_value = ((jodo - ADC_MIN) * (OUTPUT_MAX - OUTPUT_MIN)) / (ADC_MAX - ADC_MIN) + OUTPUT_MIN;
            mood_value = 100 - mood_value;
            TIM2->CCR2 = mood_value; // LED는 작은 값 넣어야 밝아짐

            if(mood_value >= 70) { 
                // 기본 원리가 : 가리면(모드값작아짐) -> 추우니까 -> 모터가 안 돌아
                // 기본 원리가 : 떼면(밝으니까 모드값 커짐) -> 더우니까 -> 모터 돌아야함
                Motor_Control(1, 1);
                Motor_Control(2, 1);
            }     
            else if(mood_value >= 50) { 
                Motor_Control(1, 1);
                Motor_Control(2, 0);
            } 
            else if(mood_value < 50) {
                Motor_Control(1, 0);
                Motor_Control(2, 0);
            } 
            __enable_irq(); 
        }
        // 사용자 모드
        else {
         
        }
    }
     
    return 0;
}

