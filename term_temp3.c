#include "stm32f10x.h"

#define ECHO_PIN GPIO_Pin_10     // Echo 핀 (PA10, TIM1 관련 핀)
#define TRIGGER_PIN GPIO_Pin_11  // Trigger 핀 (PA11)

volatile uint32_t pulseWidth = 0;  // Echo 핀의 HIGH 시간
volatile uint8_t echoState = 0;    // Echo 핀 상태 (0: LOW, 1: HIGH)

void GPIO_Configuration(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    // GPIOA 클럭 활성화
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

    // Trigger Pin을 출력으로 설정
    GPIO_InitStructure.GPIO_Pin = TRIGGER_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Echo Pin을 입력으로 설정
    GPIO_InitStructure.GPIO_Pin = ECHO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;  // 풀다운 설정
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // LED 설정 (PD4, PD7)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_SET);
    GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_SET);
}

void Timer_Configuration(void) {
    // TIM1 설정: 1μs 단위로 카운터 동작
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_InitStructure;
    TIM_InitStructure.TIM_Prescaler = 72 - 1;  // 1μs 타이머 주기 설정 (72MHz 클럭)
    TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_InitStructure.TIM_Period = 0xFFFF;  // 최대 타이머 값 설정
    TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM1, &TIM_InitStructure);
    TIM_Cmd(TIM1, ENABLE);
}

void EXTI_Configuration(void) {
    EXTI_InitTypeDef EXTI_InitStructure;

    // Echo 핀 (PA10)에 대한 외부 인터럽트 설정
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource10);
    EXTI_InitStructure.EXTI_Line = EXTI_Line10;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; // 상승/하강 엣지
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // NVIC 설정: 인터럽트 우선순위
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void EXTI15_10_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line10) != RESET) {
        if (GPIO_ReadInputDataBit(GPIOA, ECHO_PIN) == Bit_SET) {
            // Echo가 HIGH로 변했을 때, 타이머를 리셋하여 시간 측정 시작
            TIM_SetCounter(TIM1, 0);        // 타이머 카운터 초기화
            echoState = 1;                  // Echo 신호가 HIGH
        } else {
            // Echo가 LOW로 변했을 때, 타이머 값으로 시간 측정 완료
            pulseWidth = TIM_GetCounter(TIM1);  // 타이머 값 읽기
            echoState = 0;                      // Echo 신호가 LOW
        }
        EXTI_ClearITPendingBit(EXTI_Line10);  // 인터럽트 플래그 클리어
    }
}

void TriggerPulse(void) {
    // Trigger 핀에 짧은 펄스 전송 (10μs)
    GPIO_WriteBit(GPIOA, TRIGGER_PIN, Bit_SET);
    for (volatile int i = 0; i < 720; i++);  // 약 10μs 대기 (72MHz 클럭 기준)
    GPIO_WriteBit(GPIOA, TRIGGER_PIN, Bit_RESET);
}

int main(void) {
    // 초기화
    SystemInit();
    GPIO_Configuration();
    Timer_Configuration();
    EXTI_Configuration();

    GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_SET);
    GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_SET);

    while (1) {
        TriggerPulse();  // 초음파 송신을 위해 Trigger 핀에 펄스 보내기
        for (volatile int i = 0; i < 1000000; i++);  // 대기 (센서 안정화 시간)

        // 거리 계산 (단위: cm)
        float distance = (pulseWidth * 0.0343) / 2;  // 거리 계산 (음속은 약 343m/s)
        if((int)distance < 10) {
            GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_RESET);
        }
        // 거리 출력 (디버깅 환경에서 printf 사용 가능)
        // 예: printf("Distance: %.2f cm\n", distance);
    }
}
