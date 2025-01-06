/**
 * @brief 4주차 코드 + 주석 
*/
#include "stm32f10x.h"

void delay(){
  int i;
  for(i=0; i<1500000; i++){} // 간단한 지연 함수
}

int main(void)
{
    // 포트 초기화
    *(volatile unsigned int *) 0x40021018 |= 0x3C;
    // 이 명령은 RCC_APB2ENR 레지스터(0x40021018)를 설정하여 GPIO 클럭을 활성화합니다. 
    // IOPDEN, IOPCEN, IOPBEN, IOPAEN비트가 활성화됨. 그래서 A~D가 활성화
    // 0x3C : 0b 0011 1100 (2)

    *(volatile unsigned int *) 0X40011400 = 0X10011100; 
// PORT D(0x4001 1400)에 output 설정 (PD2, PD3, PD4, PD7 활성화 (led1~4에 해당함))

    *(volatile unsigned int *) 0x40010800 = 0X8;    
/*
0x4001 0800는 A포트의 CRL(low)임.
0x8을 이진수로 표현하면 1000이며 ref9.2.1확인해보면 CNF0+MODE0에 해당
CNF가 10이고 MODE가 00이니 인풋모드로 초기화 즉 PA0인 Key4가 초기화됨
*/

    *(volatile unsigned int *) 0x40010C04 = 0X800;     
/*
0x4001 0C04는 B포트의 CRH(high)임.
ref9.2.2 확인해보면 1000이 3번째 위치하니 CNF10+MODE10에 해당함
즉 PB10을 의미하고 CNF가 10(alternate function output push pull)
MODE가 00(input mode (reset state))에 해당
PB10인 Key2를 초기화하는 코드
*/
    *(volatile unsigned int *) 0x40011000 = 0X80000;
/*
ref9.2.1
0x4001 1000은 C포트의 CRL(low)임.
이진수 표현시 1000 0000 0000 0000 0000 임
CNF비트+MODE비트. 1000은 5번째이니 PC4에 해당. 
cnf 10은 input with pull-up/pull-down
mode 00은 input mode(reset state)에 해당
PC4인 Key1을 초기화하는 코드
*/   
    *(volatile unsigned int *) 0x40011004 = 0X800000;   
/*
0x4001 1004는 C포트의 CRH(high)임.
1000 0000 0000 0000 0000 0000 이니 CNF13+MODE13에 해당.
CNF(10)+MODE(00)이니 역시 PC13인 key3을 초기화함. 
*/


    // 모든 LED 초기화 (꺼짐)
    // Address offset: 0x10(BSRR)을 0x40011400 portD에 더한거임.
    // 04, 08, 10, 80은 노트에 정리됨. 해당자리가 1로 바뀌면서 활성화시켜 set하는거임
    // 포트의 핀 설정시 주의사항 레지스터의 사용하려는 부분을 0으로 초기화 후 사용
    *(volatile unsigned int *) 0x40011410 |= 0x04; // PD2 LED 꺼짐
    *(volatile unsigned int *) 0x40011410 |= 0x08; // PD3 LED 꺼짐
    *(volatile unsigned int *) 0x40011410 |= 0x10; // PD4 LED 꺼짐
    *(volatile unsigned int *) 0x40011410 |= 0x80; // PD7 LED 꺼짐
   
    while (1) {
        // 버튼 1번 (PC4) - 2, 4번째 불빛 켜짐 (PD2, PD4)
        if((*(volatile unsigned int *)0x40011008 & 0x00000010) == 0){
            // IDR은 GPIO 포트 C의 입력 데이터를 읽도록 사용됨
            // These bits are read only and can be accessed in Word mode only.
            // Port C : 0x4001 1000 + IDR 0x08h 
            // 0x00000010은 레지스터 값의 4번째 비트
            // 0001 0000 (2) 이니까 (GPIOC 포트의 4번 핀 상태)만 검사하게 됩니다.
            // AND 연산의 결과가 0이면, 해당 비트가 클리어(0) 상태임을 의미합니다. 
            // 즉, GPIOC의 4번 핀이 LOW(버튼이 눌린 상태)임을 뜻합니다.
            // 조건문은 key가 눌린지 확인하니 key1은 pc4니까 port c를 조사하고
            // 실제로 led는 pd4, pd7에 연결되어 있으니 밑 코드로 활성화시켜주는거임.
// 0x4001 1000 + 0x14 더하면 0x4001 1014
            *(volatile unsigned int *) 0x40011414 |= 0x04; // PD2 LED 켜기
            // 0x40011400 portD에 BRR offset 0x14 더한 0x40011414
            // BRR (Bit Reset Register)는 특정 GPIO 핀을 (하위비트기준) 1일 때 리셋(LOW) 상태로 만들 때 사용
            // 이 레지스터에 특정 비트를 쓰면 해당 핀을 0으로 리셋하게 됩니다.
            // 0x04는 PD2 핀을 LOW로 설정합니다.
            // 누가 1로 켜놨으니 1을 인식하고 reset인 0으로 만드는거인듯?
            *(volatile unsigned int *) 0x40011414 |= 0x10; // PD4 LED 켜기
            delay(); 
        }

        // 버튼 2번 (PB10) - 2, 4번째 불빛 꺼짐 (PD2, PD4)
        if((*(volatile unsigned int *)0x40010C08 & 0x00000400) == 0){
            //0x40010C08는 GPIOB(0x4001 0C00) 포트의 IDR (Input Data Register)입니다. 
            //이 레지스터는 GPIOB 포트의 입력 상태를 확인하는 데 사용됩니다.
            //*(volatile unsigned int *)는 이 주소에 있는 값을 읽어옵니다. 
            //이때, volatile 키워드는 메모리 값을 항상 실시간으로 읽도록 강제하는 것으로, 
            //레지스터 값이 바뀔 수 있으니 최적화하지 말라는 의미입니다.
            //0x00000400는 2진수로 0000 0000 0000 0100 0000 0000로, 
            //GPIOB 포트의 10번 핀 (PB10)을 확인하는 비트 마스크입니다.
            //비트 AND 연산을 사용하여 PB10 핀의 상태를 확인하는 과정입니다. 
            //여기서 PB10 핀이 HIGH 상태면 결과가 0x00000400이 되고, 
            //LOW 상태면 결과가 0이 됩니다. 
            //PB10 핀의 상태가 LOW(버튼이 눌린 것)인지 확인하는 조건입니다. 
            //즉, PB10 핀이 LOW 상태, 즉 0일 때만 if 조건문이 참이 됩니다.  
// 0x4001 1000 + 0x10
            //이 코드는 GPIOD 포트(0x4001 1400)의 BSRR(0x10) 레지스터(0x40011410)에 0x04 값을 OR 연산합니다.
            //BSRR는 하위16비트 기준 핀을 1이면 Set할 때 사용하는 레지스터로, 
            //하위 16비트에 비트를 설정하여 해당 핀을 HIGH로 만듬.
            //0x04는 2진수로 0000 0000 0000 0100이므로, PD2 핀과 PD4 핀을 HIGH 상태로 만드는 것입니다.
            //PD2 핀을 high(인듯) 상태로 만들어 PD2에 연결된 LED를 끄는 것을 의미합니다.
            *(volatile unsigned int *) 0x40011410 |= 0x04; // PD2 LED 끄기
            *(volatile unsigned int *) 0x40011410 |= 0x10; // PD4 LED 끄기
            delay(); 
        }


        // 버튼 3번 (PC13) - 1, 3번째 불빛 켜짐 (PD3, PD7)
        if((*(volatile unsigned int *)0x40011008 & 0x00002000) == 0){
            *(volatile unsigned int *) 0x40011414 |= 0x08; // PD3 LED 켜기
            *(volatile unsigned int *) 0x40011414 |= 0x80; // PD7 LED 켜기
            delay(); 
        }

        // 버튼 4번 (PA0) - 1, 3번째 불빛 꺼짐 (PD3, PD7)
        if((*(volatile unsigned int *)0x40010808 & 0x1 ) == 0){
            *(volatile unsigned int *) 0x40011410 |= 0x08; // PD3 LED 끄기
            *(volatile unsigned int *) 0x40011410 |= 0x80; // PD7 LED 끄기
            delay(); // 버튼 누름 확인 딜레이
        }
    }
}
/*중요
싱크(Sink) 방식:
LED의 음극(GND)이 GPIO 핀에 연결되고, 양극(VCC)은 전원에 연결됩니다.
이 경우, GPIO 핀을 LOW로 설정(PD 핀이 0V)하면 전류가 흐르며 LED가 켜집니다.
반대로 GPIO 핀을 HIGH로 설정(PD 핀이 VCC 전압)이면 LED는 꺼집니다.
STM32 보드에서 LED 동작 (STM32F107VCT6):
STM32 개발 보드에서는 보통 싱크(Sink) 방식이 많이 사용됩니다. 즉:
PD 핀이 LOW일 때 LED가 켜집니다.
PD 핀이 HIGH일 때 LED가 꺼집니다.

양극(Anode, +): LED의 긴 다리로, 
보통 저항(Resistor)과 함께 연결되어 있고, 
전원(VCC)에 연결되거나 GPIO 핀에 직접 연결됩니다.
음극(Cathode, -): LED의 짧은 다리로, 
보통 GND(그라운드) 또는 GPIO 핀에 연결됩니다.
*/
