#include "uart.h"
#include "STC8G.h"

__bit busy;

void Uart1_Init(void) // 9600bps@16MHz
{
    SCON = 0x50;  // 8λ����,�ɱ䲨����
    AUXR |= 0x40; // ��ʱ��ʱ��1Tģʽ
    AUXR &= 0xFE; // ����1ѡ��ʱ��1Ϊ�����ʷ�����
    TMOD &= 0x0F; // ���ö�ʱ��ģʽ
    TL1  = 0xCC;  // ���ö�ʱ��ʼֵ
    TH1  = 0xFF;  // ���ö�ʱ��ʼֵ
    ET1  = 0;     // ��ֹ��ʱ���ж�
    TR1  = 1;     // ��ʱ��1��ʼ��ʱ
    ES   = 1;     // ʹ�ܴ���1�ж�
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
        TI   = 0; // ���жϱ�־
        busy = 0;
    }
    if (RI) {
        RI = 0; // ���жϱ�־
    }
}