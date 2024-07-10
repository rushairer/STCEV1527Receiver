#include "STC8G.h"
#include "config.h"
#include "uart.h"

// #define CPUIDBASE 0xfde0
//  #define ID_ADDR   ((unsigned char volatile __xdata *)(CPUIDBASE + 0x00))

void Time0_init(void) // 100微秒@16MHz
{
    AUXR |= 0x80; // 定时器时钟1T模式
    TMOD &= 0xF0; // 设置定时器模式
    TL0 = 0xA0;   // 设置定时初始值
    TH0 = 0xF6;   // 设置定时初始值
    TF0 = 0;      // 清除TF0标志
    TR0 = 1;      // 定时器0开始计时
    ET0 = 1;      // 使能定时器0中断
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

    // 发射端400us一个信号时间, 接收端时钟100us
    if (((LData >= 145) && (LData <= 160)) && (HData >= 4) && (HData <= 7)) {
        // 31:1 判断是否同步码
        SyncCodeFlag = 1;
    } else if ((SyncCodeFlag == 1) && ((HData >= 14) && (HData <= 17))) {
        // 比例 3:1
        EV1527Code = (EV1527Code << 1) + 1;
        RFCount++;
    } else if ((SyncCodeFlag == 1) && ((HData >= 4) && (HData <= 7))) {
        // 估算比例 1:3
        EV1527Code = EV1527Code << 1;
        RFCount++;
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
            RF433AddressHigh    = (EV1527Value & 0xFF0000) % 0x00FFFF;
            RF433AddressLow     = (EV1527Value & 0x00FF00) % 0x0000FF;
            RF433AddressCommand = EV1527Value & 0x0000FF;
            EV1527Value         = 0;
            RF433Address        = (RF433AddressHigh * 256) + RF433AddressLow;

            if (RF433AddressCommand == ACTION_1) {
                P33 = 1;
                P32 = 0;
            } else if (RF433AddressCommand == ACTION_2) {
                P33 = 0;
                P32 = 1;
            } else if (RF433AddressCommand == ACTION_3) {
                P33 = 1;
                P32 = 1;
            }
        }
    }
}