#include <stdio.h>
#include <stdint.h>
#include "encode.h"

#define SYMBOL_COUNT 162

unsigned long freq = 14097100UL;  // 发射频率14.0971MHz
char call[7] = "BI1TPH";     // 呼号(最大6字符+终止符)
char loc[5] = "ON80";       // 网格坐标(4字符+终止符)
uint8_t dbm = 10;           // 发射功率(10dBm)
uint8_t tx_buffer[SYMBOL_COUNT];  // 存储编码后的符号

int main(void)
{
    wspr_encode(call, loc, dbm, tx_buffer);
    printf("WSPR 编码结果: ");
    for (int i = 0; i < SYMBOL_COUNT; ++i) {
        printf("%u ", tx_buffer[i]);
    }
    printf("\n");
    return 0;
}
