#include <stdio.h>
#include <stddef.h>
#include <string.h>

/* ============================================================
 * 陷阱 1：sizeof 是编译期的事，操作数不参与求值
 * 陷阱 2：数组作形参会退化成指针，sizeof 拿到的是指针大小
 * 陷阱 3：sizeof 的结果是 size_t（无符号），相减会下溢
 * ============================================================ */

/* TODO 1：写一个求数组元素个数的宏（只对真正的数组有效） */
/* ============================================================
 * TODO 1：写一个求数组元素个数的宏
 *
 *   正确形式是 sizeof(整个数组) / sizeof(一个元素)。
 *   三个细节：
 *     - 每个参数都要加括号，否则 ARRAY_LEN(a + 1) 会算错
 *     - 不能写成函数：数组传进函数就退化成指针了
 *     - 只对**真正的数组**有效，对指针会算出 2（64 位下 8/4）
 * ============================================================ */
#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))       /* TODO：写一个求数组元素个数的宏 */

/* 反面教材，已给出：形参 arr 已经退化成指针，sizeof 拿到 8 不是 40。
 * 注意 gcc 会报 -Wsizeof-array-argument —— 编译器其实在提醒你写错了，
 * 这里显式关掉，因为这段是故意留的错误示范。 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsizeof-array-argument"
int bad_param_len(int arr[10])
{
    return (int)(sizeof(arr) / sizeof(arr[0]));
}
#pragma GCC diagnostic pop

/* TODO 2：在函数里写 sizeof(i++)，然后返回 i。
 *         用来证明 sizeof 的操作数不会真的执行。 */
/* ============================================================
 * TODO 2：证明 sizeof 的操作数不会真的执行
 *
 *   - 声明 int i = 0;
 *   - 写一句 size_t s = sizeof(i++);（用 (void)s; 消掉未使用警告）
 *   - 返回 i
 *
 * 返回 0 就说明 i++ 没执行。sizeof 在**编译期**就把答案算完了，
 * 它只需要操作数的类型，不需要值。
 * 顺带解释一个现象：sizeof(arr[i++]) 也不会越界，因为根本没访问 arr。
 * ============================================================ */
size_t sizeof_no_eval(void)
{
    int i = 0;
    size_t s = sizeof(i++);
    (void)s;
    return (size_t)i;
}

/* TODO 3：比较两个 size_t，返回 -1 / 0 / 1。
 *         不许写 `return (int)(a - b)` —— 相减会下溢。 */
/* ============================================================
 * TODO 3：比较两个 size_t，返回 -1 / 0 / 1
 *
 *   - **不许**写 return (int)(a - b)：a < b 时减法在无符号里下溢，
 *     得到一个巨大的正数，转成 int 还是错的
 *   - 正确写法一行：(a > b) - (a < b)
 *     两个比较各产生 0 或 1，相减正好是 -1/0/1
 * ============================================================ */
int compare_size(size_t a, size_t b)
{
    return (a > b) - (a < b);
}

/* TODO 4：返回形参 s 的大小（不是它指向的字符串长度） */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsizeof-array-argument"
/* ============================================================
 * TODO 4：返回形参 s 的大小
 *
 *   - 直接 return sizeof(s);
 *   - 返回的是 8（指针大小），不是 16。形参写成 char s[16] 也没用，
 *     编译器直接把它当成 char *，16 这个数字根本没被记住
 *   - 外面包了 pragma 关掉 -Wsizeof-array-argument，否则 gcc 会警告你，
 *     这个警告正是本题的答案
 * ============================================================ */
size_t param_ptr_size(const char s[16])
{
    
    return sizeof(s);
}
#pragma GCC diagnostic pop

/* 辅助：演示 size_t 下溢 */
int underflow_is_positive(void)
{
    size_t zero = 0;
    return (zero - 1) > 0;
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
    printf("===== p04 sizeof 的三个经典陷阱 =====\n");

    int  a10[10];
    char buf[32];
    memset(buf, 0, sizeof buf);      /* 先初始化，避免"未使用变量"警告 */
    ck("ARRAY_LEN(int[10])", (long long)ARRAY_LEN(a10), 10);
    ck("ARRAY_LEN(char[32])", (long long)ARRAY_LEN(buf), 32);

    ck("sizeof(int[10]) 是 40（元素个数 × 4）", (long long)sizeof(a10), 40);
    ck("sizeof(char[16]) 是 16", (long long)sizeof(char[16]), 16);

    printf("-- 陷阱 2：数组形参退化 --\n");
    ck("形参里的 sizeof(arr)/sizeof(arr[0]) = 2（指针/4，不是 10）",
       bad_param_len(a10), 2);
    ck("参数写成 const char s[16]，sizeof(s) 是 8",
       (long long)param_ptr_size("0123456789abcdef"), 8);

    printf("-- 陷阱 1：sizeof 不求值 --\n");
    ck("sizeof(i++) 之后 i 仍是 0", (long long)sizeof_no_eval(), 0);

    printf("-- 陷阱 3：size_t 无符号 --\n");
    ck("compare_size(1, 2)", compare_size(1, 2), -1);
    ck("compare_size(2, 2)", compare_size(2, 2), 0);
    ck("compare_size(3, 2)", compare_size(3, 2), 1);
    ck("0 - 1 在 size_t 里是个巨大的正数", underflow_is_positive(), 1);
    ck("compare_size(0, 1) 不受下溢影响", compare_size(0, 1), -1);

    printf("结果: %d 通过, %d 失败\n", pass, fail);
    return fail != 0;
}
