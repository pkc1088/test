/**
 * @brief 6주차 코드 + 주석
*/
#include "stm32f10x.h"
void delay(){
  int i;
  for(i=0; i<2000000; i++){}
}

int main(void)
{
//RCC의 memory mapping(datasheet)보면 0x4002 1000 - 0x4002 13FF 
//RCC_APB2_ENR은 0x4002 1018
    *(volatile unsigned int *) 0x40021018 |= 0x3C; //A,B,C,D 활성화

    *(volatile unsigned int *) 0x40011000 = 0X80000; // PC4, key1
/*
ref9.2.1
0x4001 1000은 C포트의 CRL(low)임.
이진수 표현시 1000 0000 0000 0000 0000 임
CNF비트+MODE비트. 1000은 5번째이니 PC4에 해당. 
cnf 10은 input with pull-up/pull-down
mode 00은 input mode(reset state)에 해당
PC4인 KEY1을 초기화하는 코드
*/

    *(volatile unsigned int *) 0x40010C04 = 0X800; //PB10, key2
/*
0x4001 0C04는 B포트의 CRH(high)임.
ref9.2.2 확인해보면 1000이 3번째 위치하니 CNF10+MODE10에 해당함
즉 PB10을 의미하고 CNF가 10(alternate function output push pull)
MODE가 00(input mode (reset state))에 해당
PB10인 KEY2를 초기화하는 코드
*/

    *(volatile unsigned int *) 0x40011004 = 0X800000; // PC13, key3
/*
0x4001 1004는 C포트의 CRH(high)임.
1000 0000 0000 0000 0000 0000 이니 CNF13+MODE13에 해당.
CNF(10)+MODE(00)이니 역시 PC13을 초기화함. 
*/

    *(volatile unsigned int *) 0x40010800 = 0X8; // PA0, key4

/*
key 실제 연결 : key1(pc4), key2(pb10), key3(pc13), key4(pa0) 이다. 
LED연결은 PD2, PD3, PD4, PD7 이다.
*/

    *(volatile unsigned int *) 0X40011400 = 0X00011000; 
// PD3, PD4 = LED 2, LED 3
//0x08, 0x10, 0x04, 0x80은 각각 핀 3, 4, 2, 7에 해당합니다. 

//다 끄고 시작하고
    *(volatile unsigned int *) 0x40011410 |= 0x08;
    *(volatile unsigned int *) 0x40011410 |= 0x10;
    *(volatile unsigned int *) 0x40011410 |= 0x04;
    *(volatile unsigned int *) 0x40011410 |= 0x80;
 
    while (1) {
      //key1 : 모터 정방향 회전 : led2(pd3) 켜기
      //모터1과 연결
      //Port C : 0x4001 1000 + IDR 0x08h에 0x10인 0001 0000으로 4번째 셋
        if((*(volatile unsigned int *)0x40011008 & 0x00000010) == 0){
            //*(volatile unsigned int *) 0x40011414 |= 0x10; // PD4, LED3번 켜기
            *(volatile unsigned int *) 0x40011410 |= 0x10; // PD4, LED3번 끄기 (일단 역회전 멈춤)
            *(volatile unsigned int *) 0x40011414 |= 0x08; // PD3, LED2번 켜기 (정회전)
            delay();
        }

        //key2 : 모터 역방향 회전 : led3(pd4) 켜기
        //모터2와 연결
        if((*(volatile unsigned int *)0x40010C08 & 0x00000400) == 0){
            //*(volatile unsigned int *) 0x40011410 |= 0x08; // PD3, LED2번 끄기
            //*(volatile unsigned int *) 0x40011414 |= 0x08; // PD3, LED2번 켜기
            *(volatile unsigned int *) 0x40011410 |= 0x08; // PD3, LED2번 끄기 (일단 정회전 멈춤)
            *(volatile unsigned int *) 0x40011414 |= 0x10; // PD4, LED3번 켜기 (역회전)
            delay();
        }

        //key3 : 정방향2초, 역방향2초, 정지 : led2, 2초delay, led3, 2초delay, 후 끄기
        if((*(volatile unsigned int *)0x40011008 & 0x00002000) == 0){
            //*(volatile unsigned int *) 0x40011410 |= 0x10; // PD4, LED3번 끄기
            //*(volatile unsigned int *) 0x40011414 |= 0x10; // PD4, LED3번 켜기
            *(volatile unsigned int *) 0x40011410 |= 0x10; // PD4, LED3번 끄기 (일단 역회전 멈춤)
            *(volatile unsigned int *) 0x40011414 |= 0x08; // PD3, LED2번 켜기 (정회전)
            delay();
            delay();
            *(volatile unsigned int *) 0x40011410 |= 0x08; // PD3, LED2번 끄기 (일단 정회전 멈춤)
            *(volatile unsigned int *) 0x40011414 |= 0x10; // PD4, LED3번 켜기 (역회전)
            delay();
            delay();
            //*(volatile unsigned int *) 0x40011410 |= 0x08; // PD3, LED2번 끄기 (불필요)
            *(volatile unsigned int *) 0x40011410 |= 0x10; // PD4, LED3번 끄기
        }   

        //key4 : 모터 정지 : led2, led3 끄기
        if((*(volatile unsigned int *)0x40010808 & 0x1 ) == 0){
            *(volatile unsigned int *) 0x40011410 |= 0x08; // PD3, LED2번 끄기
            *(volatile unsigned int *) 0x40011410 |= 0x10; // PD4, LED3번 끄기
            delay(); 
        }
    }
}


/*
#include "stm32f10x.h"
void delay(){
  int i;
  for(i=0; i<2000000; i++){}
}
// pc 8 pc 9로 바꿈
int main(void)
{
    // A,B,C,D 활성화 (GPIO 포트)
    *(volatile unsigned int *) 0x40021018 |= 0x3C;

    // Key1 (PC4), Key2 (PB10), Key3 (PC13), Key4 (PA0) 설정
    *(volatile unsigned int *) 0x40011000 = 0X80000; // PC4, key1
    *(volatile unsigned int *) 0x40010C04 = 0X800;   // PB10, key2
    *(volatile unsigned int *) 0x40011004 = 0X800000; // PC13, key3
    *(volatile unsigned int *) 0x40010800 = 0X8;      // PA0, key4

    // PC8, PC9를 출력으로 설정
    *(volatile unsigned int *) 0x40011000 |= 0x03000000; // PC8, 모터 정방향 (LED2)
    *(volatile unsigned int *) 0x40011000 |= 0x0C000000; // PC9, 모터 역방향 (LED3)

    // 모터 정지 상태로 시작 (LED2, LED3 끄기)
    *(volatile unsigned int *) 0x40011010 |= 0x0100; // PC8 끄기 (LED2)
    *(volatile unsigned int *) 0x40011010 |= 0x0200; // PC9 끄기 (LED3)

    while (1) {
        // Key1 : 모터 정방향 회전 (PC8 켜기)
        if((*(volatile unsigned int *)0x40011008 & 0x00000010) == 0){
            *(volatile unsigned int *) 0x40011010 |= 0x0200; // PC9, 역방향 끄기
            *(volatile unsigned int *) 0x40011014 |= 0x0100; // PC8, 정방향 켜기
            delay();
        }

        // Key2 : 모터 역방향 회전 (PC9 켜기)
        if((*(volatile unsigned int *)0x40010C08 & 0x00000400) == 0){
            *(volatile unsigned int *) 0x40011010 |= 0x0100; // PC8, 정방향 끄기
            *(volatile unsigned int *) 0x40011014 |= 0x0200; // PC9, 역방향 켜기
            delay();
        }

        // Key3 : 정방향 2초, 역방향 2초, 정지
        if((*(volatile unsigned int *)0x40011008 & 0x00002000) == 0){
            *(volatile unsigned int *) 0x40011010 |= 0x0200; // PC9 끄기 (역방향)
            *(volatile unsigned int *) 0x40011014 |= 0x0100; // PC8 켜기 (정방향)
            delay();
            delay();
            *(volatile unsigned int *) 0x40011010 |= 0x0100; // PC8 끄기 (정방향)
            *(volatile unsigned int *) 0x40011014 |= 0x0200; // PC9 켜기 (역방향)
            delay();
            delay();
            *(volatile unsigned int *) 0x40011010 |= 0x0200; // PC9 끄기 (역방향)
        }

        // Key4 : 모터 정지 (PC8, PC9 끄기)
        if((*(volatile unsigned int *)0x40010808 & 0x1 ) == 0){
            *(volatile unsigned int *) 0x40011010 |= 0x0100; // PC8 끄기 (정방향)
            *(volatile unsigned int *) 0x40011010 |= 0x0200; // PC9 끄기 (역방향)
            delay();
        }
    }
}


*/