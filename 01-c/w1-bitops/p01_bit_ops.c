#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* ============================================================
 * p01：位操作四件套 —— 置位 / 清位 / 取位 / 翻转
 *
 * 这四个动作是嵌入式里出现频率最高的代码：寄存器配置、标志位管理、
 * 中断开关，全都是它们的排列组合。
 *
 * 本文件用「指针 + 位号」的**函数**形式实现，p02 会用**宏**重写一遍。
 * 写完你会自己看到宏为什么能赢：函数拿到的 bit 永远是运行时变量，
 * 编译器没法把它折叠成常量；宏在调用点展开，位号可以是编译期常量。
 *
 * 三条纪律，四个函数通用：
 *   1. bit 必须在 [0, 32)，越界不能崩、也不能乱写
 *   2. 置位/清位/翻转都是「读-改-写」：整个寄存器读进来，改一位，写回去
 *   3. 只能动目标那一位，其余 31 位必须原样保留
 * ============================================================ */

enum { REG_BITS = 32 };

/* TODO 1：置位 —— 第 bit 位置 1，其余位不动
 *
 *   - reg 为 NULL、或 bit 不在 [0, 32) → 直接 return
 *   - 惯用法：*reg |= (1u << bit)
 *   - 是 1u 不是 1：1 << 31 在 32 位 int 上是未定义行为
 */
void bit_set(uint32_t *reg, int bit)
{
    (void)reg;
    (void)bit;

    /* TODO：见上方注释 */
}

/* TODO 2：清位 —— 第 bit 位置 0，其余位不动
 *
 *   - 边界同 TODO 1
 *   - 惯用法：*reg &= ~(1u << bit)
 *   - 为什么不是 *reg &= (0u << bit)：0 << bit 恒为 0，会把整个寄存器清空
 */
void bit_clear(uint32_t *reg, int bit)
{
    (void)reg;
    (void)bit;

    /* TODO：见上方注释 */
}

/* TODO 3：取位 —— 返回第 bit 位，只可能是 0 或 1
 *
 *   - bit 越界 → 返回 0（这里没有指针可判，reg 按值传）
 *   - 惯用法：(reg >> bit) & 1u
 *   - 少了 & 1u 就会把高位一起带回来，那是「掩码」不是「取值」
 */
int bit_get(uint32_t reg, int bit)
{
    (void)reg;
    (void)bit;

    /* TODO：见上方注释 */
    return 0;
}

/* TODO 4：翻转 —— 第 bit 位取反，其余位不动
 *
 *   - 边界同 TODO 1
 *   - 惯用法：*reg ^= (1u << bit)
 *   - 连续翻转两次必须回到原值，这是最省事的自检
 */
void bit_toggle(uint32_t *reg, int bit)
{
    (void)reg;
    (void)bit;

    /* TODO：见上方注释 */
}

/* 把 32 位打印成二进制，只用于看错在哪一位 */
static void show_bits(const char *tag, uint32_t v)
{
    printf("    %-12s ", tag);
    for (int i = 31; i >= 0; i--)
    {
        putchar(((v >> i) & 1u) ? '1' : '0');
        if (i % 8 == 0 && i != 0) { putchar(' '); }
    }
    printf("  0x%08X\n", v);
}

static int pass = 0, fail = 0;
static void ck(const char *name, long long got, long long want)
{
    if (got == want) { pass++; printf("  [PASS] %-44s = %lld\n", name, got); }
    else { fail++; printf("  [FAIL] %-44s 期望 %lld，实际 %lld\n", name, want, got); }
}

int main(void)
{
    printf("===== p01 位操作四件套 =====\n");

    uint32_t r = 0;

    printf("-- 置位 --\n");
    r = 0; bit_set(&r, 0);
    ck("set bit0 → 1", r, 0x00000001);
    r = 0; bit_set(&r, 31);
    ck("set bit31 → 0x80000000", (long long)r, 0x80000000LL);
    r = 0; bit_set(&r, 15);
    ck("set bit15 → 0x00008000", r, 0x00008000);
    r = 0x00000001; bit_set(&r, 0);
    ck("重复 set bit0 不变（幂等）", r, 0x00000001);
    r = 0; bit_set(&r, 0); bit_set(&r, 31);
    ck("set bit0 + bit31 → 0x80000001", (long long)r, 0x80000001LL);

    printf("-- 清位 --\n");
    r = 0xFFFFFFFF; bit_clear(&r, 0);
    ck("clear bit0 → 0xFFFFFFFE", (long long)r, 0xFFFFFFFELL);
    r = 0xFFFFFFFF; bit_clear(&r, 31);
    ck("clear bit31 → 0x7FFFFFFF", r, 0x7FFFFFFF);
    r = 0; bit_clear(&r, 5);
    ck("clear 本来就 0 的位，不变", r, 0);
    r = 0x80000001; bit_clear(&r, 31);
    ck("clear bit31 后 bit0 还在", r, 0x00000001);

    printf("-- 取位 --\n");
    ck("get(0x80000001, 0)  = 1", bit_get(0x80000001u, 0), 1);
    ck("get(0x80000001, 31) = 1", bit_get(0x80000001u, 31), 1);
    ck("get(0x80000001, 30) = 0", bit_get(0x80000001u, 30), 0);
    ck("get 不改寄存器", (bit_get(0xFFFFFFFFu, 7), 1), 1);

    printf("-- 翻转 --\n");
    r = 0; bit_toggle(&r, 3);
    ck("toggle bit3 → 0x08", r, 0x08);
    bit_toggle(&r, 3);
    ck("再 toggle 一次回到 0", r, 0);
    r = 0xFFFFFFFF; bit_toggle(&r, 0);
    ck("toggle 全 1 的最低位 → 0xFFFFFFFE", (long long)r, 0xFFFFFFFELL);
    r = 0x80000000; bit_toggle(&r, 31); bit_toggle(&r, 31);
    ck("bit31 连翻两次回到原值", (long long)r, 0x80000000LL);

    printf("-- 越界与 NULL --\n");
    r = 0x12345678; bit_set(&r, -1);
    ck("set(-1) 不改", r, 0x12345678);
    r = 0x12345678; bit_set(&r, 32);
    ck("set(32) 不改", r, 0x12345678);
    r = 0x12345678; bit_clear(&r, 32);
    ck("clear(32) 不改", r, 0x12345678);
    r = 0x12345678; bit_toggle(&r, 99);
    ck("toggle(99) 不改", r, 0x12345678);
    ck("get(-1) 返回 0", bit_get(0xFFFFFFFFu, -1), 0);
    ck("get(32) 返回 0", bit_get(0xFFFFFFFFu, 32), 0);
    bit_set(NULL, 0); bit_clear(NULL, 0); bit_toggle(NULL, 0);
    ck("set/clear/toggle 传 NULL 不崩", 1, 1);

    printf("-- 全 32 位扫描 --\n");
    int ok_one = 1, ok_get = 1;
    for (int b = 0; b < REG_BITS; b++)
    {
        r = 0;
        bit_set(&r, b);
        if (r != (1u << b)) { ok_one = 0; show_bits("set 出错", r); }
        if (bit_get(r, b) != 1) { ok_get = 0; }
    }
    ck("逐位 set：每次只亮一位且位置正确", ok_one, 1);
    ck("逐位 get：与 set 一致", ok_get, 1);

    r = 0;
    for (int b = 0; b < REG_BITS; b++) { bit_set(&r, b); }
    ck("32 位全置 → 0xFFFFFFFF", (long long)r, 0xFFFFFFFFLL);
    for (int b = 0; b < REG_BITS; b++) { bit_clear(&r, b); }
    ck("32 位全清 → 0", r, 0);

    printf("结果: %d 通过, %d 失败\n", pass, fail);
    return fail != 0;
}
