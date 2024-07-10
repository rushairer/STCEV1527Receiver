#include "uart.h"
#include "STC8G.h"

__bit busy;

void Uart1_Init(void) // 9600bps@16MHz
{
    SCON = 0x50;  // 8位数据,可变波特率
    AUXR |= 0x40; // 定时器时钟1T模式
    AUXR &= 0xFE; // 串口1选择定时器1为波特率发生器
    TMOD &= 0x0F; // 设置定时器模式
    TL1  = 0xCC;  // 设置定时初始值
    TH1  = 0xFF;  // 设置定时初始值
    ET1  = 0;     // 禁止定时器中断
    TR1  = 1;     // 定时器1开始计时
    ES   = 1;     // 使能串口1中断
    busy = 0;
}

void UartSend(char dat)
{
    while (busy)
        ;
    busy = 1;
    SBUF = dat;
}

void Uart1_Isr(void) __interrupt(4)
{
    if (TI) {
        TI   = 0; // 清中断标志
        busy = 0;
    }
    if (RI) {
        RI = 0; // 清中断标志
    }
}