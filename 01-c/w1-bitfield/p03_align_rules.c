#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* 四个用来手算的结构体 */
typedef struct { char a; int  b; char c; } s1_t;
typedef struct { int  b; char a; char c; } s2_t;
typedef struct { char a; double d; char c; } s3_t;
typedef struct { char a[3]; short s; int i; } s4_t;

/* ---------- 通用对齐工具 ---------- */

/* 判断 n 是不是 2 的幂（0 不是） */
/* ============================================================
 * TODO 1：判断 n 是不是 2 的幂（0 不是）
 *
 *   - 经典写法：n != 0 且 (n & (n - 1)) == 0
 *   - 想一想为什么成立：2 的幂只有一位是 1，减一之后那一位变 0、
 *     低位全变 1，两个数与起来必然是 0
 * ============================================================ */
int is_power_of_two(size_t n)
{
    (void)n;
    /* TODO：见上方注释 */
    return 0;
}

/* 把 n 向上取整到 align 的整数倍；align 必须是 2 的幂 */
/* ============================================================
 * TODO 2：把 n 向上取整到 align 的整数倍
 *
 *   - align 为 0 时直接返回 n（防除零 / 防取模炸弹）
 *   - 经典写法： (n + align - 1) & ~(align - 1)
 *     只有 align 是 2 的幂时这个式子才成立
 *   - 用取模的写法 `n + (align - n % align) % align` 也对，
 *     但在 MCU 上除法比位运算贵几十倍
 *
 * align_up(12, 8) 应该是多少？先算再写。
 * ============================================================ */
size_t align_up(size_t n, size_t align)
{
    (void)n; (void)align;
    /* TODO：见上方注释 */
    return 0;
}

/* 填充字节数 = sizeof - 各成员大小之和 */
/* ============================================================
 * TODO 3：填充字节数 = sizeof - 各成员大小之和
 *
 *   - size < members_sum 时返回 0（不能返回负数，size_t 没有负数）
 *   - 这个函数就是"填充浪费了多少"的直接度量
 * ============================================================ */
size_t padding_bytes(size_t size, size_t members_sum)
{
    (void)size; (void)members_sum;
    /* TODO：见上方注释 */
    return 0;
}

/* 指针是否按 align 对齐 */
/* ============================================================
 * TODO 4：p 是否按 align 对齐
 *
 *   - p 为 NULL 或 align 为 0 → 返回 0
 *   - 地址要转成整数才能取模：((uintptr_t)p % align) == 0
 *   - 需要 #include <stdint.h>，文件顶部已经有了
 * ============================================================ */
int is_aligned(const void *p, size_t align)
{
    (void)p; (void)align;
    /* TODO：见上方注释 */
    return 0;
}

/* ---------- 测试 ---------- */
static int pass = 0, fail = 0;
static void ck(const char *name, long long got, long long want)
{
    if (got == want) { pass++; printf("  [PASS] %-50s = %lld\n", name, got); }
    else { fail++; printf("  [FAIL] %-50s 期望 %lld，实际 %lld\n", name, want, got); }
}

int main(void)
{
    printf("===== p03 对齐规则：先手算，再上机 =====\n");

    printf("-- 手算结果 vs 实测 --\n");
    ck("s1 {char; int; char;}  sizeof=12（补 6）", (long long)sizeof(s1_t), 12);
    ck("  s1 的 b 偏移=4（a 后面补 3）", (long long)offsetof(s1_t, b), 4);
    ck("  s1 的 c 偏移=8", (long long)offsetof(s1_t, c), 8);

    ck("s2 {int; char; char;}  sizeof=8（只补尾部 2）", (long long)sizeof(s2_t), 8);
    ck("  s2 的 a 偏移=4", (long long)offsetof(s2_t, a), 4);
    ck("  s2 的 c 偏移=5", (long long)offsetof(s2_t, c), 5);

    ck("s3 {char; double; char;} sizeof=24（补 14）", (long long)sizeof(s3_t), 24);
    ck("  s3 的 d 偏移=8（double 要 8 对齐）", (long long)offsetof(s3_t, d), 8);
    ck("  s3 的 c 偏移=16", (long long)offsetof(s3_t, c), 16);

    ck("s4 {char[3]; short; int;} sizeof=12（补 3）", (long long)sizeof(s4_t), 12);
    ck("  s4 的 s 偏移=4（short 要 2 对齐）", (long long)offsetof(s4_t, s), 4);
    ck("  s4 的 i 偏移=8（int 要 4 对齐）", (long long)offsetof(s4_t, i), 8);

    ck("数组 s1_t[2] 占 24（每个元素都补齐，不是只补一次）",
       (long long)sizeof(s1_t[2]), 24);

    printf("-- 填充字节数 --\n");
    ck("s1 填充 = 12 - (1+4+1)", (long long)padding_bytes(sizeof(s1_t), 1+4+1), 6);
    ck("s2 填充 = 8 - (4+1+1)",  (long long)padding_bytes(sizeof(s2_t), 4+1+1), 2);
    ck("s3 填充 = 24 - (1+8+1)", (long long)padding_bytes(sizeof(s3_t), 1+8+1), 14);
    ck("s4 填充 = 12 - (3+2+4)", (long long)padding_bytes(sizeof(s4_t), 3+2+4), 3);
    ck("成员和大于 size 时返回 0（不返回负数）",
       (long long)padding_bytes(4, 8), 0);

    printf("-- align_up --\n");
    ck("align_up(5, 4)",  (long long)align_up(5, 4), 8);
    ck("align_up(4, 4)",  (long long)align_up(4, 4), 4);
    ck("align_up(1, 8)",  (long long)align_up(1, 8), 8);
    ck("align_up(0, 8)",  (long long)align_up(0, 8), 0);
    ck("align_up(12, 8)", (long long)align_up(12, 8), 16);
    ck("align_up(13, 1)", (long long)align_up(13, 1), 13);

    printf("-- is_power_of_two --\n");
    ck("is_power_of_two(0)",  is_power_of_two(0), 0);
    ck("is_power_of_two(1)",  is_power_of_two(1), 1);
    ck("is_power_of_two(3)",  is_power_of_two(3), 0);
    ck("is_power_of_two(64)", is_power_of_two(64), 1);

    printf("-- is_aligned --\n");
    ck("is_aligned(s1, 4)", (long long)is_aligned((const void *)&(s1_t){0}, 4), 1);
    ck("is_aligned(NULL, 4) 返回 0", (long long)is_aligned(NULL, 4), 0);
    ck("is_aligned(p, 0) 返回 0（防除零）", (long long)is_aligned((const void *)"x", 0), 0);

    printf("结果: %d 通过, %d 失败\n", pass, fail);
    return fail != 0;
}
