/**
 * @brief 11주차 코드 / DMA 
 * RCC와 GPIO Configuration() 살림
*/

#include "stm32f10x.h"
#include "core_cm3.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include "lcd.h"

void GPIO_Configure(void);
void DMA_Configure(void);
void ADC_Configure(void);
void Delay(void);

int color[12] =
{WHITE, CYAN, BLUE, RED, MAGENTA, LGRAY, GREEN, YELLOW, BROWN, BRRED,GRAY};
//배경이 바뀌는 조도센서의 threshold를 define으로 설정
#define threshold 3000
//조도센서값을 저장할 ADC_Value 선언, BUFFER size는 1로 설정
#define BUFFER_SIZE 1

volatile uint32_t ADC_Value[BUFFER_SIZE]; // adc 값

void RCC_Configure(void) 
/*
ADC 입력을 ADC1포트에 인가, B포트 0번을 ADC1입력으로 사용하기로함
*/
{
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); //ADC1은 DMA1 사용
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}

void GPIO_Configure(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;     // 0 번 사용
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; // 입력모드
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void DMA_Configuration() {
    DMA_InitTypeDef DMA_InitStructure;
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&ADC1->DR); //DMA가 읽어올 데이터로 ADC1의 DR(Data Register)주소 설정
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) &ADC_Value[0]; //읽어온 데이터는 ADC_Value 배열에 저장
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; //주변장치에서 메모리로 데이터 전송 (방향설정)
    DMA_InitStructure.DMA_BufferSize = BUFFER_SIZE; //한번에 전송할 양 설정 : BUFFER_SIZE->1로 설정
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //주변 장치 및 메모리 주소 증가 비활성화.
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; //주변 장치에서 전송되는 데이터 크기 설정 word(32비트)
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word; //메모리에 저장되는 데이터의 크기를 설정 위와 똑같이 하기위해 word(32비트)
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //DMA 모드는 계속해서 조도센서 값을 읽어와야 하므로 Circular로 설정
    DMA_InitStructure.DMA_Priority = DMA_Priority_High; //전송 우선순위는 높게
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; //메모리 간 직접 전송 모드 비활성화
    DMA_Init(DMA1_Channel1, &DMA_InitStructure); //설정한 DMA롤 통해 DMA1의 채널1 초기화 및 활성화
    DMA_Cmd(DMA1_Channel1, ENABLE);
}
/*  Github
    1. DMA_PeripheralBaseAddr은 DMA 채널의 주변 장치의 기본 주소를 지정하는 변수이다. 
    2. DMA_MemoryBaseAddr은 DMA 채널의 메모리 기본 주소를 지정한다. 
    3. DMA_DIR은 주변 장치가 소스(source)인지 대상(destination)인지 지정하는 변수이다. 
    4. DMA_BufferSize는 지정된 채널의 버퍼 사이즈를 데이터 단위로 지정하는데, 
    데이터 단위는 DMA_PeripheralDataSize에 설정된 구성과 동일하다.
    5. DMA_PeripheralInc는 주변기기의 주소 레지스터가 증가할지 여부를 지정한다.
    6. DMA_MemoryInc는 메모리 주소 레지스터가 증가할지 여부를 지정한다.
    7. DMA_PeripheralDataSize는 주변기기의 데이터 width를 지정한다.
    8. DMA_MemoryDataSize는 메모리의 데이터 width를 지정한다.
    9. DMA_Mode는 DMA 채널의 작동 모드를 지정한다.
    10. DMA_Priority는 DMA 채널에 대한 소프트웨어 우선 순위를 지정한다.
    11. DMA_M2M은 DMA 채널이 memory to memory 전송에 사용되는지 여부를 지정한다.
*/   


void ADC_Configuration() {
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE; // ADC가 연속해서 변환을 수행함
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; 
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1; // 변환할 채널 수
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5); // ADC_Channel_8의 변환이 수행됨
    ADC_Init(ADC1, &ADC_InitStructure); // ADC1 초기화

    // 기존 10주차 GPIO_Configure (GPIO (General Purpose Input/Output) 핀을 구성),
    // ADC_Configure (아날로그-디지털 변환기(ADC)를 설정) 하는 코드 사용
    // Interupt가 아니라 DMA를 사용해야하므로 DMACmd함수 사용
    ADC_DMACmd(ADC1, ENABLE); // ADC가 DMA를 사용하여 데이터를 전송하도록 설정.
    // DMA 모드를 활성화하여 변환된 데이터를 DMA를 통해 직접 메모리로 전송하도록 설정합니다.
    ADC_Cmd(ADC1, ENABLE);
    ADC_ResetCalibration(ADC1); // ADC의 내부 상태를 초기화하여, 새로운 캘리브레이션 작업이 시작될 수 있도록 준비
    // ResetCalibration 이 과정은 ADC의 정확성을 높이기 위해 필요하며, 보통 ADC 하드웨어가 처음 초기화되거나 정확도 재조정이 필요할 때 수행
    while (ADC_GetResetCalibrationStatus(ADC1)); // 캘리브레이션 리셋이 완료됐는지 확인
    ADC_StartCalibration(ADC1);// 실제 캘리브레이션을 시작하며, 이 과정은 ADC가 더 정확한 값을 측정할 수 있도록 내부의 오차를 보정하는 작업임
    /*리셋이 완료되면 ADC_StartCalibration()을 호출하여 새로 캘리브레이션을 수행하며, 이 과정을 통해 ADC는 변화하는 전압 입력을 보다 정밀하게 디지털 값으로 변환할 수 있습니다.*/
    while (ADC_GetCalibrationStatus(ADC1));
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

int main() {
    SystemInit();
    RCC_Configure(); 
    GPIO_Configure();
    LCD_Init();
    DMA_Configuration();
    ADC_Configuration();
    LCD_Clear(WHITE);
    while (1) {
        while (DMA_GetFlagStatus(DMA1_FLAG_TC1) == RESET) ; // DMA 전송 완료까지 대기
        DMA_ClearFlag(DMA1_FLAG_TC1); // Flag clear
        if(ADC_Value[0] < threshold) { //조도센서의 값이 threshold(1000)보다 낮을 시
            LCD_Clear(WHITE);
            LCD_ShowNum(50, 50, ADC_Value[0], 4, BLACK, WHITE); //배경을 흰색으로
        }
        else if(ADC_Value[0] >= threshold){//조도센서의 값이 threshold(1000)보다 높을 시
            LCD_Clear(GRAY);
            LCD_ShowNum(50, 50, ADC_Value[0], 4, BLACK, GRAY); //배경을 회색으로
        }
    }
    return 0;
}
//평소에는 1300 ~ 1500, 플래시를 비추면 200 ~ 500정도의 조도센서 값이 나왔고, 중간정도의 값인 700을 threshold로 하였다.

/*
void NVIC_Configure(void) {
  NVIC_InitTypeDef NVIC_InitStructure;
  // TODO: fill the arg you want
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  // ADC1
  //@ NVIC Line ADC1
  NVIC_EnableIRQ(ADC1_2_IRQn);
  NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // TODO
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; // TODO
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
*/