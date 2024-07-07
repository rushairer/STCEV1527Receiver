#ifndef __CONFIG_H_
#define __CONFIG_H_
#include <stdint.h>

#define RF_PIN  P54
#define BTN_PIN P55

uint8_t HTime        = 0; // �ߵ�ƽʱ��
uint8_t LTime        = 0; // �͵�ƽʱ��
uint8_t HData        = 0; // �������ݽ���
uint8_t LData        = 0; // �������ݽ���
uint8_t RFCount      = 0; // ����λ��
uint8_t SyncCodeFlag = 0; // ͬ�����־
uint8_t IsRFSuccess  = 0; // ������ɱ�־

uint32_t EV1527Code;
uint32_t EV1527Value, EV1527Value1, EV1527Value2;
uint32_t TempValue;
__bit Synced = 1;

uint8_t RF433AddressHigh    = 0;
uint8_t RF433AddressLow     = 0;
uint8_t RF433AddressCommand = 0;

uint16_t RF433Address = 65535; // �������

#endif // !__CONFIG_H_