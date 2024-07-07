#include "STC8G.h"
#include "config.h"

__bit busy;

#define CPUIDBASE 0xfde0
#define ID_ADDR   ((unsigned char volatile __xdata *)(CPUIDBASE + 0x00))

void UartSend(char dat);

void Time0_init(void) // 100微秒@16MHz
{
    AUXR |= 0x80; // 定时器时钟1T模式
    TMOD &= 0xF0; // 设置定时器模式
    TL0 = 0xC0;   // 设置定时初始值
    TH0 = 0xF9;   // 设置定时初始值
    TF0 = 0;      // 清除TF0标志
    TR0 = 1;      // 定时器0开始计时
    ET0 = 1;      // 使能定时器0中断
}

uint8_t RFPinValue(void)
{
    return RF_PIN == 1;
}

// 优先级大于下降沿触发
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
    INTCLKO = 0x10; // 使能INT2中断
}

// 下降沿触发
void INT2_Isr(void) __interrupt(10)
{
    HData = HTime;
    LData = LTime;

    HTime = 0;
    LTime = 0;

    // 124:4
    if (((LData >= 80) && (LData <= 150)) && (HData >= 2) && (HData <= 6)) {
        // 估算比例 判断是否同步码
        SyncCodeFlag = 1;
    } else if ((SyncCodeFlag == 1) && ((HData >= 8) && (HData <= 14))) {
        // 估算比例 1
        EV1527Code = EV1527Code << 1 + 1;
        RFCount++;
    } else if ((SyncCodeFlag == 1) && ((HData >= 2) && (HData <= 6))) {
        // 估算比例 0
        EV1527Code = EV1527Code << 1;
        RFCount++;
    }

    IE0 = 0;
}

void Uart1_Init(void) // 9600bps@16MHz
{
    SCON = 0x50;  // 8位数据,可变波特率
    AUXR |= 0x40; // 定时器时钟1T模式
    AUXR &= 0xFE; // 串口1选择定时器1为波特率发生器
    TMOD &= 0x0F; // 设置定时器模式
    TL1  = 0x5F;  // 设置定时初始值
    TH1  = 0xFE;  // 设置定时初始值
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

void main(void)
{
    P_SW2 |= 0x80; // 使能访问XFR

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