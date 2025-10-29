// vim: set ai et ts=4 sw=4:
#ifndef _SI5351_H_
#define _SI5351_H_

// PLL选择枚举
typedef enum {
    SI5351_PLL_A = 0,
    SI5351_PLL_B,
} si5351PLL_t;

// R分频器枚举
typedef enum {
    SI5351_R_DIV_1   = 0,   // 1分频
    SI5351_R_DIV_2   = 1,   // 2分频
    SI5351_R_DIV_4   = 2,   // 4分频
    SI5351_R_DIV_8   = 3,   // 8分频
    SI5351_R_DIV_16  = 4,   // 16分频
    SI5351_R_DIV_32  = 5,   // 32分频
    SI5351_R_DIV_64  = 6,   // 64分频
    SI5351_R_DIV_128 = 7,   // 128分频
} si5351RDiv_t;

// 驱动强度枚举
typedef enum {
    SI5351_DRIVE_STRENGTH_2MA = 0x00, // 约 2.2 dBm
    SI5351_DRIVE_STRENGTH_4MA = 0x01, // 约 7.5 dBm
    SI5351_DRIVE_STRENGTH_6MA = 0x02, // 约 9.5 dBm
    SI5351_DRIVE_STRENGTH_8MA = 0x03, // 约 10.7 dBm
} si5351DriveStrength_t;

// PLL配置结构体
typedef struct {
    int32_t mult;   // 倍频系数
    int32_t num;    // 分子
    int32_t denom;  // 分母
} si5351PLLConfig_t;

// 输出配置结构体
typedef struct {
    uint8_t allowIntegerMode; // 是否允许整数模式
    int32_t div;              // 分频系数
    int32_t num;              // 分子
    int32_t denom;            // 分母
    si5351RDiv_t rdiv;        // R分频器
} si5351OutputConfig_t;

/*
 * 基本接口：仅支持使用CLK0和CLK2。
 * 此接口为CLK0和CLK2分别使用独立的PLL，因此两者频率可独立设置。
 * 若需同时使用CLK1，则需让两个CLKx共享一个PLL，配置会更复杂。
 * 选择CLK0和CLK2是因为它们在常见Si5351模块上物理距离较远，
 * 使用时更方便。
 */
void si5351_Init(int32_t correction);
void si5351_SetupCLK0(int32_t Fclk, si5351DriveStrength_t driveStrength);
void si5351_SetupCLK2(int32_t Fclk, si5351DriveStrength_t driveStrength);
void si5351_EnableOutputs(uint8_t enabled);

/*
 * 高级接口。适用于以下需求：
 *
 * a. 同时使用CLK0、CLK1和CLK2；
 * b. 需要两个通道间90°相移；
 *
 * si5351_Calc()在81 MHz以下频率总是使用900 MHz的PLL。
 * 该PLL可安全地被所有<=81 MHz的CLKx共享。
 * 你也可以修改si5351.c，让一个PLL支持<=112.5 MHz的所有频率，
 * 但最坏情况下计算误差会增大到13 Hz。
 */
void si5351_Calc(int32_t Fclk, si5351PLLConfig_t* pll_conf, si5351OutputConfig_t* out_conf);

/*
 * si5351_CalcIQ() 用于寻找能在两个通道间产生90°相移的PLL和MS参数，
 * 若分别为这两个通道传入0和(uint8_t)out_conf.div作为phaseOffset即可。
 * 两通道需使用同一PLL。
 */
void si5351_CalcIQ(int32_t Fclk, si5351PLLConfig_t* pll_conf, si5351OutputConfig_t* out_conf);

void si5351_SetupPLL(si5351PLL_t pll, si5351PLLConfig_t* conf);
int si5351_SetupOutput(uint8_t output, si5351PLL_t pllSource, si5351DriveStrength_t driveStength, si5351OutputConfig_t* conf, uint8_t phaseOffset);

#endif
