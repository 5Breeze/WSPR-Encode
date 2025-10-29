// vim: set ai et ts=4 sw=4:

// 针对你的MCU进行更改
#include "stm32f1xx_hal.h"

#include <si5351.h>
#define SI5351_ADDRESS 0x60
#define I2C_HANDLE hi2c1
extern I2C_HandleTypeDef I2C_HANDLE;

// 私有函数声明。
void si5351_writeBulk(uint8_t baseaddr, int32_t P1, int32_t P2, int32_t P3, uint8_t divBy4, si5351RDiv_t rdiv);
void si5351_write(uint8_t reg, uint8_t value);

// 详见 http://www.silabs.com/Support%20Documents/TechnicalDocs/AN619.pdf
enum {
    SI5351_REGISTER_0_DEVICE_STATUS                       = 0,
    SI5351_REGISTER_1_INTERRUPT_STATUS_STICKY             = 1,
    SI5351_REGISTER_2_INTERRUPT_STATUS_MASK               = 2,
    SI5351_REGISTER_3_OUTPUT_ENABLE_CONTROL               = 3,
    SI5351_REGISTER_9_OEB_PIN_ENABLE_CONTROL              = 9,
    SI5351_REGISTER_15_PLL_INPUT_SOURCE                   = 15,
    SI5351_REGISTER_16_CLK0_CONTROL                       = 16,
    SI5351_REGISTER_17_CLK1_CONTROL                       = 17,
    SI5351_REGISTER_18_CLK2_CONTROL                       = 18,
    SI5351_REGISTER_19_CLK3_CONTROL                       = 19,
    SI5351_REGISTER_20_CLK4_CONTROL                       = 20,
    SI5351_REGISTER_21_CLK5_CONTROL                       = 21,
    SI5351_REGISTER_22_CLK6_CONTROL                       = 22,
    SI5351_REGISTER_23_CLK7_CONTROL                       = 23,
    SI5351_REGISTER_24_CLK3_0_DISABLE_STATE               = 24,
    SI5351_REGISTER_25_CLK7_4_DISABLE_STATE               = 25,
    SI5351_REGISTER_42_MULTISYNTH0_PARAMETERS_1           = 42,
    SI5351_REGISTER_43_MULTISYNTH0_PARAMETERS_2           = 43,
    SI5351_REGISTER_44_MULTISYNTH0_PARAMETERS_3           = 44,
    SI5351_REGISTER_45_MULTISYNTH0_PARAMETERS_4           = 45,
    SI5351_REGISTER_46_MULTISYNTH0_PARAMETERS_5           = 46,
    SI5351_REGISTER_47_MULTISYNTH0_PARAMETERS_6           = 47,
    SI5351_REGISTER_48_MULTISYNTH0_PARAMETERS_7           = 48,
    SI5351_REGISTER_49_MULTISYNTH0_PARAMETERS_8           = 49,
    SI5351_REGISTER_50_MULTISYNTH1_PARAMETERS_1           = 50,
    SI5351_REGISTER_51_MULTISYNTH1_PARAMETERS_2           = 51,
    SI5351_REGISTER_52_MULTISYNTH1_PARAMETERS_3           = 52,
    SI5351_REGISTER_53_MULTISYNTH1_PARAMETERS_4           = 53,
    SI5351_REGISTER_54_MULTISYNTH1_PARAMETERS_5           = 54,
    SI5351_REGISTER_55_MULTISYNTH1_PARAMETERS_6           = 55,
    SI5351_REGISTER_56_MULTISYNTH1_PARAMETERS_7           = 56,
    SI5351_REGISTER_57_MULTISYNTH1_PARAMETERS_8           = 57,
    SI5351_REGISTER_58_MULTISYNTH2_PARAMETERS_1           = 58,
    SI5351_REGISTER_59_MULTISYNTH2_PARAMETERS_2           = 59,
    SI5351_REGISTER_60_MULTISYNTH2_PARAMETERS_3           = 60,
    SI5351_REGISTER_61_MULTISYNTH2_PARAMETERS_4           = 61,
    SI5351_REGISTER_62_MULTISYNTH2_PARAMETERS_5           = 62,
    SI5351_REGISTER_63_MULTISYNTH2_PARAMETERS_6           = 63,
    SI5351_REGISTER_64_MULTISYNTH2_PARAMETERS_7           = 64,
    SI5351_REGISTER_65_MULTISYNTH2_PARAMETERS_8           = 65,
    SI5351_REGISTER_66_MULTISYNTH3_PARAMETERS_1           = 66,
    SI5351_REGISTER_67_MULTISYNTH3_PARAMETERS_2           = 67,
    SI5351_REGISTER_68_MULTISYNTH3_PARAMETERS_3           = 68,
    SI5351_REGISTER_69_MULTISYNTH3_PARAMETERS_4           = 69,
    SI5351_REGISTER_70_MULTISYNTH3_PARAMETERS_5           = 70,
    SI5351_REGISTER_71_MULTISYNTH3_PARAMETERS_6           = 71,
    SI5351_REGISTER_72_MULTISYNTH3_PARAMETERS_7           = 72,
    SI5351_REGISTER_73_MULTISYNTH3_PARAMETERS_8           = 73,
    SI5351_REGISTER_74_MULTISYNTH4_PARAMETERS_1           = 74,
    SI5351_REGISTER_75_MULTISYNTH4_PARAMETERS_2           = 75,
    SI5351_REGISTER_76_MULTISYNTH4_PARAMETERS_3           = 76,
    SI5351_REGISTER_77_MULTISYNTH4_PARAMETERS_4           = 77,
    SI5351_REGISTER_78_MULTISYNTH4_PARAMETERS_5           = 78,
    SI5351_REGISTER_79_MULTISYNTH4_PARAMETERS_6           = 79,
    SI5351_REGISTER_80_MULTISYNTH4_PARAMETERS_7           = 80,
    SI5351_REGISTER_81_MULTISYNTH4_PARAMETERS_8           = 81,
    SI5351_REGISTER_82_MULTISYNTH5_PARAMETERS_1           = 82,
    SI5351_REGISTER_83_MULTISYNTH5_PARAMETERS_2           = 83,
    SI5351_REGISTER_84_MULTISYNTH5_PARAMETERS_3           = 84,
    SI5351_REGISTER_85_MULTISYNTH5_PARAMETERS_4           = 85,
    SI5351_REGISTER_86_MULTISYNTH5_PARAMETERS_5           = 86,
    SI5351_REGISTER_87_MULTISYNTH5_PARAMETERS_6           = 87,
    SI5351_REGISTER_88_MULTISYNTH5_PARAMETERS_7           = 88,
    SI5351_REGISTER_89_MULTISYNTH5_PARAMETERS_8           = 89,
    SI5351_REGISTER_90_MULTISYNTH6_PARAMETERS             = 90,
    SI5351_REGISTER_91_MULTISYNTH7_PARAMETERS             = 91,
    SI5351_REGISTER_92_CLOCK_6_7_OUTPUT_DIVIDER           = 92,
    SI5351_REGISTER_165_CLK0_INITIAL_PHASE_OFFSET         = 165,
    SI5351_REGISTER_166_CLK1_INITIAL_PHASE_OFFSET         = 166,
    SI5351_REGISTER_167_CLK2_INITIAL_PHASE_OFFSET         = 167,
    SI5351_REGISTER_168_CLK3_INITIAL_PHASE_OFFSET         = 168,
    SI5351_REGISTER_169_CLK4_INITIAL_PHASE_OFFSET         = 169,
    SI5351_REGISTER_170_CLK5_INITIAL_PHASE_OFFSET         = 170,
    SI5351_REGISTER_177_PLL_RESET                         = 177,
    SI5351_REGISTER_183_CRYSTAL_INTERNAL_LOAD_CAPACITANCE = 183
};

typedef enum {
    SI5351_CRYSTAL_LOAD_6PF  = (1<<6),
    SI5351_CRYSTAL_LOAD_8PF  = (2<<6),
    SI5351_CRYSTAL_LOAD_10PF = (3<<6)
} si5351CrystalLoad_t;

int32_t si5351Correction;

/*
 * 初始化Si5351。请在进行其他操作前调用此函数。
 * `correction`为实际频率与期望频率在100 MHz时的差值。
 * 可以在较低频率下测量并线性缩放。
 * 例如：如果你得到10_000_097 Hz而不是10_000_000 Hz，`correction`为97*10=970
 */
void si5351_Init(int32_t correction) {
    si5351Correction = correction;

    // 通过将CLKx_DIS位置1，禁用所有输出
    si5351_write(SI5351_REGISTER_3_OUTPUT_ENABLE_CONTROL, 0xFF);

    // 关闭所有输出驱动
    si5351_write(SI5351_REGISTER_16_CLK0_CONTROL, 0x80);
    si5351_write(SI5351_REGISTER_17_CLK1_CONTROL, 0x80);
    si5351_write(SI5351_REGISTER_18_CLK2_CONTROL, 0x80);
    si5351_write(SI5351_REGISTER_19_CLK3_CONTROL, 0x80);
    si5351_write(SI5351_REGISTER_20_CLK4_CONTROL, 0x80);
    si5351_write(SI5351_REGISTER_21_CLK5_CONTROL, 0x80);
    si5351_write(SI5351_REGISTER_22_CLK6_CONTROL, 0x80);
    si5351_write(SI5351_REGISTER_23_CLK7_CONTROL, 0x80);

    // 设置晶振负载电容
    si5351CrystalLoad_t crystalLoad = SI5351_CRYSTAL_LOAD_10PF;
    si5351_write(SI5351_REGISTER_183_CRYSTAL_INTERNAL_LOAD_CAPACITANCE, crystalLoad);
}

// 设置指定PLL的倍频参数
void si5351_SetupPLL(si5351PLL_t pll, si5351PLLConfig_t* conf) {
    int32_t P1, P2, P3;
    int32_t mult = conf->mult;
    int32_t num = conf->num;
    int32_t denom = conf->denom;

    P1 = 128 * mult + (128 * num)/denom - 512;
    // P2 = 128 * num - denom * ((128 * num)/denom);
    P2 = (128 * num) % denom;
    P3 = denom;

    // 获取PLL寄存器的基地址
    uint8_t baseaddr = (pll == SI5351_PLL_A ? 26 : 34);
    si5351_writeBulk(baseaddr, P1, P2, P3, 0, 0);

    // 复位两个PLL
    si5351_write(SI5351_REGISTER_177_PLL_RESET, (1<<7) | (1<<5) );
}

// 配置PLL源、驱动强度、多合成分频器、R分频器和相位偏移。
// 成功返回0，失败返回非0。
int si5351_SetupOutput(uint8_t output, si5351PLL_t pllSource, si5351DriveStrength_t driveStrength, si5351OutputConfig_t* conf, uint8_t phaseOffset) {
    int32_t div = conf->div;
    int32_t num = conf->num;
    int32_t denom = conf->denom;
    uint8_t divBy4 = 0;
    int32_t P1, P2, P3;

    if(output > 2) {
        return 1;
    }

    if((!conf->allowIntegerMode) && ((div < 8) || ((div == 8) && (num == 0)))) {
        // 仅在整数模式下，div可为{4,6,8}
        return 2;
    }

    if(div == 4) {
        // 特殊DIVBY4情况，详见AN619 4.1.3
        P1 = 0;
        P2 = 0;
        P3 = 1;
        divBy4 = 0x3;
    } else {
        P1 = 128 * div + ((128 * num)/denom) - 512;
        // P2 = 128 * num - denom * (128 * num)/denom;
        P2 = (128 * num) % denom;
        P3 = denom;
    }

    // 获取对应通道的寄存器地址
    uint8_t baseaddr = 0;
    uint8_t phaseOffsetRegister = 0;
    uint8_t clkControlRegister = 0;
    switch (output) {
    case 0:
        baseaddr = SI5351_REGISTER_42_MULTISYNTH0_PARAMETERS_1;
        phaseOffsetRegister = SI5351_REGISTER_165_CLK0_INITIAL_PHASE_OFFSET;
        clkControlRegister = SI5351_REGISTER_16_CLK0_CONTROL;
        break;
    case 1:
        baseaddr = SI5351_REGISTER_50_MULTISYNTH1_PARAMETERS_1;
        phaseOffsetRegister = SI5351_REGISTER_166_CLK1_INITIAL_PHASE_OFFSET;
        clkControlRegister = SI5351_REGISTER_17_CLK1_CONTROL;
        break;
    case 2:
        baseaddr = SI5351_REGISTER_58_MULTISYNTH2_PARAMETERS_1;
        phaseOffsetRegister = SI5351_REGISTER_167_CLK2_INITIAL_PHASE_OFFSET;
        clkControlRegister = SI5351_REGISTER_18_CLK2_CONTROL;
        break;
    }

    uint8_t clkControl = 0x0C | driveStrength; // 时钟不反相，输出使能
    if(pllSource == SI5351_PLL_B) {
        clkControl |= (1 << 5); // 使用PLLB
    }

    if((conf->allowIntegerMode) && ((num == 0)||(div == 4))) {
        // 使用整数模式
        clkControl |= (1 << 6);
    }

    si5351_write(clkControlRegister, clkControl);
    si5351_writeBulk(baseaddr, P1, P2, P3, divBy4, conf->rdiv);
    si5351_write(phaseOffsetRegister, (phaseOffset & 0x7F));

    return 0;
}

// 计算给定Fclk（范围8_000~160_000_000）下的PLL、MS和RDiv参数。
// 假设`correction`正确，实际频率与Fclk的误差小于6Hz。
void si5351_Calc(int32_t Fclk, si5351PLLConfig_t* pll_conf, si5351OutputConfig_t* out_conf) {
    if(Fclk < 8000) Fclk = 8000;
    else if(Fclk > 160000000) Fclk = 160000000;

    out_conf->allowIntegerMode = 1;

    if(Fclk < 1000000) {
        // 对于[8_000, 500_000]范围的频率，可以用si5351_Calc(Fclk*64, ...)和SI5351_R_DIV_64。
        // 实际上，低于1MHz的频率都建议这样做，可减小误差。
        Fclk *= 64;
        out_conf->rdiv = SI5351_R_DIV_64;
    } else {
        out_conf->rdiv = SI5351_R_DIV_1;
    }

    // 应用校正，在确定rdiv之后。
    Fclk = Fclk - (int32_t)((((double)Fclk)/100000000.0)*((double)si5351Correction));

    // 这里我们寻找整数a,b,c,x,y,z使得：
    // N = a + b / c    # PLL参数
    // M = x + y / z    # MS参数
    // Fclk = Fxtal * N / M
    // N在[24,36]，M在[8,1800]或M为{4,6}，b<c，y<z，b,c,y,z<=2^20，c,z!=0
    // 对于[500K,160MHz]的Fclk，该算法能找到abs(Ffound-Fclk)<=6Hz的解

    const int32_t Fxtal = 25000000;
    int32_t a, b, c, x, y, z, t;

    if(Fclk < 81000000) {
        // 适用于0.5~112.5MHz，81MHz以上误差大于6Hz
        a = 36; // PLL运行在900MHz
        b = 0;
        c = 1;
        int32_t Fpll = 900000000;
        x = Fpll/Fclk;
        t = (Fclk >> 20) + 1;
        y = (Fpll % Fclk) / t;
        z = Fclk / t;
    } else {
        // 适用于75~160MHz
        if(Fclk >= 150000000) {
            x = 4;
        } else if (Fclk >= 100000000) {
            x = 6;
        } else {
            x = 8;
        }
        y = 0;
        z = 1;
        
        int32_t numerator = x*Fclk;
        a = numerator/Fxtal;
        t = (Fxtal >> 20) + 1;
        b = (numerator % Fxtal) / t;
        c = Fxtal / t;
    }

    pll_conf->mult = a;
    pll_conf->num = b;
    pll_conf->denom = c;
    out_conf->div = x;
    out_conf->num = y;
    out_conf->denom = z;
}

// si5351_CalcIQ()用于寻找能在两个通道间产生90°相移的PLL和MS参数，
// 若分别为这两个通道传入0和(uint8_t)out_conf.div作为phaseOffset即可。
// 两通道需使用同一PLL。Fclk范围1.4MHz~100MHz，假设`correction`正确，实际频率误差小于4Hz。
void si5351_CalcIQ(int32_t Fclk, si5351PLLConfig_t* pll_conf, si5351OutputConfig_t* out_conf) {
    const int32_t Fxtal = 25000000;
    int32_t Fpll;

    if(Fclk < 1400000) Fclk = 1400000;
    else if(Fclk > 100000000) Fclk = 100000000;

    // 应用校正
    Fclk = Fclk - ((Fclk/1000000)*si5351Correction)/100;

    // 禁用整数模式
    out_conf->allowIntegerMode = 0;

    // 使用R分频器会改变相移，AN619未说明此时的行为
    out_conf->rdiv = 0;

    if(Fclk < 4900000) {
        // 小技巧：让PLL低于600MHz以覆盖1.4~4.725MHz。
        // AN619未明说PLL不能低于600MHz，实测低于177MHz不稳定，
        // 这限制了Fclk最低为177/127=1.4MHz。
        out_conf->div = 127;
    } else if(Fclk < 8000000) {
        out_conf->div = 625000000 / Fclk;
    } else {
        out_conf->div = 900000000 / Fclk;
    }
    out_conf->num = 0;
    out_conf->denom = 1;

    Fpll = Fclk * out_conf->div;
    pll_conf->mult = Fpll / Fxtal;
    pll_conf->num = (Fpll % Fxtal) / 24;
    pll_conf->denom = Fxtal / 24; // denom不能超过0xFFFFF
}

// 设置CLK0为指定频率和驱动强度，使用PLLA。
void si5351_SetupCLK0(int32_t Fclk, si5351DriveStrength_t driveStrength) {
	si5351PLLConfig_t pll_conf;
	si5351OutputConfig_t out_conf;

	si5351_Calc(Fclk, &pll_conf, &out_conf);
	si5351_SetupPLL(SI5351_PLL_A, &pll_conf);
	si5351_SetupOutput(0, SI5351_PLL_A, driveStrength, &out_conf, 0);
}

// 设置CLK2为指定频率和驱动强度，使用PLLB。
void si5351_SetupCLK2(int32_t Fclk, si5351DriveStrength_t driveStrength) {
	si5351PLLConfig_t pll_conf;
	si5351OutputConfig_t out_conf;

	si5351_Calc(Fclk, &pll_conf, &out_conf);
	si5351_SetupPLL(SI5351_PLL_B, &pll_conf);
	si5351_SetupOutput(2, SI5351_PLL_B, driveStrength, &out_conf, 0);
}

// 根据提供的位掩码使能或禁用输出。
// 示例：
// si5351_EnableOutputs(1 << 0) 只使能CLK0，禁用CLK1和CLK2
// si5351_EnableOutputs((1 << 2) | (1 << 0)) 使能CLK0和CLK2，禁用CLK1
void si5351_EnableOutputs(uint8_t enabled) {
    si5351_write(SI5351_REGISTER_3_OUTPUT_ENABLE_CONTROL, ~enabled);
}

// 通过I2C写入8位寄存器值。
void si5351_write(uint8_t reg, uint8_t value) {
    while (HAL_I2C_IsDeviceReady(&I2C_HANDLE, (uint16_t)(SI5351_ADDRESS<<1), 3, HAL_MAX_DELAY) != HAL_OK) { }

    HAL_I2C_Mem_Write(&I2C_HANDLE,                  // I2C句柄
                      (uint8_t)(SI5351_ADDRESS<<1), // I2C地址，左对齐
                      (uint8_t)reg,                 // 寄存器地址
                      I2C_MEMADD_SIZE_8BIT,         // si5351使用8位寄存器地址
                      (uint8_t*)(&value),           // 要写入的数据
                      1,                            // 写入字节数
                      HAL_MAX_DELAY);               // 超时时间
}

// _SetupPLL和_SetupOutput的通用写寄存器代码
void si5351_writeBulk(uint8_t baseaddr, int32_t P1, int32_t P2, int32_t P3, uint8_t divBy4, si5351RDiv_t rdiv) {
    si5351_write(baseaddr,   (P3 >> 8) & 0xFF);
    si5351_write(baseaddr+1, P3 & 0xFF);
    si5351_write(baseaddr+2, ((P1 >> 16) & 0x3) | ((divBy4 & 0x3) << 2) | ((rdiv & 0x7) << 4));
    si5351_write(baseaddr+3, (P1 >> 8) & 0xFF);
    si5351_write(baseaddr+4, P1 & 0xFF);
    si5351_write(baseaddr+5, ((P3 >> 12) & 0xF0) | ((P2 >> 16) & 0xF));
    si5351_write(baseaddr+6, (P2 >> 8) & 0xFF);
    si5351_write(baseaddr+7, P2 & 0xFF);
}
