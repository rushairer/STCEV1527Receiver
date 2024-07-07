#include "STC8G.h"
#include "config.h"

__bit busy;

#define CPUIDBASE 0xfde0
#define ID_ADDR   ((unsigned char volatile __xdata *)(CPUIDBASE + 0x00))

void UartSend(char dat);

void Time0_init(void) // 100΢��@16MHz
{
    AUXR |= 0x80; // ��ʱ��ʱ��1Tģʽ
    TMOD &= 0xF0; // ���ö�ʱ��ģʽ
    TL0 = 0xC0;   // ���ö�ʱ��ʼֵ
    TH0 = 0xF9;   // ���ö�ʱ��ʼֵ
    TF0 = 0;      // ���TF0��־
    TR0 = 1;      // ��ʱ��0��ʼ��ʱ
    ET0 = 1;      // ʹ�ܶ�ʱ��0�ж�
}

uint8_t RFPinValue(void)
{
    return RF_PIN == 1;
}

// ���ȼ������½��ش���
void Timer0_Isr(void) __interrupt(1)
{
    if (RF_PIN) {
        HTime++;
    } else {
        LTime++;
    }

    if (RFCount >= 23) {
        RFCount      = 0;
        SyncCodeFlag = 0;
        if (Synced) {
            EV1527Value1 = EV1527Code;
            EV1527Code   = 0;
        } else {
            EV1527Value2 = EV1527Code;
            EV1527Code   = 0;
        }

        if (EV1527Value1 == EV1527Value2) {
            IsRFSuccess = 1;
            EV1527Value = EV1527Value1;
        }

        Synced = !Synced;
    }
}

void INT2_init(void)
{
    INTCLKO = 0x10; // ʹ��INT2�ж�
}

// �½��ش���
void INT2_Isr(void) __interrupt(10)
{
    HData = HTime;
    LData = LTime;

    HTime = 0;
    LTime = 0;

    // 124:4
    if (((LData >= 80) && (LData <= 150)) && (HData >= 2) && (HData <= 6)) {
        // ������� �ж��Ƿ�ͬ����
        SyncCodeFlag = 1;
    } else if ((SyncCodeFlag == 1) && ((HData >= 8) && (HData <= 14))) {
        // ������� 1
        EV1527Code = EV1527Code << 1 + 1;
        RFCount++;
    } else if ((SyncCodeFlag == 1) && ((HData >= 2) && (HData <= 6))) {
        // ������� 0
        EV1527Code = EV1527Code << 1;
        RFCount++;
    }

    IE0 = 0;
}

void Uart1_Init(void) // 9600bps@16MHz
{
    SCON = 0x50;  // 8λ����,�ɱ䲨����
    AUXR |= 0x40; // ��ʱ��ʱ��1Tģʽ
    AUXR &= 0xFE; // ����1ѡ��ʱ��1Ϊ�����ʷ�����
    TMOD &= 0x0F; // ���ö�ʱ��ģʽ
    TL1  = 0x5F;  // ���ö�ʱ��ʼֵ
    TH1  = 0xFE;  // ���ö�ʱ��ʼֵ
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

void main(void)
{
    P_SW2 |= 0x80; // ʹ�ܷ���XFR

    P5M0 = 0x00;
    P5M1 = 0x10;
    P5PU |= 0x10;
    P5NCS &= ~0x10;

    P3M0 = 0x00;
    P3M1 = 0x00;
    P33  = 1;
    P32  = 1;

    Time0_init();
    INT2_init();
    Uart1_Init();

    EA = 1;

    while (1) {
        if (IsRFSuccess) {
            IsRFSuccess         = 0;
            RF433AddressHigh    = (EV1527Value & 0x00FF0000) % 0x0000FFFF;
            RF433AddressLow     = (EV1527Value & 0x0000FF00) % 0x000000FF;
            RF433AddressCommand = EV1527Value & 0x000000FF;
            EV1527Value         = 0;

            RF433Address = (RF433AddressHigh * 256) + RF433AddressLow;

            SBUF = RF433AddressHigh;
            SBUF = RF433AddressLow;
            SBUF = RF433AddressCommand;
            P32  = !P32;
        }
    }
}