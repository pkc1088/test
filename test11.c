/**
 * @brief 11주차 코드 / Timer
*/

#include "stm32f10x.h"
#include "core_cm3.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_adc.h"
#include "lcd.h"
#include "touch.h"
#include "stm32f10x_tim.h"


int color[12] =
{WHITE,CYAN,BLUE,RED,MAGENTA,LGRAY,GREEN,YELLOW,BROWN,BRRED,GRAY};
uint16_t time_cnt = 1;
uint16_t Btn_State = 1;
uint16_t LED1_state = 1;
uint16_t LED2_state = 1;
uint16_t angle = 2000; // 서보모터 초기 각도

void RCC_Configure(void) // ADC 입력을 ADC1포트에 인가&& B포트 0번을 ADC1입력으로 사용하기로함
{
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}

void GPIO_Configure(void)
{
    // TIM3 - PWM PB0 
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // LED 1, 2인 PD2, PD3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

}


void NVIC_Configure(void)
{
  NVIC_InitTypeDef NVIC_InitStructure1;
  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
  NVIC_EnableIRQ(TIM2_IRQn);
  NVIC_InitStructure1.NVIC_IRQChannel = TIM2_IRQn; //TIM2 인터럽 핸들러
  NVIC_InitStructure1.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStructure1.NVIC_IRQChannelPreemptionPriority = 0x0;
  NVIC_InitStructure1.NVIC_IRQChannelSubPriority = 0x0;
  NVIC_Init(&NVIC_InitStructure1);
  
  /*
  // 필요한가??
  NVIC_EnableIRQ(TIM3_IRQn);  //TIM3 인터럽 핸들러
  NVIC_InitStructure1.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure1.NVIC_IRQChannelCmd =ENABLE;
  NVIC_InitStructure1.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure1.NVIC_IRQChannelSubPriority = 0x0;
  NVIC_Init(&NVIC_InitStructure1);
  */
}

void TIM_Configure(void) { //#define SYSCLK_FREQ_72MHz 72 / 100 0000
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; //
    TIM_OCInitTypeDef TIM_OCInitStructure;

    // TIM3 설정 (서보모터를 제어)
    // 50Hz ~ 1000Hz의 주파수를 요구하는 서보모터에 맞춰서 Period, Prescaler 설정
    // TIM3 타이머의 한 사이클이 20,000 타이머 틱(클록) 동안 지속됨을 의미
    // 즉 TIM_Period : 타이머가 오버플로(리셋)되기 전까지의 카운트 값으로, 
    // 여기서는 20,000으로 설정되어 있습니다.
    TIM_TimeBaseStructure.TIM_Period = 20000; 
    // PWM 주기의 기간 설정 (20,000 타이머 틱 = 20ms 주기 = 50Hz)
    TIM_TimeBaseStructure.TIM_Prescaler = 72; 
    // 타이머는 72MHz(기본클락) / 72 = 1MHz의 클록으로 작동함
    // 즉 Prescaler가 72를 이용해 1MHz로 분주하고 Period가 20000이니까 
    // 최종 주파수는 1MHz/20000 = 50Hz로서 서보모터 요구사항 충족
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;


    // TIM3 - PWM 설정
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 
    //PWM 모드 1로 설정되어 타이머가 일정한 듀티 사이클을 따라 출력 신호를 조절합니다.
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; 
    //PWM 신호의 초기 극성을 High로 설정해, 초기 신호가 High로 시작합니다.
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; 
    //타이머 출력을 활성화하여 실제 핀에 신호를 전달합니다.
    //Pulse는 듀티 사이클을 설정하는 데 사용됩니다. 
    //예를 들어, TIM_Pulse가 1000이면 전체 주기 중 1000(pulse) / 20000(주기) = 5% 동안 신호가 High로 유지됩니다.
    /*서보모터는 펄스 폭에 따라 회전 각도가 달라집니다. 일반적으로 펄스 폭이 작아지면 각도가 
    감소하고, 커지면 각도가 증가합니다. 예를 들어, 펄스 폭이 1000이면 서보모터가 
    특정 각도로 설정되고, 펄스 폭이 더 커지거나 작아지면 다른 각도로 설정됩니다.*/
    // PWM : 아날로그 신호를 디지털화 하여 인코딩하는 방법. 
    TIM_OCInitStructure.TIM_Pulse = 1000; // TODO 
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Disable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);


    // TIM2 설정 1초를 세어야 하므로 이에 맞춰서 Period, Prescaler 설정
    // TIM2 - 1초마다 인터럽트 발생
    // 1초에 한번씩 작동하도록 설정해주기 위해 
    // (1/72Mhz) 10000 (72M/10000) = 1의 계산식으로 1초로 설정
    // (1/F_clk) x Prescaler x Period
    TIM_TimeBaseStructure.TIM_Period = 10000; // 10,000으로 설정되어 한 사이클이 10,000 타이머 틱 동안 지속됨을 의미합니다.
    // 즉 TIM_Period : 타이머가 오버플로(리셋)되기 전까지의 카운트 값으로, 여기서는 10,000으로 설정되어 있습니다.
    // 프리스케일러가 7200으로 설정되어 클록이 72MHz / 7200 = 10kHz로 분주됩니다. 
    // 결과적으로 10,000 / 10,000Hz = 1초가 되므로, 1초마다 인터럽트가 발생하게 됩니다.
    TIM_TimeBaseStructure.TIM_Prescaler = 7200; 
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // down?
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_ARRPreloadConfig(TIM2, ENABLE);
    TIM_Cmd(TIM2, ENABLE);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); // TIM2가 오버플로할 때마다 인터럽트를 발생시키도록 설정합니다.
}

/*
//서보모터의 펄스 폭을 설정하여 모터의 각도를 조정합니다. 
//TIM3->CCR3 레지스터에 펄스 값(pulse)을 쓰면, 해당 펄스 폭으로 서보모터가 회전합니다.
void rotateServo(uint16_t pulse){ // <- angle +-100
    TIM3->CCR3 = pulse;
}*/

/*
//TIM3의 인터럽트 핸들러
void TIM3_IRQHandler() {
    //On 상태일 시 degree가 계속 증가, 최고 각도 2000을 넘어갈 시 
    //다시 최저 각도인 1000으로 초기화.
    if (Btn_State == 1) {
        if (angle >= 2000){ angle = 1000; }
        else{ angle += 100; }
    }
    //Off 상태일 시 degree가 계속 감소, 최저 각도 1000을 넘어갈 시 
    //다시 최고 각도인 2000으로 초기화.
    else{
        if (angle <= 1000){ angle = 2000; }
        else{ angle -= 100; }
    }
    rotateServo(angle);
}
*/

void TIM2_IRQHandler(void) {
    time_cnt++;
    if(Btn_State == 1){ // on 상태
        if (time_cnt % 5 == 0) { // 5초마다
            if (LED1_state == 1) {
                LED1_state = 0;
                GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_RESET);    // LED2 켜기
            } else {
                LED1_state = 1;
                GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_SET);      // LED2 끄기
            }
        }
        if(time_cnt % 1 == 0) { // 1초마다
            if (LED2_state == 1) {
                LED2_state = 0;
                GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_RESET);    // LED1 켜기
            } else {
                LED2_state = 1;
                GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET);      // LED1 끄기
            }
        }
    }
    if (Btn_State == 1) { 
        if (angle > 2000) { angle = 1000; }
        else { angle += 100; }
    } else{
        GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET);    // LED1 끄기
        GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_SET);    // LED2 끄기
        if (angle < 1000){ angle = 2000; }
        else{ angle -= 100; }
    }
    TIM3->CCR3 = angle; //rotateServo(angle);
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}



int main() {
    SystemInit();
    RCC_Configure();
    GPIO_Configure();
    NVIC_Configure();
    TIM_Configure();
    LCD_Init();
    Touch_Configuration();
    Touch_Adjust();
    LCD_Clear(WHITE);

    uint16_t x, y, tch_x, tch_y;
    char msg[]= "TUE_Team11";
    LCD_ShowString(50, 50, msg, BLACK, WHITE);

    LCD_DrawRectangle(100, 100, 150, 150);
    
    LCD_ShowString(125, 125, "On ", GREEN,WHITE);
    GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET);
    while(1) {
        // TODO : LCD 값 출력 및 터치 좌표 읽기
        Touch_GetXY(&x, &y, 2);
        Convert_Pos(x, y, &tch_x, &tch_y);

        if (tch_x <= 150 && tch_x >= 100 && tch_y <= 150 && tch_y >= 100) {
            if (Btn_State == 1) {
                Btn_State = 0;
                LCD_ShowString(125, 125, "Off", RED, WHITE);
            } else {
                Btn_State = 1;
                LCD_ShowString(125, 125, "On ", GREEN, WHITE);
            }
            x = 0; y = 0;
            tch_x = 0; tch_y = 0;
        }
    }
    return 0;
}