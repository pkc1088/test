/**
 * @brief 10주차 BlueTooth 코드 / 분석 중
*/

#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"

#include "misc.h"

/* function prototype */
void RCC_Configure(void);
void GPIO_Configure(void);
void USART1_Init(void);
void USART2_Init(void);
void NVIC_Configure(void);
// PC의 putty 프로그램과 Bluetooth 모듈 간 통신이 가능하도록 펌웨어 작성

/*
'RCC_APB2PeriphClockCmd' 함수를 사용하여 GPIOA, GPIOD, AFIO, 
그리고 USART1의 클록을 활성화합니다.
'RCC_APB1PeriphClockCmd'를 사용하여 USART2의 클록을 활성화합니다.
*/
void RCC_Configure(void)
{  
    // TODO: Enable the APB2 peripheral clock using the function 'RCC_APB2PeriphClockCmd'
    // UART1 tx / rx를 PA9, PA10에 연결할 것이므로 GPIOA를 활성화 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    
    // UART2 tx / rx를 PD5, PD6에 연결할 것이므로 GPIOD를 활성화
    // 원래는 PA2 / PA3이였으나 보드 소음 발생으로 REMAP port 사용
    /*
    RS232 시리얼 케이블과 블루투스를 통해 USART1, USART2 동시 사용 시 보드에서 
    소음(부저음)이 발생할 수 있음 (Default Pin 대신 Remap Pin을 사용하여 문제 해결 가능)
    */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE); // 없애도 됨

    // USART1, USART2 clock enable
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    
    // Alternate Function IO clock enable
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    // AFIO (Alternate Function I/O) 레지스터를 통해 리맵핑 설정을 활성화
}

/*
PIO 핀들을 초기화합니다. USART1의 TX는 PA9, RX는 PA10으로 설정됩니다.
USART2의 TX는 PD5, RX는 PD6으로 설정됩니다. 이는 기본 위치인 PA2/PA3에서 
변경되어 있으며, 이는 핀 리맵 기능을 통해 구현됩니다.
*/
void GPIO_Configure(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // TODO : Initialize the GPIO pins using the structure 'GPIO_InitTypeDef' and the function 'GPIO_Init'
    // USART1 pin setting
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
    
    // USART2 pin setting
    // TX
    // TX를 PD5에 연결 //xxxxPA2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; //GPIO_Pin_2;  // changed
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure); //GPIO_Init(GPIOA, &GPIO_InitStructure);
    // RX
    // RX를 PD6에 연결 //xxxxPA3 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; // GPIO_Pin_3; // changed
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure); // GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*
USART1을 초기화하고, 9600 baud rate로 설정합니다. 데이터 길이는 8비트, 스톱 비트는 1, 
패리티는 없음, 하드웨어 플로우 컨트롤은 사용하지 않습니다.
RX 인터럽트를 활성화하여, 데이터 수신 시 인터럽트가 발생하도록 설정합니다.
*/
void USART1_Init(void)
{
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

/*
USART2도 USART1과 동일한 방식으로 초기화합니다.
여기서도 GPIO_PinRemapConfig를 사용하여 TX와 RX 핀을 PD5와 PD6으로 리맵합니다.
마찬가지로 RX 인터럽트를 활성화합니다.
*/
void USART2_Init(void)
{
    USART_InitTypeDef USART2_InitStructure;
    // Enable the USART2 peripheral
    USART_Cmd(USART2, ENABLE);

    // TODO : Initialize the USART using the structure 'USART_InitTypeDef' and the function 'USART_Init'
    // USART2의 기초설정 BuadRate는 9600으로 설정
    USART2_InitStructure.USART_BaudRate = 9600;
    USART2_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART2_InitStructure.USART_StopBits = USART_StopBits_1;
    USART2_InitStructure.USART_Parity = USART_Parity_No;
    USART2_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART2_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART2, &USART2_InitStructure);

    // Remap 활성화
    GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);

    // TODO : Enable the USART2 RX interrupts using the function 'USART_ITConfig'
    // 설정을 통해 UART2 활성화
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
}

/*
인터럽트 우선 순위를 설정합니다. 'NVIC_PriorityGroupConfig'를 사용하여 인터럽트 그룹을 설정합니다.
USART1과 USART2 인터럽트를 활성화하고 우선 순위를 각각 0과 1로 설정합니다.
*/
void NVIC_Configure(void) {

    NVIC_InitTypeDef NVIC_InitStructure;
    
    // TODO : fill the arg you want
    //NVIC의 인터럽트 우선순위 결정 : 
    //NVIC_PriorityGroup_4 > 4비트로 우선 순위 구성, 서브 그룹도 4비트로 구성
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    // USART1
    // 'NVIC_EnableIRQ' is only required for USART setting
    // UART1 설정 우선순위를 0으로 설정.
    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //TODO
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; //TODO
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // USART2
    // 'NVIC_EnableIRQ' is only required for USART setting
    // UART2 설정 우선순위를 1로 설정.
    NVIC_EnableIRQ(USART2_IRQn);
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //TODO
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; //TODO
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*
USART1에서 RXNE(Read Data Register Not Empty) 인터럽트가 발생할 때 호출됩니다.
받은 데이터를 USART2로 전송합니다.
인터럽트 플래그를 클리어하여 다음 데이터 수신을 준비합니다.
*/
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

/*
USART2에서 RXNE 인터럽트가 발생할 때 호출됩니다.
받은 데이터를 USART1로 전송합니다.
마찬가지로 인터럽트 플래그를 클리어합니다.
*/
void USART2_IRQHandler() {
    uint16_t word;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        // the most recent received data by the USART2 peripheral
        word = USART_ReceiveData(USART2);
        
        // TODO: Implement your code to handle received data
        // 받은 데이터를 UART1로 전달
        USART_SendData(USART1, word);
        // clear 'Read data register not empty' flag
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}
/* 
usart1, 2가 무한루프에 빠지지 않는 이유 : usart1의 경우 word에 저장되는건
사용자가 푸티에 입력한 값일 뿐이다. 이걸 usart1에 보내기만 하는 거고. 
usart1에서 받는 입력값은 블투 터미널에서 사용자가 입력한 값일 뿐이며
이것도 역시 usart1 푸티에 sendData할 뿐인거다. 각 화면에 표시되는건 각자의
앱들에서 처리하는거인듯. 즉 receiveData는 사용자가 입력한 값을 처리하는거지
다른쪽 IRQ에서 보낸 데이터를 word에 저장하는건 아닌듯. 정리하면 USART2_IRQ에서 
USART_SendData(USART1, word);이걸로 usart1에 send를 해도 USART1_IRQ의 word에 
그 값이 읽혀지는건 아니님. usrat1의 word는 푸티 앱에서 사용자가 직접 입력한 
값을 읽는 것 뿐임? 
*/
int main(void)
{
  char msg[] = "abcde\r\n";
  unsigned int i;
  
    SystemInit();

    RCC_Configure();

    GPIO_Configure();

    USART1_Init();      // UART1 for PC
    
    USART2_Init();      // UART2 for Bluetooth

    NVIC_Configure();

    while (1) {
    }
    return 0;
}
