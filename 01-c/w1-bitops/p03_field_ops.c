#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* ============================================================
 * p03：多位字段操作 —— 一个寄存器里塞好几个字段
 *
 * 真实外设寄存器从来不是「一位一个功能」，而是把好几个字段挤进 32 位：
 *
 *     bit0-3   分频系数      4 位
 *     bit4-11  比较值        8 位
 *     bit12-15 模式          4 位
 *
 * 读写这种字段有三步，缺一不可：
 *   1. 造掩码：把 width 个 1 挪到 pos 位置
 *   2. 清旧值：*reg &= ~mask   ← 最容易漏，漏了就变成「叠加」而不是「赋值」
 *   3. 写新值：*reg |= (val << pos) & mask
 *
 * 第 2 步是这题的核心。少了它，先写 0xF 再写 0x3，寄存器里是 0xF。
 * ============================================================ */

enum { REG_BITS = 32 };

/* 造掩码：从 pos 开始的 width 个 1。
 * 注意 width == 32 时 1u << 32 是未定义行为，要单独处理。 */
static uint32_t field_mask(int pos, int width)
{
    if (width >= REG_BITS) { return 0xFFFFFFFFu; }
    return ((1u << width) - 1u) << pos;
}

/* 参数是否合法：位置非负、宽度 1~32、且不跨出 32 位 */
static int field_ok(int pos, int width)
{
    return pos >= 0 && width >= 1 && width <= REG_BITS && pos + width <= REG_BITS;
}

/* TODO 1：把 reg 里 [pos, pos+width) 这段字段设成 val
 *
 *   - reg 为 NULL、或 field_ok(pos, width) 为假 → 直接 return
 *   - 三步走：造掩码 → 清旧值 → 写新值
 *   - val 超宽要截断（val = 0x1F 写进 4 位字段，落进去的是 0xF）
 *   - 注意先 (val & mask_bits) 再左移，否则 val 很大时左移会溢出 32 位
 */
void field_set(uint32_t *reg, int pos, int width, uint32_t val)
{
    if (reg == NULL || field_ok(pos, width) == 0)
    {
        return;
    }
    
    uint32_t a = field_mask(pos, width);
    *reg &= ~a;
    *reg |= (val << pos)&a;

    /* TODO：见上方注释 */
}

/* TODO 2：读出 reg 里 [pos, pos+width) 这段字段
 *
 *   - 参数不合法 → 返回 0
 *   - 先右移到最低位，再用掩码砍掉高位
 *   - 顺序反了（先掩码后右移）结果一样，但代码难读；习惯上先移后掩
 */
uint32_t field_get(uint32_t reg, int pos, int width)
{
     if (!field_ok(pos, width)) {
        return 0u;
    }
    
    uint32_t a = field_mask(pos, width);
    return (reg & a)>>pos;

}

static int pass = 0, fail = 0;
static void ck(const char *name, long long got, long long want)
{
    if (got == want) { pass++; printf("  [PASS] %-46s = %lld\n", name, got); }
    else { fail++; printf("  [FAIL] %-46s 期望 %lld，实际 %lld\n", name, want, got); }
}

int main(void)
{
    printf("===== p03 多位字段读写 =====\n");

    uint32_t r = 0;

    printf("-- 写字段 --\n");
    r = 0; field_set(&r, 0, 4, 0xF);
    ck("set(pos0, w4, 0xF) → 0x0000000F", r, 0x0000000F);
    r = 0; field_set(&r, 4, 8, 0xA5);
    ck("set(pos4, w8, 0xA5) → 0x00000A50", r, 0x00000A50);
    r = 0; field_set(&r, 12, 4, 0x3);
    ck("set(pos12, w4, 0x3) → 0x00003000", r, 0x00003000);

    printf("-- 读字段 --\n");
    ck("get(0x00000A50, pos4, w8) = 0xA5", field_get(0x00000A50u, 4, 8), 0xA5);
    ck("get(0x0000F000, pos12, w4) = 0xF", field_get(0x0000F000u, 12, 4), 0xF);
    ck("get 不存在的字段 = 0", field_get(0x00000000u, 20, 4), 0);

    printf("-- 先清后置：连续写不叠加 --\n");
    r = 0;
    field_set(&r, 0, 4, 0xF);
    field_set(&r, 0, 4, 0x3);
    ck("先写 0xF 再写 0x3 → 0x3（不是 0xF）", r, 0x03);

    printf("-- 超宽截断 --\n");
    r = 0; field_set(&r, 0, 4, 0x1F);
    ck("set(pos0, w4, 0x1F) → 0xF（高位丢弃）", r, 0x0F);
    r = 0; field_set(&r, 8, 4, 0xAB);
    ck("set(pos8, w4, 0xAB) → 0xB00", r, 0xB00);

    printf("-- 只改目标字段，邻居不动 --\n");
    r = 0xFFFFFFFFu;
    field_set(&r, 0, 4, 0);
    ck("全 1 里清低 4 位 → 0xFFFFFFF0", (long long)r, 0xFFFFFFF0LL);
    r = 0x00000A50u;
    field_set(&r, 12, 4, 0x7);
    ck("改 pos12 时 pos4 的 0xA5 还在", (long long)r, 0x00007A50LL);

    printf("-- width = 32 的整寄存器 --\n");
    r = 0; field_set(&r, 0, 32, 0xDEADBEEFu);
    ck("set(pos0, w32, 0xDEADBEEF)", (long long)r, 0xDEADBEEFLL);
    ck("get(pos0, w32) = 0xDEADBEEF", (long long)field_get(r, 0, 32), 0xDEADBEEFLL);

    printf("-- 越界与非法参数 --\n");
    r = 0x12345678u; field_set(&r, 0, 0, 0xF);
    ck("width = 0 不写", r, 0x12345678);
    r = 0x12345678u; field_set(&r, 28, 8, 0xF);
    ck("pos+w > 32 不写", r, 0x12345678);
    r = 0x12345678u; field_set(&r, -1, 4, 0xF);
    ck("pos = -1 不写", r, 0x12345678);
    r = 0x12345678u; field_set(&r, 0, 33, 0xF);
    ck("width = 33 不写", r, 0x12345678);
    field_set(NULL, 0, 4, 0xF);
    ck("reg 为 NULL 不崩", 1, 1);
    ck("get 参数非法返回 0", (long long)field_get(0xFFFFFFFFu, 30, 8), 0);

    printf("-- 真实一点的用法：一个 GPIO 配置寄存器 --\n");
    uint32_t cfg = 0;
    field_set(&cfg, 0, 4, 0x2);      /* 分频 2 */
    field_set(&cfg, 4, 8, 0xC8);     /* 比较值 200 */
    field_set(&cfg, 12, 4, 0x5);     /* 模式 5 */
    ck("三个字段拼成 0x00005C82", (long long)cfg, 0x00005C82LL);
    ck("  读回分频", (long long)field_get(cfg, 0, 4), 0x2);
    ck("  读回比较值", (long long)field_get(cfg, 4, 8), 0xC8);
    ck("  读回模式", (long long)field_get(cfg, 12, 4), 0x5);

    printf("结果: %d 通过, %d 失败\n", pass, fail);
    return fail != 0;
}
