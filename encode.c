#define WSPR_BIT_COUNT 162
#define WSPR_SYMBOL_COUNT 162
#define WSPR_MESSAGE_BYTE_SIZE 11

#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "nhash.h"

static char callsign[12];
static char locator[7];
static int8_t power;

/**
 * @brief 合并同步向量到符号数组中。
 *
 * @param g 输入的比特数组，长度为 WSPR_SYMBOL_COUNT。
 * @param symbols 输出的符号数组，长度为 WSPR_SYMBOL_COUNT。
 *
 * 该函数将同步向量与输入比特数组 g 合并，生成最终的符号数组。
 */
void wspr_merge_sync_vector(uint8_t *g, uint8_t *symbols)
{
	uint8_t i;
	// WSPR 协议的同步向量，长度为 162
	const uint8_t sync_vector[WSPR_SYMBOL_COUNT] =
		{1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0,
		 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0,
		 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1,
		 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0,
		 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1,
		 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1,
		 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
		 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0};

	for (i = 0; i < WSPR_SYMBOL_COUNT; i++)
	{
		symbols[i] = sync_vector[i] + (2 * g[i]); // 合并同步向量和比特流
	}
}

/**
 * @brief 对输入比特流进行交织处理。
 *
 * @param s 输入输出数组，长度为 WSPR_BIT_COUNT。
 *
 * 该函数对输入的比特流进行比特反转索引的交织操作，提高抗干扰能力。
 */
void wspr_interleave(uint8_t *s)
{
	uint8_t d[WSPR_BIT_COUNT];
	uint8_t rev, index_temp, i, j, k;

	i = 0;

	for (j = 0; j < 255; j++)
	{
		// 反转索引的比特顺序
		index_temp = j;
		rev = 0;

		for (k = 0; k < 8; k++)
		{
			if (index_temp & 0x01)
			{
				rev = rev | (1 << (7 - k));
			}
			index_temp = index_temp >> 1;
		}

		if (rev < WSPR_BIT_COUNT)
		{
			d[rev] = s[i];
			i++;
		}

		if (i >= WSPR_BIT_COUNT)
		{
			break;
		}
	}

	memcpy(s, d, WSPR_BIT_COUNT); // 拷贝交织后的数据回原数组
}

/**
 * @brief 卷积编码器，将输入比特流编码为冗余比特流。
 *
 * @param c 输入数据数组。
 * @param s 输出编码后的比特流。
 * @param message_size 输入数据长度（字节）。
 * @param bit_size 输出比特流长度（比特）。
 *
 * 该函数实现了 WSPR 协议的卷积编码，增加纠错能力。
 */
void convolve(uint8_t *c, uint8_t *s, uint8_t message_size, uint8_t bit_size)
{
	uint32_t reg_0 = 0;
	uint32_t reg_1 = 0;
	uint32_t reg_temp = 0;
	uint8_t input_bit, parity_bit;
	uint8_t bit_count = 0;
	uint8_t i, j, k;

	for (i = 0; i < message_size; i++)
	{
		for (j = 0; j < 8; j++)
		{
			// 取当前字节的最高位作为输入比特
			input_bit = (((c[i] << j) & 0x80) == 0x80) ? 1 : 0;

			// 两个寄存器左移并写入新比特
			reg_0 = reg_0 << 1;
			reg_1 = reg_1 << 1;
			reg_0 |= (uint32_t)input_bit;
			reg_1 |= (uint32_t)input_bit;

			// 与反馈多项式做与运算，计算奇偶校验位
			reg_temp = reg_0 & 0xf2d05351;
			parity_bit = 0;
			for (k = 0; k < 32; k++)
			{
				parity_bit = parity_bit ^ (reg_temp & 0x01);
				reg_temp = reg_temp >> 1;
			}
			s[bit_count] = parity_bit;
			bit_count++;

			// 第二组反馈多项式
			reg_temp = reg_1 & 0xe4613c47;
			parity_bit = 0;
			for (k = 0; k < 32; k++)
			{
				parity_bit = parity_bit ^ (reg_temp & 0x01);
				reg_temp = reg_temp >> 1;
			}
			s[bit_count] = parity_bit;
			bit_count++;
			if (bit_count >= bit_size)
			{
				break;
			}
		}
	}
}

/**
 * @brief 将字符转换为 WSPR 协议规定的编码值。
 *
 * @param c 输入字符。
 * @return uint8_t 对应的编码值。
 *
 * 只允许数字、大写字母和空格，其他字符统一编码为 36。
 */
uint8_t wspr_code(char c)
{
	// 校验输入字符并返回对应编码值，不允许的字符统一转为空格

	if (isdigit(c))
	{
		return (uint8_t)(c - 48); // 数字 0-9
	}
	else if (c == ' ')
	{
		return 36; // 空格
	}
	else if (c >= 'A' && c <= 'Z')
	{
		return (uint8_t)(c - 55); // 大写字母 A-Z
	}
	else
	{
		return 36; // 其他字符
	}
}



void pad_callsign(char * call)
{
	// If only the 2nd character is a digit, then pad with a space.
	// If this happens, then the callsign will be truncated if it is
	// longer than 6 characters.
	if(isdigit(call[1]) && isupper(call[2]))
	{
		// memmove(call + 1, call, 6);
    call[5] = call[4];
    call[4] = call[3];
    call[3] = call[2];
    call[2] = call[1];
    call[1] = call[0];
		call[0] = ' ';
	}

	// Now the 3rd charcter in the callsign must be a digit
	// if(call[2] < '0' || call[2] > '9')
	// {
	// 	// return 1;
	// }
}

/**
 * @brief WSPR 消息比特打包。
 *
 * @param c 输出的比特数组，长度至少为 11 字节。
 *
 * 该函数根据 callsign、locator、power 等全局变量，将消息内容打包为比特流。
 * 支持三种类型的 WSPR 消息（普通、带斜杠、特殊格式）。
 */
void wspr_bit_packing(uint8_t *c)
{
	uint32_t n, m;

	// 判断消息类型（1、2、3）
	char *slash_avail = strchr(callsign, (int)'/');
	if (callsign[0] == '<')
	{
		// 类型 3 消息
		char base_call[13];
		memset(base_call, 0, 13);
		uint32_t init_val = 146;
		char *bracket_avail = strchr(callsign, (int)'>');
		int call_len = bracket_avail - callsign - 1;
		strncpy(base_call, callsign + 1, call_len);
		uint32_t hash = nhash_(base_call, &call_len, &init_val);
		hash &= 32767;

		// 6 字符网格转换为 callsign 格式，首字符移到末尾
		char temp_loc = locator[0];
		locator[0] = locator[1];
		locator[1] = locator[2];
		locator[2] = locator[3];
		locator[3] = locator[4];
		locator[4] = locator[5];
		locator[5] = temp_loc;

		n = wspr_code(locator[0]);
		n = n * 36 + wspr_code(locator[1]);
		n = n * 10 + wspr_code(locator[2]);
		n = n * 27 + (wspr_code(locator[3]) - 10);
		n = n * 27 + (wspr_code(locator[4]) - 10);
		n = n * 27 + (wspr_code(locator[5]) - 10);

		m = (hash * 128) - (power + 1) + 64;
	}
	else if (slash_avail == (void *)0)
	{
		// 类型 1 消息
		char callsign_local[12];
		strncpy(callsign_local, callsign, 12);
		pad_callsign(callsign_local);
		n = wspr_code(callsign_local[0]);
		n = n * 36 + wspr_code(callsign_local[1]);
		n = n * 10 + wspr_code(callsign_local[2]);
		n = n * 27 + (wspr_code(callsign_local[3]) - 10);
		n = n * 27 + (wspr_code(callsign_local[4]) - 10);
		n = n * 27 + (wspr_code(callsign_local[5]) - 10);

		m = ((179 - 10 * (locator[0] - 'A') - (locator[2] - '0')) * 180) +
			(10 * (locator[1] - 'A')) + (locator[3] - '0');
		m = (m * 128) + power + 64;
	}
	else if (slash_avail)
	{
		// 类型 2 消息
		int slash_pos = slash_avail - callsign;
		uint8_t i;

		// 判断是前缀还是后缀
		if (callsign[slash_pos + 2] == ' ' || callsign[slash_pos + 2] == 0)
		{
			// 单字符后缀
			char base_call[7];
			memset(base_call, 0, 7);
			strncpy(base_call, callsign, slash_pos);
			for (i = 0; i < 7; i++)
			{
				base_call[i] = toupper(base_call[i]);
				if (!(isdigit(base_call[i]) || isupper(base_call[i])))
				{
					base_call[i] = ' ';
				}
			}
			pad_callsign(base_call);

			n = wspr_code(base_call[0]);
			n = n * 36 + wspr_code(base_call[1]);
			n = n * 10 + wspr_code(base_call[2]);
			n = n * 27 + (wspr_code(base_call[3]) - 10);
			n = n * 27 + (wspr_code(base_call[4]) - 10);
			n = n * 27 + (wspr_code(base_call[5]) - 10);

			char x = callsign[slash_pos + 1];
			if (x >= 48 && x <= 57)
			{
				x -= 48;
			}
			else if (x >= 65 && x <= 90)
			{
				x -= 55;
			}
			else
			{
				x = 38;
			}

			m = 60000 - 32768 + x;

			m = (m * 128) + power + 2 + 64;
		}
		else if (callsign[slash_pos + 3] == ' ' || callsign[slash_pos + 3] == 0)
		{
			// 两位数字后缀
			char base_call[7];
			memset(base_call, 0, 7);
			strncpy(base_call, callsign, slash_pos);
			for (i = 0; i < 6; i++)
			{
				base_call[i] = toupper(base_call[i]);
				if (!(isdigit(base_call[i]) || isupper(base_call[i])))
				{
					base_call[i] = ' ';
				}
			}
			pad_callsign(base_call);

			n = wspr_code(base_call[0]);
			n = n * 36 + wspr_code(base_call[1]);
			n = n * 10 + wspr_code(base_call[2]);
			n = n * 27 + (wspr_code(base_call[3]) - 10);
			n = n * 27 + (wspr_code(base_call[4]) - 10);
			n = n * 27 + (wspr_code(base_call[5]) - 10);

			// TODO: 需要校验数字
			m = 10 * (callsign[slash_pos + 1] - 48) + callsign[slash_pos + 2] - 48;
			m = 60000 + 26 + m;
			m = (m * 128) + power + 2 + 64;
		}
		else
		{
			// 前缀
			char prefix[4];
			char base_call[7];
			memset(prefix, 0, 4);
			memset(base_call, 0, 7);
			strncpy(prefix, callsign, slash_pos);
			strncpy(base_call, callsign + slash_pos + 1, 7);

			if (prefix[2] == ' ' || prefix[2] == 0)
			{
				// 右对齐前缀
				prefix[3] = 0;
				prefix[2] = prefix[1];
				prefix[1] = prefix[0];
				prefix[0] = ' ';
			}

			for (uint8_t i = 0; i < 6; i++)
			{
				base_call[i] = toupper(base_call[i]);
				if (!(isdigit(base_call[i]) || isupper(base_call[i])))
				{
					base_call[i] = ' ';
				}
			}
			pad_callsign(base_call);

			n = wspr_code(base_call[0]);
			n = n * 36 + wspr_code(base_call[1]);
			n = n * 10 + wspr_code(base_call[2]);
			n = n * 27 + (wspr_code(base_call[3]) - 10);
			n = n * 27 + (wspr_code(base_call[4]) - 10);
			n = n * 27 + (wspr_code(base_call[5]) - 10);

			m = 0;
			for (uint8_t i = 0; i < 3; ++i)
			{
				m = 37 * m + wspr_code(prefix[i]);
			}

			if (m >= 32768)
			{
				m -= 32768;
				m = (m * 128) + power + 2 + 64;
			}
			else
			{
				m = (m * 128) + power + 1 + 64;
			}
		}
	}

	// 打包 callsign 的 28 位到 c[0]~c[3]，低位在前
	// c[3] 低 4 位用于 callsign 的低 4 位，高 4 位留给 locator/power
	c[3] = (uint8_t)((n & 0x0f) << 4); // callsign 低 4 位放到 c[3] 高 4 位
	n = n >> 4;
	c[2] = (uint8_t)(n & 0xff);        // callsign 中间 8 位
	n = n >> 8;
	c[1] = (uint8_t)(n & 0xff);        // callsign 次高 8 位
	n = n >> 8;
	c[0] = (uint8_t)(n & 0xff);        // callsign 最高 8 位

	// 打包 locator/power 的 22 位到 c[3]~c[6]
	// c[6] 只用高 2 位，c[5]、c[4] 全部，c[3] 低 4 位
	c[6] = (uint8_t)((m & 0x03) << 6); // locator/power 低 2 位放到 c[6] 高 2 位
	m = m >> 2;
	c[5] = (uint8_t)(m & 0xff);        // locator/power 中间 8 位
	m = m >> 8;
	c[4] = (uint8_t)(m & 0xff);        // locator/power 次高 8 位
	m = m >> 8;
	c[3] |= (uint8_t)(m & 0x0f);       // locator/power 最高 4 位放到 c[3] 低 4 位
	m = m >> 8;
	c[3] |= (uint8_t)(m & 0x0f);
	c[7] = 0;
	c[8] = 0;
	c[9] = 0;
	c[10] = 0;
}

/**
 * @brief WSPR 消息预处理，包括呼号、网格和功率的校验与规范化。
 *
 * @param call 呼号字符串，最大 12 字符。
 * @param loc 网格定位字符串，4 或 6 字符。
 * @param dbm 功率（dBm）。
 *
 * 该函数会对输入的呼号、网格和功率进行合法性校验和标准化处理。
 */
void wspr_message_prep(char *call, char *loc, int8_t dbm)
{
	// 呼号校验与填充
	// -------------------------------

	// 只允许数字、大写字母、斜杠和尖括号
	uint8_t i;
	for (i = 0; i < 12; i++)
	{
		if (call[i] != '/' && call[i] != '<' && call[i] != '>')
		{
			call[i] = toupper(call[i]);
			if (!(isdigit(call[i]) || isupper(call[i])))
			{
				call[i] = ' ';
			}
		}
	}
	call[12] = 0;

	strncpy(callsign, call, 12);

	// 网格定位校验
	if (strlen(loc) == 4 || strlen(loc) == 6)
	{
		for (i = 0; i <= 1; i++)
		{
			loc[i] = toupper(loc[i]);
			if ((loc[i] < 'A' || loc[i] > 'R'))
			{
				strncpy(loc, "AA00AA", 7);
			}
		}
		for (i = 2; i <= 3; i++)
		{
			if (!(isdigit(loc[i])))
			{
				strncpy(loc, "AA00AA", 7);
			}
		}
	}
	else
	{
		strncpy(loc, "AA00AA", 7);
	}

	if (strlen(loc) == 6)
	{
		for (i = 4; i <= 5; i++)
		{
			loc[i] = toupper(loc[i]);
			if ((loc[i] < 'A' || loc[i] > 'X'))
			{
				strncpy(loc, "AA00AA", 7);
			}
		}
	}
	strncpy(locator, loc, 6);
	locator[6] = '\0';

	// 功率校验，只允许特定步进
	#define VALID_DBM_SIZE 28
	const int8_t valid_dbm[VALID_DBM_SIZE] =
		{-30, -27, -23, -20, -17, -13, -10, -7, -3,
		 0, 3, 7, 10, 13, 17, 20, 23, 27, 30, 33, 37, 40,
		 43, 47, 50, 53, 57, 60};
	// 默认赋值为最小值，防止未赋值
	power = valid_dbm[0];
	for (i = 0; i < VALID_DBM_SIZE; i++)
	{
		if (dbm == valid_dbm[i])
		{
			power = dbm;
		}
	}
	// 如果不是合法功率，向下取整
	for (i = 1; i < VALID_DBM_SIZE; i++)
	{
		if (dbm < valid_dbm[i] && dbm >= valid_dbm[i - 1])
		{
			power = valid_dbm[i - 1];
		}
	}
}

/*
 * @brief WSPR 编码主流程。
 *
 * @param call 呼号（最长 12 字符）。
 * @param loc Maidenhead 网格定位（最长 6 字符）。
 * @param dbm 输出功率（dBm）。
 * @param symbols 输出的符号数组，长度为 WSPR_SYMBOL_COUNT。
 *
 * 该函数将呼号、网格和功率编码为 WSPR 协议的符号序列。
 * 支持 Type 1、2、3 消息。
 */
void wspr_encode(const char *call, const char *loc, const int8_t dbm, uint8_t *symbols)
{
    char call_[13];
    char loc_[7];
    uint8_t dbm_ = dbm;
    strncpy(call_, call, 12);
    call_[12] = '\0';
    strncpy(loc_, loc, 6);
    loc_[6] = '\0';

    // 确保消息文本符合标准
    wspr_message_prep(call_, loc_, dbm_);

    // 比特打包
    uint8_t c[WSPR_MESSAGE_BYTE_SIZE];
    wspr_bit_packing(c);

    // 卷积编码
    uint8_t s[WSPR_SYMBOL_COUNT];
    convolve(c, s, WSPR_MESSAGE_BYTE_SIZE, WSPR_BIT_COUNT);

    // 交织处理
    wspr_interleave(s);

    // 合并同步向量
    wspr_merge_sync_vector(s, symbols);
}


