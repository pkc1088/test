/**
 * @brief LED PWM PA1 - PC12
 */

#include <stm32f10x.h>

volatile unsigned int Timer2_Counter = 0;

void init_port(void) {
    GPIO_InitTypeDef PORTA;
    GPIO_InitTypeDef PORTC;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);

    PORTA.GPIO_Pin = GPIO_Pin_1;
    PORTA.GPIO_Mode = GPIO_Mode_AF_PP;
    PORTA.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &PORTA);

    PORTC.GPIO_Pin = GPIO_Pin_12;
    PORTC.GPIO_Mode = GPIO_Mode_Out_PP;
    PORTC.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOC, &PORTC);
}

void TIM2_IRQHandler(void) {
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  // Clear the interrupt flag
        Timer2_Counter++;
        GPIOC->BRR = GPIO_Pin_12; 
    }

    if (TIM_GetITStatus(TIM2, TIM_IT_CC2) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);
        GPIOC->BSRR = GPIO_Pin_12; 
    }
}

void init_Timer2() {
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef OutputChannel;

    /* TIM2 Clock Enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    /* Enable TIM2 Global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

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
    TIM_ITConfig(TIM2, TIM_IT_Update | TIM_IT_CC2, ENABLE);  // Interrupt enable
}

// 필요 없는 듯
void make_pwm(u16 val) {
    TIM_OCInitTypeDef OutputChannel;

    OutputChannel.TIM_OCMode = TIM_OCMode_PWM1;
    OutputChannel.TIM_OutputState = TIM_OutputState_Enable;
    OutputChannel.TIM_OutputNState = TIM_OutputNState_Enable;
    OutputChannel.TIM_Pulse = val;
    OutputChannel.TIM_OCPolarity = TIM_OCPolarity_Low;
    OutputChannel.TIM_OCNPolarity = TIM_OCNPolarity_High;
    OutputChannel.TIM_OCIdleState = TIM_OCIdleState_Set;
    OutputChannel.TIM_OCNIdleState = TIM_OCIdleState_Reset;
    TIM_OC2Init(TIM2, &OutputChannel);
}

void delay(unsigned int del) {
    Timer2_Counter = 0;
    while (Timer2_Counter < del);
}

int main() {
    u16 i;
    SystemInit();
    init_port();
    init_Timer2();

    while (1) {
        for (i = 0; i < 100; i++) {
            TIM2->CCR2 = i;
            delay(100);
        }

        for (i = 98; i > 0; i--) {
            TIM2->CCR2 = i;
            delay(100);
        }
    }
}