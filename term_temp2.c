/**
 * @brief DC Motor
*/
#include "stm32f10x.h"

// RCC 설정
void RCC_Configure(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // GPIOB 클럭 활성화
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); // GPIOC 클럭 활성화
}

// GPIO 설정
void GPIO_Configure(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    // 모터 1: EN1 (PB6), IN1 (PB7)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   // Push-Pull Output
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 모터 2: EN2 (PC0), IN3 (PC1)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   // Push-Pull Output
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

// 딜레이 함수
void delay(uint32_t time) {
    for (uint32_t i = 0; i < time; i++) {
        __NOP();  // No Operation (대기)
    }
}

// 모터 제어 함수
void Motor_Control(int motor, int enable) {
    if (motor == 1) {  // 모터 1 제어
        if (enable) {
            GPIO_WriteBit(GPIOB, GPIO_Pin_6, Bit_SET); // EN1 핀 HIGH (모터 활성화)
            GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_SET); // IN1 핀 HIGH (정방향 고정)
        } else {
            GPIO_WriteBit(GPIOB, GPIO_Pin_6, Bit_RESET); // EN1 핀 LOW (모터 비활성화)
        }
    } else if (motor == 2) {  // 모터 2 제어
        if (enable) {
            GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_SET); // EN2 핀 HIGH (모터 활성화)
            GPIO_WriteBit(GPIOC, GPIO_Pin_1, Bit_SET); // IN3 핀 HIGH (정방향 고정)
        } else {
            GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_RESET); // EN2 핀 LOW (모터 비활성화)
        }
    }
}

int main(void) {
    SystemInit();
    RCC_Configure();
    GPIO_Configure();

    while (1) {
        // 모터 1 제어
        Motor_Control(1, 1);  // 모터 1 활성화
        delay(5000000);
        delay(5000000);
        Motor_Control(1, 0);  // 모터 1 비활성화
        delay(5000000);

        // 모터 2 제어
        Motor_Control(2, 1);  // 모터 2 활성화
        delay(5000000);
        delay(5000000);
        Motor_Control(2, 0);  // 모터 2 비활성화
        delay(5000000);
    }

    return 0;
}
