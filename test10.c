/**
 * @brief 10주차 LCD 코드 / main.c
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

int color[12] =
{WHITE,CYAN,BLUE,RED,MAGENTA,LGRAY,GREEN,YELLOW,BROWN,BRRED,GRAY};
uint16_t value; // 조도센서값

void RCC_Configure(void) 
/*
ADC 입력을 ADC1포트에 인가, B포트 0번을 ADC1입력으로 사용하기로함
*/
{
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}

void GPIO_Configure(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;     // 0 번 사용
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; // 입력모드
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void ADC_Configure(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE; //?
        
    ADC_Init(ADC1, &ADC_InitStructure);
    ADC_RegularChannelConfig(ADC1,ADC_Channel_8,1,ADC_SampleTime_239Cycles5);
    ADC_ITConfig(ADC1,ADC_IT_EOC,ENABLE);
    ADC_Cmd(ADC1,ENABLE);
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1)!=RESET);
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
    ADC_SoftwareStartConvCmd(ADC1,ENABLE);
}


void NVIC_Configure(void)
{
    NVIC_InitTypeDef NVIC_InitStructure1;
    NVIC_EnableIRQ(ADC1_2_IRQn);
    NVIC_InitStructure1.NVIC_IRQChannel = ADC1_2_IRQn;
    NVIC_InitStructure1.NVIC_IRQChannelCmd =ENABLE;
    NVIC_InitStructure1.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure1.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure1);
}

void ADC1_2_IRQHandler() {
    //ADC 값은 인터럽트로 처리하여 전역변수에 저장 이때 전역변수는 uint16_t value인 조도센서값임
    if(ADC_GetITStatus(ADC1,ADC_IT_EOC)!=RESET){
        value = ADC_GetConversionValue(ADC1);
    }
    ADC_ClearITPendingBit(ADC1,ADC_IT_EOC);
}

int main() {
  
// LCD 관련 설정은 LCD_Init에 구현되어 있으므로 여기서 할 필요 없음
    SystemInit();
    RCC_Configure();
    GPIO_Configure();
    ADC_Configure();
    NVIC_Configure();
    
    LCD_Init();
    Touch_Configuration();
    Touch_Adjust();
    LCD_Clear(WHITE);

    uint16_t x, y, tch_x, tch_y;
    char msg[]= "TUE_Team11";
    LCD_ShowString(10, 10, msg, BLACK, WHITE); //LCD_ShowString(50, 50, msg, BLACK, WHITE);

    while(1){
    // 터치 할 때 마다 작은 원 생성 및 ADC 값 출력 갱신, 터치 좌표 출력
        //void Touch_GetXY(uint16_t *x, uint16_t *y, uint8_t ext)
        Touch_GetXY(&x, &y, 2);
        Convert_Pos(x, y, &tch_x, &tch_y);
        //void LCD_ShowNum(u8 x, u8 y, u32 num, u8 len, u16 PenColor, u16 BackColor)
        
        LCD_ShowNum(50, 50, tch_x, 4, GREEN,WHITE);//LCD_ShowNum(8, 100, tch_x, 10, GREEN,WHITE);
        LCD_ShowNum(50, 70, tch_y, 4, GREEN,WHITE);//LCD_ShowNum(8, 120, tch_y, 10, GREEN,WHITE);
        
        /*
        ADC_ITConfig(ADC1,ADC_IT_EOC,ENABLE);
        */
        LCD_DrawCircle(tch_x, tch_y, 5);
        LCD_ShowNum(60, 100, value, 4, BLACK,WHITE);//LCD_ShowNum(8, 140, value, 10, GREEN,WHITE);
        // ADC_ITConfig(ADC1,ADC_IT_EOC,DISABLE);?
    }
    return 0;
}

