#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* ============================================================
 * p02：寄存器宏封装 —— REG_SET / REG_CLR / REG_GET / REG_TOGGLE
 *
 * 上一题用函数实现同样的四个动作，这一题改成宏。差别不在写法好不好看，
 * 而在于两件实打实的事：
 *
 *   1. 位号可以是编译期常量。宏在调用点展开，REG_SET(reg, 3) 里的 3
 *      直接变成 1u << 3，汇编里是一条立即数指令；函数收到的是运行时
 *      参数，只能先算 1u << bit 再改。
 *   2. 没有函数调用。Cortex-M0 上 BL/RET 要 6~8 个周期，宏是零。
 *
 * 代价是宏没有类型检查、参数会被重复求值，所以要靠括号和命名纪律兜住。
 *
 * 四条宏的括号纪律（写的时候自查）：
 *   - 每个参数都加括号：(reg) (bit)
 *   - 整体再加一层括号：整个宏是个表达式，参与更大表达式时不能被拆散
 *   - 常数写 1u：1 << 31 在 32 位 int 上是未定义行为
 *   - 宏名和左括号之间不能有空格，否则退化成对象式宏
 * ============================================================ */

/* TODO 1：置位 —— 第 bit 位置 1，其余位不动 */
#define REG_SET(reg, bit)    ((void)(reg), (void)(bit))                    /* TODO */

/* TODO 2：清位 —— 第 bit 位置 0，其余位不动
 *   提示：`reg &= (0u << bit)` 会把整个寄存器清空，因为 0u << bit 恒为 0 */
#define REG_CLR(reg, bit)    ((void)(reg), (void)(bit))                    /* TODO */

/* TODO 3：取位 —— 只返回 0 或 1
 *   提示：只写 (reg) & (1u << bit) 得到的是「掩码」，不是「取值」 */
#define REG_GET(reg, bit)    ((void)(reg), (void)(bit), 0u)               /* TODO */

/* TODO 4：翻转 —— 第 bit 位取反，其余位不动 */
#define REG_TOGGLE(reg, bit) ((void)(reg), (void)(bit))                    /* TODO */

/* ---------- 反面教材：不要模仿 ---------- */
/* 1. 参数没括号：BAD_SET(r, 7 & 3) 展开成 r |= 1 << 7 & 3，
 *    `&` 比 `<<` 优先级低，于是先算 (1<<7) 再 & 3，结果是 0 —— 这一位直接丢了。 */
#define BAD_SET(reg, bit)    reg |= 1 << bit
/* 2. 取位返回掩码：BAD_GET(r, 8) 在 r=0x100 时给 0x100，不是 1。
 *    写成 if (BAD_GET(r, 8) == 1) 永远为假，这个 bug 极其常见。 */
#define BAD_GET(reg, bit)    (reg & (1u << bit))

/* ---------- 虚拟寄存器 ---------- */
/* 真实 MCU 上这些地址会映射到外设，所以必须 volatile：
 * 否则编译器看到「写了又没人读」就可能把整条读-改-写优化掉。
 * 桌面上模拟时，volatile 也让它保持和真实寄存器一样的语义。 */
typedef struct {
    volatile uint32_t CR;   /* 控制寄存器：bit0 使能、bit31 软复位 */
    volatile uint32_t SR;   /* 状态寄存器：bit3 是错误标志 */
} sim_regs_t;

static int pass = 0, fail = 0;
static void ck(const char *name, long long got, long long want)
{
    if (got == want) { pass++; printf("  [PASS] %-42s = %lld\n", name, got); }
    else { fail++; printf("  [FAIL] %-42s 期望 %lld，实际 %lld\n", name, want, got); }
}

int main(void)
{
    printf("===== p02 寄存器宏封装 =====\n");

    sim_regs_t regs;
    memset(&regs, 0, sizeof regs);

    printf("-- bit0 与 bit31（本周验收项）--\n");
    REG_SET(regs.CR, 0);
    ck("SET bit0 → 1", regs.CR, 0x00000001);
    REG_SET(regs.CR, 31);
    ck("SET bit31 → 0x80000001（bit0 还在）", (long long)regs.CR, 0x80000001LL);
    ck("GET bit0  = 1", (long long)REG_GET(regs.CR, 0), 1);
    ck("GET bit31 = 1", (long long)REG_GET(regs.CR, 31), 1);
    ck("GET bit30 = 0", (long long)REG_GET(regs.CR, 30), 0);
    REG_CLR(regs.CR, 31);
    ck("CLR bit31 后只剩 bit0", regs.CR, 0x00000001);
    REG_CLR(regs.CR, 0);
    ck("CLR bit0 → 0", regs.CR, 0);

    printf("-- 翻转 --\n");
    REG_SET(regs.CR, 5);
    REG_TOGGLE(regs.CR, 5);
    ck("SET 后 TOGGLE 回到 0", regs.CR, 0);
    REG_TOGGLE(regs.CR, 31); REG_TOGGLE(regs.CR, 31);
    ck("bit31 连翻两次回到原值", regs.CR, 0);

    printf("-- 只动目标位，其余位原样 --\n");
    regs.CR = 0xFFFFFFFFu;
    REG_CLR(regs.CR, 7);
    ck("全 1 里清 bit7 → 0xFFFFFF7F", (long long)regs.CR, 0xFFFFFF7FLL);
    regs.CR = 0xA5A5A5A5u;
    REG_SET(regs.CR, 1);
    ck("0xA5A5A5A5 里置 bit1 → 0xA5A5A5A7", (long long)regs.CR, 0xA5A5A5A7LL);

    printf("-- 两个寄存器互不干扰 --\n");
    memset(&regs, 0, sizeof regs);
    REG_SET(regs.CR, 0);
    REG_SET(regs.SR, 3);
    ck("CR = 1", regs.CR, 1);
    ck("SR = 0x08（改 CR 不影响 SR）", regs.SR, 0x08);

    printf("-- 括号纪律 --\n");
    /* bit 传进来是 7 & 3 时：宏里带括号 → 1u << (7&3) = 1<<3 = 0x08
     *                         宏里不带括号 → 1 << 7 & 3 = (1<<7)&3 = 0
     * `&` 的优先级比 `<<` 低，整条表达式被拆成两半，这一位直接丢了。 */
    memset(&regs, 0, sizeof regs);
    REG_SET(regs.CR, 7 & 3);
    ck("REG_SET(_, 7 & 3) → 0x08（括号保住语义）", regs.CR, 0x08);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wparentheses"
    regs.CR = 0;
    BAD_SET(regs.CR, 7 & 3);
    ck("BAD_SET(_, 7 & 3) → 0（位被吃掉）", regs.CR, 0);
#pragma GCC diagnostic pop

    printf("-- 取位返回 0/1，不是掩码 --\n");
    regs.CR = 0x00000100u;
    ck("REG_GET(_, 8) = 1", (long long)REG_GET(regs.CR, 8), 1);
    ck("BAD_GET(_, 8) = 0x100（这是掩码）", (long long)BAD_GET(regs.CR, 8), 0x100LL);
    ck("if (BAD_GET(...) == 1) 永远不成立", (long long)(BAD_GET(regs.CR, 8) == 1u), 0);

    printf("-- 位号来自变量 --\n");
    memset(&regs, 0, sizeof regs);
    for (int i = 0; i < 32; i++) { REG_SET(regs.CR, i); }
    ck("循环置满 32 位 → 0xFFFFFFFF", (long long)regs.CR, 0xFFFFFFFFLL);
    for (int i = 0; i < 32; i++) { REG_CLR(regs.CR, i); }
    ck("循环清空 32 位 → 0", regs.CR, 0);

    int all_ok = 1;
    for (int i = 0; i < 32; i++)
    {
        REG_SET(regs.CR, i);
        if (REG_GET(regs.CR, i) != 1u) { all_ok = 0; }
        REG_CLR(regs.CR, i);
        if (REG_GET(regs.CR, i) != 0u) { all_ok = 0; }
    }
    ck("32 位逐位 SET→GET→CLR→GET 全对", all_ok, 1);

    printf("-- 一个像样的用法：使能 + 清错 + 软复位 --\n");
    memset(&regs, 0, sizeof regs);
    REG_SET(regs.CR, 0);            /* 使能 */
    REG_SET(regs.SR, 3);            /* 硬件把错误标志置起来 */
    if (REG_GET(regs.SR, 3))        /* 读状态位判断 */
    {
        REG_CLR(regs.SR, 3);        /* 写 1 清零（这里简化为直接清） */
    }
    ck("清错后 SR bit3 = 0", (long long)REG_GET(regs.SR, 3), 0);
    ck("CR 的使能位还在", (long long)REG_GET(regs.CR, 0), 1);

    printf("结果: %d 通过, %d 失败\n", pass, fail);
    return fail != 0;
}
