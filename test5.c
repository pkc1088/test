// my code
*(volatile unsigned int *) 0x40011414 |= 0x08; // PD3 LED 켜짐
// jokbo
*(volatile unsigned int *) 0x40011414 ^= 0x08; // PD3 LED 켜짐

<BRR 0x14>
0001 0100 mycode
0000 1000 or 
0001 '1'100 -> BR3인 PD3가 1로 바뀌니 RESET ODR3 명령어 수행 -> 켜짐

0001 0100 jokbo
0000 1000 xor 
0001 '1'100 -> BR3인 PD3가 1로 바뀌니 RESET ODR3 명령어 수행 -> 켜짐


// my code & jokbo
*(volatile unsigned int *) 0x40011410 |= 0x08; // PD3 LED 꺼짐

<BSRR 0x10>
0001 0000 
0000 1000 or 
0001 '1'000 -> BS3인 PD3가 1로 바뀌니 SET ODR3 명령어 수행 -> 꺼짐

