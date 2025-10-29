
void encode()
{
    uint8_t i;
    
    // 1. 编码WSPR消息
    wspr_encode(call, loc, dbm, tx_buffer);

    // 2. 启用时钟输出和LED
    set_clock_pwr(SI5351_CLK0, 1);

    // 3. 发送每个符号
    for (i = 0; i < WSPR_SYMBOL_COUNT; i++)
    {
        // 计算当前符号频率
        uint64_t frequency = (freq * 100) + (tx_buffer[i] * TONE_SPACING);
        set_freq(frequency, SI5351_CLK0);

        // 等待定时器中断
        proceed = false;
        while (!proceed);
    }

    
    // 4. 关闭输出
    set_clock_pwr(SI5351_CLK0, 0);
}


unsigned long freq = 14097100UL;  // 发射频率14.0971MHz
char call[7] = "BI1TPH";     // 呼号(最大6字符+终止符)
char loc[5] = "ON80";       // 网格坐标(4字符+终止符)
uint8_t dbm = 10;           // 发射功率(10dBm)
uint8_t tx_buffer[SYMBOL_COUNT];  // 存储编码后的符号

main()
{
    init(SI5351_CRYSTAL_LOAD_8PF, 0, CORRECTION);
    set_freq(freq * 100, SI5351_CLK0);
    drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
    set_clock_pwr(SI5351_CLK0, 0);

	while (condition)  
	{
		encode();
	}	
	
}