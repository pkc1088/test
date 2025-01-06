/**
 * @brief 7주차 Clock Tree 코드
 * 
 */

#include "stm32f10x.h"

void SysInit(void) {
    /* Set HSION bit */
    /* Internal Clock Enable */
    RCC->CR |= (uint32_t)0x00000001; //HSION

    /* Reset SW, HPRE, PPRE1, PPRE2, ADCPRE and MCO bits */
    RCC->CFGR &= (uint32_t)0xF0FF0000;

    /* Reset HSEON, CSSON and PLLON bits */
    RCC->CR &= (uint32_t)0xFEF6FFFF;

    /* Reset HSEBYP bit */
    RCC->CR &= (uint32_t)0xFFFBFFFF;

    /* Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE/OTGFSPRE bits */
    RCC->CFGR &= (uint32_t)0xFF80FFFF;

    /* Reset PLL2ON and PLL3ON bits */
    RCC->CR &= (uint32_t)0xEBFFFFFF;

    /* Disable all interrupts and clear pending bits  */
    RCC->CIR = 0x00FF0000;

    /* Reset CFGR2 register */
    RCC->CFGR2 = 0x00000000;

    // ref 8.3.13에 다 나옴
}

void SetSysClock(void) {
    volatile uint32_t StartUpCounter = 0, HSEStatus = 0;
    /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration ---------------------------*/
    /* Enable HSE */
    RCC->CR |= ((uint32_t)RCC_CR_HSEON);
    /* Wait till HSE is ready and if Time out is reached exit */
    do {
        HSEStatus = RCC->CR & RCC_CR_HSERDY;
        StartUpCounter++;
    } while ((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

    if ((RCC->CR & RCC_CR_HSERDY) != RESET) {
        HSEStatus = (uint32_t)0x01;
    }
    else {
        HSEStatus = (uint32_t)0x00;
    }

    if (HSEStatus == (uint32_t)0x01) {
        /* Enable Prefetch Buffer */
        FLASH->ACR |= FLASH_ACR_PRFTBE;
        /* Flash 0 wait state */
        FLASH->ACR &= (uint32_t)((uint32_t)~FLASH_ACR_LATENCY);
        FLASH->ACR |= (uint32_t)FLASH_ACR_LATENCY_0;

//@TODO - 1 Set the clock
        // ref 8.3.13 table19에 나옴
        // sysinit 함수에서 비트 다 클리어 시켰으니 이제 셋팅하면됨
        /* HCLK = SYSCLK */
        RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1; //AHB 프리스케일러(AHB Prescaler)를 의미
        /*
        따라서 이 코드는 AHB 클럭의 분주비를 1로 설정하여, 시스템 클럭(HCLK)이 
        APB나 다른 버스에 적용될 때 나누어지지 않고 그대로 사용되도록 설정하는 것입니다. 
        결과적으로 AHB 버스의 클럭 속도는 시스템 클럭과 동일하게 유지됩니다.
        */
        /* PCLK2 = HCLK / 2, use PPRE2 */
        RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE2_DIV2; 
        //APB2 클럭을 HCLK의 절반(28/2 = 14mhz)으로 설정하여, APB2에 연결된 주변장치들이 시스템 클럭의 1/2 속도로 동작하게 만듭니다.
        /* PCLK1 = HCLK */
        RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV1; //APB1 클럭을 HCLK와 동일하게 설정하여 APB1에 연결된 주변장치들이 시스템 클럭과 동일한 속도로 동작

        /* Configure PLLs ------------------------------------------------------*/
       

        RCC->CFGR2 &= (uint32_t)~(RCC_CFGR2_PREDIV2 | RCC_CFGR2_PLL2MUL 
                               | RCC_CFGR2_PREDIV1 | RCC_CFGR2_PREDIV1SRC);
        /* 
        해당 비트를 0으로 리셋함 (클리어 하는 행위)
        (PLL2로 들어가는 클럭을 분할하는 프리스케일러,  PLL2의 클럭 배수
         PLL로 들어가는 클럭을 분할하는 프리스케일러, PLL로 들어가는 클럭의 소스를 선택)
        */
        RCC->CFGR2 |= (uint32_t)(RCC_CFGR2_PREDIV2_DIV10 | RCC_CFGR2_PLL2MUL8 
                               | RCC_CFGR2_PREDIV1SRC_PLL2 | RCC_CFGR2_PREDIV1_DIV5);
        /* 
        해당 비트를 1로 셋함
        (PLL2로 들어가는 클럭(HSE인 25mhz임)을 10으로 나눔, PLL2의 클럭을 8배로 증가시킴
         PLL로 들어가는 클럭의 소스(PREDIV1SRC)로 PLL2가 선택됨, PLL로 들어가는 클럭(방금 선택된 PLL2)을 5로 나눔)
        나누기 10, 곱하기 8, MUX 중 PLL 2 선택, 나누기 5 
        기본 25 / 10 x 8 / 5 = 4mhz

        25 / 5 x 4 / 5 = 4 이거도 됨.
        */

        RCC->CFGR &= (uint32_t)~( RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL);
        /* 
        해당 비트를 0으로 리셋함 (클리어하는 행위)
        (PLL로 들어가는 클럭의 소스 선택, PLL의 배수 값 리셋)
        */
        RCC->CFGR |= (uint32_t)( RCC_CFGR_PLLSRC_PREDIV1 | RCC_CFGR_PLLMULL7);
        /*
        해당 비트를 1로 셋함
        (PREDIV1에서 분할된 클럭을 PLL 입력으로 사용, PLL 클럭을 7배로 설정)
        위에 4mhz를 선택해서 x7 하면 28mhz 나옴. 이게 그림 상 PLLCLK가 되어서 SW MUX에서 선택됨.
        */

        /*
        이 코드 블록은 STM32에서 PLL과 프리스케일러를 설정하여 클럭 소스와 배수를 조정하는 
        코드입니다. 이를 통해 시스템 클럭의 주파수를 필요한 만큼 조정할 수 있습니다. 
        각 프리스케일러와 PLL 배수값이 조정되면, 마이크로컨트롤러는 설정된 클럭 주파수를 
        사용하여 주변 장치나 시스템을 동작시킵니다.
        */

//@End of TODO - 1

        /* Enable PLL2 */
        RCC->CR |= RCC_CR_PLL2ON;
        /* Wait till PLL2 is ready */
        while ((RCC->CR & RCC_CR_PLL2RDY) == 0)
        {
        }
        /* Enable PLL */
        RCC->CR |= RCC_CR_PLLON;
        /* Wait till PLL is ready */
        while ((RCC->CR & RCC_CR_PLLRDY) == 0)
        {
        }
        /* Select PLL as system clock source */
        RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
        RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;
        /* Wait till PLL is used as system clock source */
        while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)0x08)
        {
        }
        /* Select System Clock as output of MCO */

//@TODO - 2 Set the MCO port for system clock output

        RCC->CFGR &= ~(uint32_t)RCC_CFGR_MCO; // MCO 신호 클리어
        RCC->CFGR |= RCC_CFGR_MCO_SYSCLK;   //MCO(PA8) 신호를 system clock으로 설정함

//@End of TODO - 2
    
    }
    else {
        /* If HSE fails to start-up, the application will have wrong clock
        configuration. User can add here some code to deal with this error */
    }
}

void RCC_Enable(void) {
//@TODO - 3 RCC Setting

    /*---------------------------- RCC Configuration -----------------------------*/
    /* GPIO RCC Enable  */
    /* User S1 Button RCC Enable */ // 유저 버튼을 port C로 쓰려는듯?
    /* UART Tx, Rx, MCO port */ // 얘네 PA에 연결되어 있음 PA 9, 10, 8
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPCEN;    // port A, C 활성화
    /* USART RCC Enable */
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;   // USART1 클럭 활성화 ref 8.3.7에 있음

    //PA8 - MCO, USART1 tx - PA9 USART1 rx - PA10, UART4 tx - PC10 rx - PC11
//@End of TODO - 3
}

void PortConfiguration(void) {
//@TODO - 4 GPIO Configuration

    /* Reset(Clear) Port A CRH - MCO, USART1 TX,RX*/
    GPIOA->CRH &= ~(
       (GPIO_CRH_CNF8 | GPIO_CRH_MODE8) |
       (GPIO_CRH_CNF9 | GPIO_CRH_MODE9) |
       (GPIO_CRH_CNF10 | GPIO_CRH_MODE10)
   );
    /* MCO Pin Configuration */
    GPIOA->CRH |= GPIO_CRH_MODE8_1 | GPIO_CRH_CNF8_1; // PA8을 Alternate Function Push-Pull, 2MHz로 설정
    /* USART Pin Configuration */
    GPIOA->CRH |= GPIO_CRH_MODE9_1 | GPIO_CRH_CNF9_1;  // PA9 (USART1 TX) Alternate Function Push-Pull, 2MHz
    GPIOA->CRH |= GPIO_CRH_CNF10_1; // PA10 (USART1 RX) Input floating

    //Reset(Clear) Port C4 CRH - User S1 Button
    //key1에 해당하는 PC4를 user button으로 설정 
    GPIOC->CRL &= ~(GPIO_CRL_CNF4 | GPIO_CRL_MODE4);
    // User S1 Button Configuration 
    GPIOC->CRL |= GPIO_CRL_CNF4_1;

//@End of TODO - 4    
}

void UartInit(void) {
    /*---------------------------- USART CR1 Configuration -----------------------*/
    /* Clear M, PCE, PS, TE and RE bits */
    USART1->CR1 &= ~(uint32_t)(USART_CR1_M | USART_CR1_PCE | USART_CR1_PS | USART_CR1_TE | USART_CR1_RE);
    /* Configure the USART Word Length, Parity and mode ----------------------- */
    
    /* Set the M bits according to USART_WordLength value */
//@TODO - 6: WordLength : 8bit
    USART1->CR1 |= (uint32_t)(USART_WordLength_8b); 
    // 8비트 단어 길이를 나타내며, 이 비트를 CR1 레지스터에 설정하여 데이터 프레임의 길이를 8비트로 지정
   
    /* Set PCE and PS bits according to USART_Parity value */
//@TODO - 7: Parity : None
    USART1->CR1 |= (uint32_t)(USART_Parity_No);
   
    /* Set TE and RE bits according to USART_Mode value */
//@TODO - 8: Enable Tx and Rx
    USART1->CR1 |= (uint32_t)(USART_Mode_Rx | USART_Mode_Tx);


    /*---------------------------- USART CR2 Configuration -----------------------*/
    /* Clear STOP[13:12] bits */
    USART1->CR2 &= ~(uint32_t)(USART_CR2_STOP);
    /* Configure the USART Stop Bits, Clock, CPOL, CPHA and LastBit ------------*/
    USART1->CR2 &= ~(uint32_t)(USART_CR2_CPHA | USART_CR2_CPOL | USART_CR2_CLKEN);


    /* Set STOP[13:12] bits according to USART_StopBits value */
//@TODO - 9: Stop bit : 1bit
    /*---------------------------- USART CR3 Configuration -----------------------*/
    /* Clear CTSE and RTSE bits */
    USART1->CR3 &= ~(uint32_t)(USART_CR3_CTSE | USART_CR3_RTSE);


    /* Configure the USART HFC -------------------------------------------------*/
    /* Set CTSE and RTSE bits according to USART_HardwareFlowControl value */
//@TODO - 10: CTS, RTS : disable
    USART1->CR3 |= USART_HardwareFlowControl_None;


    /*---------------------------- USART BRR Configuration -----------------------*/
    /* Configure the USART Baud Rate -------------------------------------------*/
    /* Determine the integer part */
    /* Determine the fractional part */
//@TODO - 11: Calculate & configure BRR
    USART1->BRR |= 0x1E6;
    /*
    uint32_t brr_value = (apb_clock + (baud_rate / 2)) / baud_rate; // BRR 계산
    USART1->BRR = (uint32_t)brr_value; // BRR 레지스터 설정

    PCLK2가 14MHZ (apb_clock) 이고 Baud rate가 28800이면 brr_value 식에 넣어서 계산하고
    16진수 먹이면 0x1E6 나옴.

    Baud Rate= PCLK / 16×(USARTDIV)
    app_clock: sysclk 또는 pclk2 
    PCLK는 UART가 사용하고 있는 APB2 클락입니다.
    */


    /*---------------------------- USART Enable ----------------------------------*/
    /* USART Enable Configuration */
//@TODO - 12: Enable UART (UE)
    USART1->CR1 |= USART_CR1_UE; // USART 기능 활성화
}

void delay(void){
    int i = 0;
    for(i=0;i<1000000;i++);
}

void SendData(uint16_t data) {
    /* Transmit Data */
   USART1->DR = data;

   /* Wait till TC is set */
   while ((USART1->SR & USART_SR_TC) == 0);
}

int main() {
   int i;
   char msg[] = "Hello Team11\r\n";
   
    SysInit();
    SetSysClock();
    RCC_Enable();
    PortConfiguration();
    UartInit();
   
    // if you need, init pin values here
   
    while (1) {
      //@TODO - 13: Send the message when button is pressed
        if(~GPIOC->IDR & GPIO_IDR_IDR4) { 
            /* 
            GPIOC 포트의 입력 데이터를 읽고 NOT 연산자를 사용하여 IDR의 값을 반전시킵니다. 
            즉, HIGH 상태는 LOW로, LOW 상태는 HIGH로 변환됩니다.이는 버튼이 눌릴 때 LOW 신호를 보내는 경우에 유용합니다.
            GPIO_IDR_IDR4는 GPIOC 4번 핀에 해당하는 입력 데이터 비트를 나타냄 이걸 & 연산해서
            4번 핀이 LOW일 때(즉, 버튼이 눌렸을 때) 전체 표현식의 결과는 참임.
            */ 
           //printf("button clicked!!\n");
            for(i=0; msg[i]!='\n'; i++) {
                SendData(msg[i]);
            }
            SendData('\n'); 
        }
        delay();      
   }

}// end main