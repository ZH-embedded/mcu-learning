/* p01_funcptr.c —— 函数指针：声明、赋值、调用
 *
 * 函数也有地址。函数名在表达式里就是函数的入口地址，
 * 所以可以把"哪个函数"当成一个变量存起来，等到运行时再决定调谁。
 *
 * 三种写法，编译出来完全一样：
 *     f(3, 4)
 *   (*f)(3, 4)
 *   (**f)(3, 4)        ← 也行，但别这么写
 *
 * 真正要记的是声明怎么读：
 *     int (*f)(int, int);   f 是指针，指向"收两个 int、返回 int"的函数
 *     int *f(int, int);     f 是函数，收两个 int、返回 int*
 * 差别只在括号。没有括号，星号先跟着返回类型走。
 */
#include <stdio.h>
#include <stddef.h>

/* 三个被指向的普通函数，不用改 */
static int add(int a, int b) { return a + b; }
static int sub(int a, int b) { return a - b; }
static int mul(int a, int b) { return a * b; }

/* ============================================================
 * 函数指针类型（给你了，但要知道每一段在说什么）
 *
 *   typedef int (*binop_t)(int, int);
 *           └┬┘ └──┬──┘└──┬──┘ └───┬───┘
 *            │    │      │        └─ 参数：两个 int
 *            │    │      └─ 名字（typedef 定义的"类型名"）
 *            │    └─ 星号被括号裹住：说明这个名字是"指针"
 *            └─ 指向的函数返回 int
 *
 * 换成日常说法：binop_t 就是"收两个 int 返回一个 int 的函数的指针"。
 * 有了它，下面所有声明都不用再写那串括号了 —— 这就是 typedef 的价值。
 * ============================================================ */
typedef int (*binop_t)(int, int);

/* ============================================================
 * TODO 1：通过函数指针调用
 *
 * 要求：f 为 NULL 时返回 0（调用空函数指针会直接崩，这是嵌入式里最常见的死法之一）
 *       否则返回 f(a, b)
 *
 * 注意：不要写成 f(a,b) 之外的花样，就一行 return。
 * ============================================================ */
int apply_binop(binop_t f, int a, int b)
{
    if (f == NULL)
    {
        return 0;
    }

    return f(a, b);
}

/* ============================================================
 * TODO 2：按符号返回对应的函数指针
 *
 *   '+' → add    '-' → sub    '*' → mul    其它 → NULL
 *
 * 提示：函数名在 return 语句里就自动变成函数指针，不用写 &add。
 *      （写了也对，&add 和 add 是同一个值，就像数组名一样）
 * ============================================================ */
binop_t select_op(char sym)
{
    switch (sym)
    {
    case '+':
        return add;

    case '-':
        return sub;

    case '*':
        return mul;
    default:
        return NULL;
    }
}

/* ============================================================
 * TODO 3：拿一个函数指针，把 n 对数据全部算一遍
 *
 * 要求：a[] 和 b[] 逐对喂给 f，结果写进 out[]；
 *       返回"成功算过几对"，f 为 NULL 或 n <= 0 时返回 0。
 *
 * 这一题的意义：函数体里**从头到尾不出现任何具体运算**。
 * 想知道它算加法还是乘法，只能看调用者传了谁 —— 这就是"把行为当参数传"。
 * 12 月写驱动时 HAL 里的 xxx_Callback 全是这个结构。
 * ============================================================ */
int apply_all(binop_t f, const int *a, const int *b, int n, int *out)
{
    if ((f == NULL) || (a == NULL) || (b == NULL) || (n <= 0) || (out == NULL))
    {
        return 0;
    }

    for (int i = 0; i < n; i++)
    {
        *(out + i) = f(a[i], b[i]);
    }
    return n;
}

/* ============================================================
 * 以下不用改
 * ============================================================ */
static int g_pass = 0, g_fail = 0;

static void check_int(const char *name, int got, int want)
{
    if (got == want)
    {
        printf("  [PASS] %-34s = %d\n", name, got);
        g_pass++;
    }
    else
    {
        printf("  [FAIL] %-34s 期望 %d，实际 %d\n", name, want, got);
        g_fail++;
    }
}

/* 函数指针可以直接用 == 比较：比的是入口地址 */
static void check_fn(const char *name, binop_t got, binop_t want)
{
    if (got == want)
    {
        printf("  [PASS] %-34s 指向同一函数\n", name);
        g_pass++;
    }
    else
    {
        printf("  [FAIL] %-34s 指向的不是同一个函数\n", name);
        g_fail++;
    }
}

static void check_arr(const char *name, const int *got, const int *want, int n)
{
    for (int i = 0; i < n; i++)
    {
        if (got[i] != want[i])
        {
            printf("  [FAIL] %-34s 第 %d 个是 %d，期望 %d\n",
                   name, i, got[i], want[i]);
            g_fail++;
            return;
        }
    }
    printf("  [PASS] %-34s 前 %d 个全对\n", name, n);
    g_pass++;
}

int main(void)
{
    int a[4] = {3, -2, 7, 0};
    int b[4] = {4, 5, 3, 9};
    int out[4] = {0, 0, 0, 0};
    int want[4];

    printf("=== p01 函数指针 ===\n\n");

    /* 先看懂这几行，再做题 */
    binop_t op = add;
    printf("op == add 吗？   %s   ← 函数名本身就是入口地址\n",
           op == add ? "是" : "否");
    printf("op(3,4)         = %d\n", op(3, 4));
    printf("(*op)(3,4)      = %d   ← 和上面一样，两种写法等价\n", (*op)(3, 4));
    printf("sizeof(binop_t) = %zu 字节   ← 函数指针也是指针\n",
           sizeof(binop_t));
    /* 注意：sizeof(函数名) 在 C 里是未定义的，别拿它当"函数有多大" */

    printf("--- 测试 ---\n");

    check_int("apply_binop(add, 3, 4)", apply_binop(add, 3, 4), 7);
    check_int("apply_binop(sub, 3, 4)", apply_binop(sub, 3, 4), -1);
    check_int("apply_binop(mul, 3, 4)", apply_binop(mul, 3, 4), 12);
    check_int("apply_binop(NULL) 返回 0", apply_binop(NULL, 3, 4), 0);

    check_fn("select_op('+') 是 add", select_op('+'), add);
    check_fn("select_op('-') 是 sub", select_op('-'), sub);
    check_fn("select_op('*') 是 mul", select_op('*'), mul);
    check_fn("select_op('?') 是 NULL", select_op('?'), NULL);

    /* 函数指针数组：4 个函数指针排成一张表，按下标取 */
    binop_t table[3] = {select_op('+'), select_op('-'), select_op('*')};
    check_int("table[0](6,2) 加法", table[0] ? table[0](6, 2) : -999, 8);
    check_int("table[1](6,2) 减法", table[1] ? table[1](6, 2) : -999, 4);
    check_int("table[2](6,2) 乘法", table[2] ? table[2](6, 2) : -999, 12);

    want[0] = 12;
    want[1] = -10;
    want[2] = 21;
    want[3] = 0;
    check_int("apply_all(mul) 返回 4 对", apply_all(mul, a, b, 4, out), 4);
    check_arr("apply_all(mul) 结果", out, want, 4);

    want[0] = 7;
    want[1] = 3;
    want[2] = 10;
    want[3] = 9;
    check_int("apply_all(add) 返回 4 对", apply_all(add, a, b, 4, out), 4);
    check_arr("apply_all(add) 结果", out, want, 4);

    check_int("apply_all(NULL) 返回 0", apply_all(NULL, a, b, 4, out), 0);
    check_int("apply_all n=0 返回 0", apply_all(add, a, b, 0, out), 0);
    check_int("apply_all a=NULL 返回 0", apply_all(add, NULL, b, 4, out), 0);

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
