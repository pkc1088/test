/**
 * @brief 9주차 Interrupt 코드
*/
//ctrl shift p
//C/C++: Disable Error Squiggles
//C/C++: Enable Error Squiggles

#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"

#include "misc.h"

/* function prototype */
void RCC_Configure(void);
void GPIO_Configure(void);
void EXTI_Configure(void);
void USART1_Init(void);
void NVIC_Configure(void);

void EXTI15_10_IRQHandler(void);

void Delay(void);

void sendDataUART1(uint16_t data);

//+++++LED 점등 물결방향 변경 Button 1,2랑 putty를 통한 a,b 입력 시 호출할 예정
void Direction_Set(uint16_t data);
//+++++초기 방향을 1로 설정 1->2->3->4
uint16_t Direction = 1;

//---------------------------------------------------------------------------------------------------

void RCC_Configure(void)
{
   // TODO: Enable the APB2 peripheral clock using the function 'RCC_APB2PeriphClockCmd'
   
   /* UART TX/RX port clock enable */
    //+++++Tx/Rx는 GPIO A port 이므로 GPIOA를 이용
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
   /* Button S1, S2, S3 port clock enable 맞음*/
    //+++++버튼 1,2,3는 순서대로 PC4, PB10, PC13 즉, port B와 C를 사용
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
   /* LED port clock enable 맞음*/
    //+++++LED는 PORT D 사용
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
   /* USART1 clock enable */
    //+++++USART1은 GPIO가 아니므로 고유 USART1 사용
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
   /* Alternate Function IO clock enable */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}

void GPIO_Configure(void)
{
    GPIO_InitTypeDef GPIO_InitStructure; //uint16_t GPIO_Pin; GPIOSpeed_TypeDef GPIO_Speed; GPIOMode_TypeDef GPIO_Mode;의 멤버변수 가짐

   // TODO: Initialize the GPIO pins using the structure 'GPIO_InitTypeDef' and the function 'GPIO_Init'
   
    /* Button KEY1, KEY2, KEY3 pin setting */
    //+++++Button 123 Pin 번호가 각각 4 10 13 이므로 GPIO_Pin을 다음과 같이 설정
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_10 | GPIO_Pin_13;
    //+++++Mode를 pull-up 설정
    // Input Pull-Up 모드를 설정하여 버튼 핀들이 기본적으로 HIGH 상태에 있고, 
    // 버튼이 눌리면 LOW 상태가 되도록 합니다.
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    //+++++Init을 통해 GPIOC, B 설정 적용
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    

    /* LED pin setting*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
   

    /* UART pin setting */
    // TX (PA9)
    //+++++UART TX port 번호는 9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    //+++++UART TX Mode로 Alternate function output 사용
    //TX를 Alternate Function Push-Pull 모드를 사용해 UART의 출력으로 설정합니다.
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    // RX (PA10)
    //+++++UART RX port 번호는 10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    //+++++UART TX Mode로 input floating 사용
    //GPIO_Mode_IN_FLOATING은 외부 신호를 그대로 입력받아 고유 상태를 유지하는 모드입니다.
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);


    /*
    이런식으로 pa9 넣으려면 pin_9을 초기화구조체에 셋팅하고 그걸
    pa에 연결하는 듯

    <GPIO_Mode_AF_PP>
    이 모드는 GPIO 핀이 "대체 기능"을 사용하며, "푸시-풀" 방식으로 출력되는 상태를 설정합니다.
    대체 기능(Alternate Function)은 GPIO 핀이 기본 입력/출력이 
    아닌 특정 기능을 수행하도록 설정하는 모드입니다. 
    예를 들어, UART, I2C, SPI 등의 통신 인터페이스에서 GPIO 핀이 TX, RX, SCL, MISO 등의 
    역할을 하도록 할 때 이 모드를 사용합니다.(이게 인터럽트 걸리면 바로 전환되는 요소인듯)
    Push-Pull 출력: 푸시-풀 출력은 핀이 High(1)와 Low(0) 
    전압을 적극적으로 구동할 수 있도록 해줍니다. 
    이를 통해 강한 출력 신호를 만들어 전압 신호의 안정성을 높입니다.

    <GPIO_Mode_IN_FLOATING>
    의미: 이 모드는 GPIO 핀이 "입력" 모드로 설정되며, "플로팅" 상태로 동작합니다.
    설명: 입력 핀이 플로팅 상태로 동작할 때는, 핀이 외부 신호를 입력받을 수 있으나, 
    외부 회로가 연결되지 않으면 핀이 불안정하게 떠다니는 상태가 됩니다. 
    불안정 상태에서 노이즈 영향을 받을 수 있으므로, 
    외부 풀업(pull-up) 또는 풀다운(pull-down) 저항을 사용하는 것이 좋습니다.
    용도 예시: UART 수신(RX) 핀이나 입력 버튼에서 사용되며, 입력 신호가 특정 기준 없이 
    유동적으로 변동될 수 있습니다. 
    
    Push-Pull 모드는 출력 모드 중 하나로, 핀의 상태를 HIGH와 LOW로 명확하게 출력할 수 있습니다. 
    이 모드에서는 출력 핀이 HIGH일 때 내부에서 VCC에 연결되고, 
    LOW일 때는 GND에 연결되어 강력한 전압 레벨을 제공합니다.
    */
}

void EXTI_Configure(void)//사용할 EXTILine 을 어떤 설정으로 Enable 할 것인지 결정
{
    EXTI_InitTypeDef EXTI_InitStructure;

   // TODO: Select the GPIO pin (Joystick, button) used as EXTI Line using function 'GPIO_EXTILineConfig'
   // TODO: Initialize the EXTI using the structure 'EXTI_InitTypeDef' and the function 'EXTI_Init'
   
    /* Button 1(PC4) is pressed */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource4);
    EXTI_InitStructure.EXTI_Line = EXTI_Line4;//EXTI Line을 Pin 번호와 같은 4로 설정
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//+++++Poilling이 아닌 Interrupt 사용
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    //+++++ Button 2 (PB10)
    //+++++ Pin Num 10이므로 이를 참고하여 똑같이 설정.
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource10);
    EXTI_InitStructure.EXTI_Line = EXTI_Line10; // pb '10'이니까
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // Button 3 (PC13)
    //+++++ Pin Num 13이므로 이를 참고하여 똑같이 설정.
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource13);
    EXTI_InitStructure.EXTI_Line = EXTI_Line13;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

   // NOTE: do not select the UART GPIO pin used as EXTI Line here
}

void USART1_Init(void)
{
   USART_InitTypeDef USART1_InitStructure;

   // Enable the USART1 peripheral
   USART_Cmd(USART1, ENABLE);
   
   // TODO: Initialize the USART using the structure 'USART_InitTypeDef' and the function 'USART_Init'
    //+++++USART의 BaudRate 설정 기본값인 9600으로 설정 
    USART1_InitStructure.USART_BaudRate = 9600;
    //+++++WordLength, Stopbits, Parity 설정 각각 8bit, 1, 안씀
    USART1_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART1_InitStructure.USART_StopBits = USART_StopBits_1;
    USART1_InitStructure.USART_Parity = USART_Parity_No;
    USART1_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART1_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &USART1_InitStructure);

   // TODO: Enable the USART1 RX interrupts using the function 'USART_ITConfig' 
   //and the argument value 'Receive Data register not empty interrupt'
   USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
   //수신 데이터 레지스터가 비어 있지 않을 때 발생하는 인터럽트인 RXNE. 
   //즉, 수신된 데이터가 있을 때 인터럽트를 발생시키는 설정
   //데이터 수신 시 RX 인터럽트가 발생됨.
}

void NVIC_Configure(void) {//각 Interrupt 의 우선순위를 설정

    NVIC_InitTypeDef NVIC_InitStructure;
    
    // TODO: fill the arg you want
    //+++++PriotyGroup 2로 설정
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // CMSIS - stm32flox.h에 있음
    /*
    NVIC_PriorityGroup_2는 Preemption Priority에 2비트, 
    SubPriority에 2비트를 사용하여 우선순위를 세분화하는 것입니다.
    */

    // TODO: Initialize the NVIC using the structure 'NVIC_InitTypeDef' and the function 'NVIC_Init'
    // Button S1
    //+++++밑의 UART1의 설정을 참고
    //EXTI Line을 설정함 이를 통해 이후 Handler랑 연결
    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn; //pc4이니까
    //EXTI4_IRQn은 S1 버튼에 연결된 외부 인터럽트로, 
    //이 라인을 설정하여 EXTI(외부 인터럽트) 핸들러와 연결합니다
    //+++++실행우선순위 설정 UART보다 낮게 하기 위해 2로 설정
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    //+++++대기 우선순위 설정
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // Button S2, S3
    /*
    EXTI15_10_IRQn는 S2와 S3 버튼에 연결된 외부 인터럽트입니다. 
    S2, S3 인터럽트가 동일한 IRQ 라인을 공유하므로 동일한 Preemption Priority와 SubPriority가 적용됩니다.
    */
    // 질문 : button s1과 우선순위가 달라야 동시 처리 문제가 없지 않나?
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn; //pb10과 pc13이니까
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // UART1
    // 'NVIC_EnableIRQ' is only required for USART setting
    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    //+++++Button보다 높게 하기 위해 PreemptionPriority와 SubPriority 둘 다 1로 설정
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // TODO
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; // TODO
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    /*
    각 Interrupt 의 우선순위를 설정. 
    Nested Vectored Interrupt Controller (NVIC)를 설정하는 함수입니다. 
    여기서는 버튼과 USART1의 인터럽트 우선순위를 설정하고 있습니다.
    */
}

void USART1_IRQHandler() {// 각 Interrupt 들이 발생 하였을 때 처리할 작업을 정의 
    /*
    USART1_IRQHandler() : Interrupt Request Handler
    USART1으로부터 데이터를 받았을 때 호출되는 인터럽트 핸들러입니다. 
    특정 문자(a나 b)가 수신되면 LED의 물결 방향을 변경합니다.
    */
    uint16_t word;
    if(USART_GetITStatus(USART1, USART_IT_RXNE)!=RESET){
       // the most recent received data by the USART1 peripheral
        word = USART_ReceiveData(USART1);

        // TODO implement
        //+++++Putty를 통해 a 입력 받을 시 direction을 a로 설정 -> 1234
        if (word == 'a') {
         // TODO: Implement A 동작 (예: LED 물결 방향 변경 - 1->2->3->4)
            Direction_Set('a'); 
        //+++++Putty를 통해 b 입력 받을 시 direction을 b로 설정 -> 4321
        } else if (word == 'b') {
         // TODO: Implement B 동작 (예: LED 물결 방향 변경 - 4->3->2->1)
            Direction_Set('b');
        }
        // clear 'Read data register not empty' flag
        USART_ClearITPendingBit(USART1,USART_IT_RXNE);
        /*
        pending bit을 해제하는 역할
        인터럽트가 발생하면 해당 인터럽트의 대기 비트가 설정되어 인터럽트가 처리 대기 중임을 표시합니다. 
        핸들러 함수 내에서 EXTI_ClearITPendingBit()를 호출해 해당 인터럽트 라인의 대기 비트를 
        해제해야 같은 인터럽트가 반복해서 발생할 수 있게 합니다.
        이 함수를 호출하지 않으면 해당 IRQ는 대기 상태로 남아 있어 이후에 동일한 인터럽트 요청이 
        발생해도 핸들러가 재실행되지 않습니다.
        */
    }
}

//+++++버튼 2,3이 눌렸을 때의 Handler
/*
EXTI line 15~10을 쓸 때 호출되는 핸들러인듯 
pb10(btn 2), pc13(btn 3)은 10과 13라인이니까 해당됨
*/
void EXTI15_10_IRQHandler(void) { // when the button is pressed
    if (EXTI_GetITStatus(EXTI_Line10) != RESET) {
        //+++++버튼 2 입력 시 물결 방향을 b로 설정 -> 4321
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == Bit_RESET) { // GPIO'B'에 10번 즉 pb10을 말함
            Direction_Set('b');
        }
        EXTI_ClearITPendingBit(EXTI_Line10);
    }
    //+++++ 버튼 3 입력 시 sendDataUart1 함수를 통해 Putty에 문자열 출력
    if (EXTI_GetITStatus(EXTI_Line13) != RESET) {
        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == Bit_RESET) {
            char msg[] = "TEAM11\r\n";
            for(int i=0; i <= 14; i++) 
                sendDataUART1(msg[i]); // 굳이 14번?
        }
        EXTI_ClearITPendingBit(EXTI_Line13);
    }
}

//+++++버튼 1이 눌렸을 때의 Handler
/*
EXTI line4 용 핸들러
*/
void EXTI4_IRQHandler(void) {

    //+++++버튼 1이 눌리면 Direction을 a로 설정 1234
    if (EXTI_GetITStatus(EXTI_Line4) != RESET) {
      if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == Bit_RESET) {
         Direction_Set('a');
      }
      EXTI_ClearITPendingBit(EXTI_Line4); // pc4 인듯 button 1
   }
}

void Direction_Set(uint16_t data){
    if(data == 'a'){
        Direction = 1;
    }
    else if(data == 'b'){
        Direction = 2;
    }
}

// TODO: Create Joystick interrupt handler functions
void Delay(void) {
   int i;
   for (i = 0; i < 1000000; i++) {}
}

void sendDataUART1(uint16_t data) {
   /* Wait till TC is set */
   while ((USART1->SR & USART_SR_TC) == 0);
   USART_SendData(USART1, data);
}
// state = 1/2/3, is_interrupted = false
int main(void)
{

    SystemInit();

    RCC_Configure();

    GPIO_Configure();

    EXTI_Configure();

    USART1_Init();

    NVIC_Configure();

    while (1) {
       // TODO: implement 
        //Global 변수인 Direction에 맞춰서 LED 차례로 점등
        if(Direction == 1){
            GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_RESET);
            GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_RESET);
            GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_RESET);
            GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_RESET);

             // LED 1 (PD2)
            GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_RESET); 
            GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_SET);
            GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_SET);
            GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_SET);
            Delay();

            // LED 2 (PD3)
            GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_RESET); 
            GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET);
            GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_SET);
            GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_SET);
            Delay();
            
            // LED 3 (PD4)
            GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_RESET); 
            GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET);
            GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_SET);
            GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_SET);
            Delay();
            
            // LED 4 (PD7)
            GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_RESET); 
            GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET);
            GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_SET);
            GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_SET);
            Delay();
        }
        else if(Direction == 2){
            GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_RESET);
            GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_RESET);
            GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_RESET);
            GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_RESET);

            // LED 4 (PD7)
            GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_RESET); 
            GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET);
            GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_SET);
            GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_SET);
            Delay();
            // LED 3 (PD4)
            GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_RESET); 
            GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET);
            GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_SET);
            GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_SET);
            Delay();
            // LED 2 (PD3)
            GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_RESET); 
            GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET);
            GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_SET);
            GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_SET);
            Delay();
            // LED 1 (PD2)
            GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_RESET); 
            GPIO_WriteBit(GPIOD, GPIO_Pin_3, Bit_SET);
            GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_SET);
            GPIO_WriteBit(GPIOD, GPIO_Pin_7, Bit_SET);
            Delay();
        }
        // Delay
        Delay();
    }
    return 0;
}