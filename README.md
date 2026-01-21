# WSPR-Encode
A multi-component development library for WSPR (Weak Signal Propagation Reporter) signal encoding, RF circuit simulation and hardware control / 面向WSPR（弱信号传播报告）系统的多组件开发库，涵盖信号编码、射频电路仿真与硬件控制功能

## Table of Contents / 目录
- [Project Overview / 项目概述](#project-overview--项目概述)
- [Core Files & Functions / 核心文件与功能](#core-files--functions--核心文件与功能)
- [Key Features / 关键特性](#key-features--关键特性)
- [Usage Guide / 使用指南](#usage-guide--使用指南)
- [Dependencies / 依赖说明](#dependencies--依赖说明)

## Project Overview / 项目概述
WSPR-Encode is a complete development library for WSPR signal processing, integrating **signal encoding logic**, **RF filter simulation**, and **hardware driver control** (Si5351 clock generator). It is designed for amateur radio enthusiasts and developers to quickly build WSPR transmitters, with MATLAB-based RF circuit verification and C-language-based embedded encoding/control implementation.
/ WSPR-Encode 是一套完整的WSPR信号处理开发库，整合了**信号编码逻辑**、**射频滤波器仿真**、**硬件驱动控制**（Si5351时钟发生器）三大核心能力。面向业余无线电爱好者与开发者，可快速搭建WSPR发射机，同时提供基于MATLAB的射频电路验证能力和基于C语言的嵌入式编码/控制实现。

## Core Files & Functions / 核心文件与功能
| File / 文件       | Language / 语言 | Core Function / 核心功能                                                                 |
|-------------------|-----------------|-----------------------------------------------------------------------------------------|
| `RF.m`            | MATLAB          | LC low-pass filter simulation for 7MHz/10MHz WSPR bands; visualizes S21/S11 RF parameters / 7MHz/10MHz WSPR频段LC低通滤波器仿真，可视化S21/S11射频参数 |
| `encode.c`/`encode.h` | C            | Core WSPR signal encoding logic; converts input data (callsign/location/power) to WSPR modulation symbols / 核心WSPR信号编码逻辑，将呼号/位置/功率等输入数据转换为WSPR调制符号 |
| `si5351.c`/`si5351.h` | C            | Driver for Si5351 clock generator; controls frequency synthesis for WSPR signal transmission / Si5351时钟发生器驱动，控制WSPR信号发射的频率合成 |
| `main.c`/`app.c`  | C               | Integration layer; calls encoding and Si5351 driver to implement end-to-end WSPR signal output / 集成层，调用编码模块与Si5351驱动实现端到端WSPR信号输出 |
| `nhash.c`/`nhash.h` | C             | Hash algorithm implementation for WSPR data verification / 哈希算法实现，用于WSPR数据校验 |
| `Filter1.ftr`     | -               | RF filter parameter configuration file (matches `RF.m` simulation) / 射频滤波器参数配置文件（匹配`RF.m`仿真参数） |
| `main.exe`        | -               | Compiled executable for Windows (test/quick use of encoding & control logic) / Windows编译可执行文件（快速测试编码与控制逻辑） |

## Key Features / 关键特性
### 1. Multi-band RF Simulation / 多频段射频仿真
- Supports 7MHz/10MHz (and test version 10MHz) WSPR bands; parameterized LC filter design via MATLAB RF Toolbox / 支持7MHz/10MHz（及10MHz测试版）WSPR频段，通过MATLAB RF Toolbox实现参数化LC滤波器设计；
- Visualizes S21 (transmission coefficient) and S11 (input reflection coefficient) to verify filter passband/impedance matching / 可视化S21（传输系数）与S11（输入反射系数），验证滤波器通带特性与阻抗匹配效果。

### 2. Lightweight C-language Encoding / 轻量级C语言编码
- Pure C implementation of WSPR encoding protocol, no complex external dependencies; easy to port to embedded platforms (e.g., Arduino/STM32) / 纯C实现WSPR编码协议，无复杂外部依赖，易移植至嵌入式平台（如Arduino/STM32）；
- Integrates hash verification to ensure encoding accuracy of WSPR core data (callsign, grid square, transmit power) / 集成哈希校验，保证WSPR核心数据（呼号、网格定位、发射功率）编码准确性。

### 3. Hardware Control Integration / 硬件控制集成
- Complete Si5351 driver to generate stable WSPR carrier frequency; supports frequency adjustment for different bands / 完整Si5351驱动，生成稳定的WSPR载波频率，支持不同频段的频率调整；
- Modular design: encoding, RF simulation, hardware control are decoupled, easy to replace/extend components (e.g., switch to other RF filters) / 模块化设计：编码、射频仿真、硬件控制解耦，易替换/扩展组件（如切换其他射频滤波器）。

## Usage Guide / 使用指南
### 1. RF Filter Simulation (MATLAB) / 射频滤波器仿真（MATLAB）
```matlab
% 1. Open RF.m in MATLAB
% 1. 在MATLAB中打开RF.m
% 2. Uncomment the target frequency parameter (7MHz/10MHz/10MHz test)
% 2. 解注释目标频段参数（7MHz/10MHz/10MHz测试版）
% 3. Run the script to generate S21/S11 curve plots
% 3. 运行脚本，生成S21/S11曲线图表
RF
```

### 2. Embedded Encoding & Control (C Language) / 嵌入式编码与控制（C语言）
```c
// Example: Call encoding and Si5351 control in main.c
// 示例：在main.c中调用编码与Si5351控制
#include "encode.h"
#include "si5351.h"

int main() {
    // 1. Initialize Si5351 (set WSPR carrier frequency: 7MHz)
    // 1. 初始化Si5351（设置WSPR载波频率：7MHz）
    si5351_init(7000000);
    
    // 2. Encode WSPR data (callsign: MYCALL, grid: BL12, power: 10W)
    // 2. 编码WSPR数据（呼号：MYCALL，网格定位：BL12，功率：10W）
    uint8_t wspr_symbols[162]; // WSPR standard symbol length
    wspr_encode("MYCALL", "BL12", 10, wspr_symbols);
    
    // 3. Output encoded symbols to RF via Si5351
    // 3. 将编码符号通过Si5351输出至射频端
    wspr_transmit(wspr_symbols);
    
    return 0;
}
```

## Dependencies / 依赖说明
### For MATLAB (RF.m) / MATLAB环境（RF.m）
- MATLAB R2018b+ with RF Toolbox / 安装RF Toolbox工具箱的MATLAB R2018b及以上版本；
- No additional third-party libraries required / 无需额外第三方库。

### For C Language Code / C语言代码
- Embedded C compiler (e.g., GCC/ARM GCC) / 嵌入式C编译器（如GCC/ARM GCC）；
- No OS dependency (portable to bare-metal embedded systems) / 无操作系统依赖（可移植至裸机嵌入式系统）；
- Optional: Si5351 hardware module (for actual RF transmission) / 可选：Si5351硬件模块（用于实际射频发射）。

## License / 许可证
This project is for amateur radio non-commercial use only. Redistribution and modification are permitted with attribution to the original author.
/ 本项目仅限业余无线电非商业用途，保留原作者署名的前提下允许重分发与修改。
