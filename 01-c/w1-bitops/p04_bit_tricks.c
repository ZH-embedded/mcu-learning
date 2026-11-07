#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* ============================================================
 * p04：四个能进面试的位技巧
 *
 * 前三个是「一句话就能答、但要求能推导」的经典题，第四个是工程里
 * 真会用到的小工具。共同点：**都不需要循环 32 次**，除了反转位序。
 *
 * 记住一个前提：所有运算都在无符号数上做。有符号数的右移是「实现定义」
 * （算术右移还是逻辑右移看编译器），而 0 - v 在有符号里是溢出 UB。
 * ============================================================ */

/* TODO 1：数一数 v 里有几个 1（popcount）
 *
 *   - 经典写法（Brian Kernighan）：v &= (v - 1) 每次清掉最低的那个 1，
 *     循环次数等于 1 的个数，最坏 32 次、最好 0 次
 *   - 笨写法逐位移 32 次也能对，但面试官问的就是这个套路
 *   - 为什么 v & (v-1) 能清最低位：减一让最低的 1 变 0、它右边的 0 全变 1，
 *     与一下，那个 1 就没了，高位不受影响
 */
int count_ones(uint32_t v)
{
    (void)v;

    /* TODO：见上方注释 */
    return 0;
}

/* TODO 2：只保留最低位的那个 1，其余清零
 *
 *   - 一行：v & (0u - v)，也就是 v & -v
 *   - 0u - v 是无符号取负：等价于 ~v + 1，低位的 0 全变 1、最低的 1 保留
 *   - 注意必须写 0u - v，不能写 -v 再和 uint32_t 相与（那是有符号取负）
 *   - 用途：遍历一个集合里的所有元素（每次取最低位、清掉、再取）
 */
uint32_t lowest_bit(uint32_t v)
{
    (void)v;

    /* TODO：见上方注释 */
    return 0u;
}

/* TODO 3：判断 v 是不是 2 的幂（0 不是）
 *
 *   - v != 0 且 (v & (v - 1)) == 0
 *   - 2 的幂只有一个 1，减一之后那一位变 0、低位全 1，与起来必为 0
 *   - **括号不能少**：(v & (v - 1)) == 0，漏一层就变成 v & ((v-1) == 0)
 */
int is_power_of_two_u(uint32_t v)
{
    (void)v;

    /* TODO：见上方注释 */
    return 0;
}

/* TODO 4：把一个字节的 8 个位顺序倒过来（bit0 ↔ bit7）
 *
 *   - 循环 8 次：每次把结果左移一位，接上 b 的最低位，b 右移一位
 *   - 用途：SPI 的 LSB-first / MSB-first、位域和外部协议的位序对齐
 *   - 自检：反两次必须回到原值（0x96 → 0x69 → 0x96）
 */
uint8_t reverse_bits8(uint8_t b)
{
    (void)b;

    /* TODO：见上方注释 */
    return 0u;
}

/* TODO 5：把 32 位整数的 4 个字节顺序颠倒（大小端互换）
 *
 *   - 就是 htonl / ntohl 干的事：0x11223344 → 0x44332211
 *   - 写法一（移位版）：(v >> 24) | ((v >> 8) & 0xFF00) | ((v << 8) & 0xFF0000) | (v << 24)
 *   - 写法二（字节指针版）：拷到 uint8_t[4] 里倒着拼，可读性更好
 *   - 自检：换两次必须回到原值
 */
uint32_t swap_u32(uint32_t v)
{
    (void)v;

    /* TODO：见上方注释 */
    return 0u;
}

static int pass = 0, fail = 0;
static void ck(const char *name, long long got, long long want)
{
    if (got == want) { pass++; printf("  [PASS] %-44s = %lld\n", name, got); }
    else { fail++; printf("  [FAIL] %-44s 期望 %lld，实际 %lld\n", name, want, got); }
}

int main(void)
{
    printf("===== p04 位技巧 =====\n");

    printf("-- 数 1 的个数 --\n");
    ck("count_ones(0)", count_ones(0), 0);
    ck("count_ones(1)", count_ones(1), 1);
    ck("count_ones(0x80000000)", count_ones(0x80000000u), 1);
    ck("count_ones(0xA5)", count_ones(0xA5u), 4);
    ck("count_ones(0xFFFFFFFF)", count_ones(0xFFFFFFFFu), 32);

    int same = 1;
    for (int b = 0; b < 32; b++)
    {
        if (count_ones(1u << b) != 1) { same = 0; }
    }
    ck("每个单 bit 值都只数出 1 个", same, 1);

    printf("-- 最低位的 1 --\n");
    ck("lowest_bit(0)", (long long)lowest_bit(0), 0);
    ck("lowest_bit(1)", (long long)lowest_bit(1), 1);
    ck("lowest_bit(0x0C) = 4", (long long)lowest_bit(0x0Cu), 4);
    ck("lowest_bit(0xFF00) = 0x100", (long long)lowest_bit(0xFF00u), 0x100LL);
    ck("lowest_bit(0x80000000)", (long long)lowest_bit(0x80000000u), 0x80000000LL);

    printf("-- 判断 2 的幂 --\n");
    ck("is_pow2(0)", is_power_of_two_u(0), 0);
    ck("is_pow2(1)", is_power_of_two_u(1), 1);
    ck("is_pow2(64)", is_power_of_two_u(64), 1);
    ck("is_pow2(96)", is_power_of_two_u(96), 0);
    ck("is_pow2(0x80000000)", is_power_of_two_u(0x80000000u), 1);
    ck("is_pow2(0xFFFFFFFF)", is_power_of_two_u(0xFFFFFFFFu), 0);

    printf("-- 字节内位序反转 --\n");
    /* 注意：0xA5 是 8 位回文（1010 0101），反转后还是自己，
     * 拿它当样例的话，写错成「原样返回」也会过。要用不对称的值。 */
    ck("reverse8(0x96) = 0x69", reverse_bits8(0x96), 0x69);
    ck("reverse8(0x01) = 0x80", reverse_bits8(0x01), 0x80);
    ck("reverse8(0x80) = 0x01", reverse_bits8(0x80), 0x01);
    ck("reverse8(0x00) = 0x00", reverse_bits8(0x00), 0x00);
    ck("reverse8(0xFF) = 0xFF", reverse_bits8(0xFF), 0xFF);
    ck("reverse8 两次回到原值", reverse_bits8(reverse_bits8(0x96)), 0x96);

    printf("-- 字节顺序反转 --\n");
    ck("swap_u32(0x11223344)", (long long)swap_u32(0x11223344u), 0x44332211LL);
    ck("swap_u32(0xAABBCCDD)", (long long)swap_u32(0xAABBCCDDu), 0xDDCCBBAAu);
    ck("swap_u32(0x000000FF)", (long long)swap_u32(0x000000FFu), 0xFF000000LL);
    ck("swap_u32 两次回到原值", (long long)swap_u32(swap_u32(0x12345678u)), 0x12345678LL);

    printf("结果: %d 通过, %d 失败\n", pass, fail);
    return fail != 0;
}
