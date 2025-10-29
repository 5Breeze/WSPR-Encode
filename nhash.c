/*
 *-------------------------------------------------------------------------------
 *
 * 本文件是 WSPR 应用程序（弱信号传播报告器）的一部分
 *
 * 文件名:   nhash.c
 * 描述: 用于哈希表查找的 32 位哈希函数
 *
 * 以下为原文版权声明，仅供参考：
 * Copyright (C) 2008-2014 Joseph Taylor, K1JT
 * License: GPL-3
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
 * Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Files: lookup3.c
 * Copyright: Copyright (C) 2006 Bob Jenkins <bob_jenkins@burtleburtle.net>
 * License: public-domain
 *  You may use this code any way you wish, private, educational, or commercial.
 *  It's free.
 *
 *-------------------------------------------------------------------------------
*/

/*
这些函数用于生成用于哈希表查找的 32 位哈希值。
hashword()、hashlittle()、hashlittle2()、hashbig()、mix() 和 final() 
是对外有用的函数。如果定义了 SELF_TEST，则包含用于测试哈希的例程。
你可以免费用于任何目的。它是公有领域的。没有任何担保。

你可能想用 hashlittle()。hashlittle() 和 hashbig() 用于哈希字节数组。
hashlittle() 在小端机器上比 hashbig() 更快。Intel 和 AMD 都是小端机器。
再想想，你可能更想用 hashlittle2()，它和 hashlittle() 一样，但能一次返回两个 32 位哈希。
你也可以实现 hashbig2()，但这里没有实现。

如果你想对正好 7 个整数求哈希，可以这样：
  a = i1;  b = i2;  c = i3;
  mix(a,b,c);
  a += i4; b += i5; c += i6;
  mix(a,b,c);
  a += i7;
  final(a,b,c);
然后用 c 作为哈希值。如果你有一个变长的 4 字节整数数组要哈希，用 hashword()。
如果你有一个字节数组（如字符串），用 hashlittle()。
如果你有多个字节数组或混合类型，见 hashlittle() 上方注释。

为什么这么大？我每次读取 12 字节到 3 个 4 字节整数，然后混合这些整数。
这样很快（你可以用 12*3 条指令在 3 个整数上做更彻底的混合），但高效地把这些字节塞进整数很麻烦。
*/

#define SELF_TEST 1

#include <stdio.h>      /* 定义测试用 printf */
#include <time.h>       /* 定义测试用 time_t */
#ifdef Win32
#include "win_stdint.h" /* 定义 uint32_t 等 */
#else
#include <stdint.h> /* 定义 uint32_t 等 */
#endif
// #include <sys/param.h>  /* 试图定义字节序 */
// #ifdef linux
// # include <endian.h>    /* 试图定义字节序 */
// #endif

#define HASH_LITTLE_ENDIAN 1

#define hashsize(n) ((uint32_t)1<<(n))
#define hashmask(n) (hashsize(n)-1)
#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))

/*
-------------------------------------------------------------------------------
mix -- 可逆地混合 3 个 32 位值。

这是可逆的，所以 mix() 前 (a,b,c) 的所有信息在 mix() 后 (a,b,c) 里仍然存在。

如果四组 (a,b,c) 输入通过 mix() 或反向 mix()，输出的 32 位中至少有 32 位有时相同有时不同。
测试内容：
* 仅一位或两位不同的输入对，在 (a,b,c) 顶部或底部任意组合。
* “不同”定义为 +、-、^ 或 ~^。对于 + 和 -，我把输出差值转为格雷码（a^(a>>1)），这样一串 1 看起来像单个 1 位差。
* 基值为伪随机、仅一位为 1 或全零加递增计数。

以下 k 值满足我的 "a-=c; a^=rot(c,k); c+=b;" 结构：
    4  6  8 16 19  4
    9 15  3 18 27 15
   14  9  3  7 17  3
不过 "9 15 3 18 27 15" 对于 + 和一位基两位差没完全达到 32 位差。
我用 http://burtleburtle.net/bob/hash/avalanche.html 选了操作、常数和变量排列。

这没达到雪崩效应。输入 (a,b,c) 的某些位不会影响输出 (a,b,c) 的某些位，尤其是 a。
混合最彻底的是 c，但也没完全雪崩。

这样允许一定并行性。读后写有助于影响更多位，所以混合目标和并行目标有冲突。
我尽力了。旋转和移位在所有机器上耗时差不多，旋转对顶底位更友好，所以我用了旋转。
-------------------------------------------------------------------------------
*/
#define mix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}

/*
-------------------------------------------------------------------------------
final -- 最终混合 3 个 32 位值 (a,b,c) 到 c

仅有少数位不同的 (a,b,c) 对通常会产生完全不同的 c。
测试内容：
* 仅一位或两位不同的输入对，在 (a,b,c) 顶部或底部任意组合。
* “不同”定义为 +、-、^ 或 ~^。对于 + 和 -，我把输出差值转为格雷码（a^(a>>1)），这样一串 1 看起来像单个 1 位差。
* 基值为伪随机、仅一位为 1 或全零加递增计数。

这些常数通过了测试：
 14 11 25 16 4 14 24
 12 14 25 16 4 14 24
以下也接近：
  4  8 15 26 3 22 24
 10  8 15 26 3 22 24
 11  8 15 26 3 22 24
-------------------------------------------------------------------------------
*/
#define final(a,b,c) \
{ \
  c ^= b; c -= rot(b,14); \
  a ^= c; a -= rot(c,11); \
  b ^= a; b -= rot(a,25); \
  c ^= b; c -= rot(b,16); \
  a ^= c; a -= rot(c,4);  \
  b ^= a; b -= rot(a,14); \
  c ^= b; c -= rot(b,24); \
}

/*
-------------------------------------------------------------------------------
hashlittle() -- 将变长 key 哈希为 32 位值
  k       : key（未对齐的变长字节数组）
  length  : key 长度，单位字节
  initval : 任意 4 字节值
返回 32 位值。key 的每一位都会影响返回值的每一位。仅一两位不同的 key 会有完全不同的哈希值。

哈希表大小最好为 2 的幂。无需取模素数（mod 很慢！）。如果只需 10 位，用位掩码：
  h = (h & hashmask(10));
此时哈希表应有 hashsize(10) 个元素。

如果要哈希 n 个字符串 (uint8_t **)k，可这样：
  for (i=0, h=0; i<n; ++i) h = hashlittle( k[i], len[i], h);

By Bob Jenkins, 2006.  bob_jenkins@burtleburtle.net. 你可任意使用本代码，私人、教育或商业用途均可。免费。

用于哈希表查找或任何可接受 2^^32 次碰撞一次的场合。不要用于加密。
-------------------------------------------------------------------------------
*/

//uint32_t hashlittle( const void *key, size_t length, uint32_t initval)
#ifdef STDCALL
uint32_t __stdcall NHASH( const void *key, size_t *length0, uint32_t *initval0)
#else
uint32_t nhash_( const void *key, int *length0, uint32_t *initval0)
#endif
{
  uint32_t a,b,c;                                          /* internal state */
  size_t length;
  uint32_t initval;
  union { const void *ptr; size_t i; } u;     /* needed for Mac Powerbook G4 */

  length=*length0;
  initval=*initval0;

  /* Set up the internal state */
  a = b = c = 0xdeadbeef + ((uint32_t)length) + initval;

  u.ptr = key;
  if (HASH_LITTLE_ENDIAN && ((u.i & 0x3) == 0)) {
    const uint32_t *k = (const uint32_t *)key;         /* read 32-bit chunks */
    const uint8_t  *k8;

    k8=0;                                     //Silence compiler warning
    /*------ all but last block: aligned reads and affect 32 bits of (a,b,c) */
    while (length > 12)
    {
      a += k[0];
      b += k[1];
      c += k[2];
      mix(a,b,c);
      length -= 12;
      k += 3;
    }

    /*----------------------------- handle the last (probably partial) block */
    /* 
     * "k[2]&0xffffff" actually reads beyond the end of the string, but
     * then masks off the part it's not allowed to read.  Because the
     * string is aligned, the masked-off tail is in the same word as the
     * rest of the string.  Every machine with memory protection I've seen
     * does it on word boundaries, so is OK with this.  But VALGRIND will
     * still catch it and complain.  The masking trick does make the hash
     * noticably faster for short strings (like English words).
     */
#ifndef VALGRIND

    switch(length)
    {
    case 12: c+=k[2]; b+=k[1]; a+=k[0]; break;
    case 11: c+=k[2]&0xffffff; b+=k[1]; a+=k[0]; break;
    case 10: c+=k[2]&0xffff; b+=k[1]; a+=k[0]; break;
    case 9 : c+=k[2]&0xff; b+=k[1]; a+=k[0]; break;
    case 8 : b+=k[1]; a+=k[0]; break;
    case 7 : b+=k[1]&0xffffff; a+=k[0]; break;
    case 6 : b+=k[1]&0xffff; a+=k[0]; break;
    case 5 : b+=k[1]&0xff; a+=k[0]; break;
    case 4 : a+=k[0]; break;
    case 3 : a+=k[0]&0xffffff; break;
    case 2 : a+=k[0]&0xffff; break;
    case 1 : a+=k[0]&0xff; break;
    case 0 : return c;              /* zero length strings require no mixing */
    }

#else /* make valgrind happy */

    k8 = (const uint8_t *)k;
    switch(length)
    {
    case 12: c+=k[2]; b+=k[1]; a+=k[0]; break;
    case 11: c+=((uint32_t)k8[10])<<16;  /* fall through */
    case 10: c+=((uint32_t)k8[9])<<8;    /* fall through */
    case 9 : c+=k8[8];                   /* fall through */
    case 8 : b+=k[1]; a+=k[0]; break;
    case 7 : b+=((uint32_t)k8[6])<<16;   /* fall through */
    case 6 : b+=((uint32_t)k8[5])<<8;    /* fall through */
    case 5 : b+=k8[4];                   /* fall through */
    case 4 : a+=k[0]; break;
    case 3 : a+=((uint32_t)k8[2])<<16;   /* fall through */
    case 2 : a+=((uint32_t)k8[1])<<8;    /* fall through */
    case 1 : a+=k8[0]; break;
    case 0 : return c;
    }

#endif /* !valgrind */

  } else if (HASH_LITTLE_ENDIAN && ((u.i & 0x1) == 0)) {
    const uint16_t *k = (const uint16_t *)key;         /* read 16-bit chunks */
    const uint8_t  *k8;

    /*--------------- all but last block: aligned reads and different mixing */
    while (length > 12)
    {
      a += k[0] + (((uint32_t)k[1])<<16);
      b += k[2] + (((uint32_t)k[3])<<16);
      c += k[4] + (((uint32_t)k[5])<<16);
      mix(a,b,c);
      length -= 12;
      k += 6;
    }

    /*----------------------------- handle the last (probably partial) block */
    k8 = (const uint8_t *)k;
    switch(length)
    {
    case 12: c+=k[4]+(((uint32_t)k[5])<<16);
             b+=k[2]+(((uint32_t)k[3])<<16);
             a+=k[0]+(((uint32_t)k[1])<<16);
             break;
    case 11: c+=((uint32_t)k8[10])<<16;     /* fall through */
    case 10: c+=k[4];
             b+=k[2]+(((uint32_t)k[3])<<16);
             a+=k[0]+(((uint32_t)k[1])<<16);
             break;
    case 9 : c+=k8[8];                      /* fall through */
    case 8 : b+=k[2]+(((uint32_t)k[3])<<16);
             a+=k[0]+(((uint32_t)k[1])<<16);
             break;
    case 7 : b+=((uint32_t)k8[6])<<16;      /* fall through */
    case 6 : b+=k[2];
             a+=k[0]+(((uint32_t)k[1])<<16);
             break;
    case 5 : b+=k8[4];                      /* fall through */
    case 4 : a+=k[0]+(((uint32_t)k[1])<<16);
             break;
    case 3 : a+=((uint32_t)k8[2])<<16;      /* fall through */
    case 2 : a+=k[0];
             break;
    case 1 : a+=k8[0];
             break;
    case 0 : return c;                     /* zero length requires no mixing */
    }

  } else {                        /* need to read the key one byte at a time */
    const uint8_t *k = (const uint8_t *)key;

    /*--------------- all but the last block: affect some 32 bits of (a,b,c) */
    while (length > 12)
    {
      a += k[0];
      a += ((uint32_t)k[1])<<8;
      a += ((uint32_t)k[2])<<16;
      a += ((uint32_t)k[3])<<24;
      b += k[4];
      b += ((uint32_t)k[5])<<8;
      b += ((uint32_t)k[6])<<16;
      b += ((uint32_t)k[7])<<24;
      c += k[8];
      c += ((uint32_t)k[9])<<8;
      c += ((uint32_t)k[10])<<16;
      c += ((uint32_t)k[11])<<24;
      mix(a,b,c);
      length -= 12;
      k += 12;
    }

    /*-------------------------------- last block: affect all 32 bits of (c) */
    switch(length)                   /* all the case statements fall through */
    {
    case 12: c+=((uint32_t)k[11])<<24; /* fall through */
    case 11: c+=((uint32_t)k[10])<<16; /* fall through */
    case 10: c+=((uint32_t)k[9])<<8;   /* fall through */
    case 9 : c+=k[8];                  /* fall through */
    case 8 : b+=((uint32_t)k[7])<<24;  /* fall through */
    case 7 : b+=((uint32_t)k[6])<<16;  /* fall through */
    case 6 : b+=((uint32_t)k[5])<<8;   /* fall through */
    case 5 : b+=k[4];                  /* fall through */
    case 4 : a+=((uint32_t)k[3])<<24;  /* fall through */
    case 3 : a+=((uint32_t)k[2])<<16;  /* fall through */
    case 2 : a+=((uint32_t)k[1])<<8;   /* fall through */
    case 1 : a+=k[0];
             break;
    case 0 : return c;
    }
  }

  final(a,b,c);
  return c;
}

//uint32_t __stdcall NHASH(const void *key, size_t length, uint32_t initval)
