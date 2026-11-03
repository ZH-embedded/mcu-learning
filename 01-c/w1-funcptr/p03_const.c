/* p03_const.c —— const 与指针的三种组合
 *
 * const 的作用只有一个：**让编译器替你挡住不该写的写操作**。
 * 它在运行时不存在，不占任何内存，编译出来和你没写 const 一模一样。
 *
 * 三种组合，读法是"从右往左，先看 const 贴着谁"：
 *
 *   ①  const int *p        p 指向的内容只读，p 自己能改指向
 *        int const *p      完全等价（const 在 int 左边还是右边都一样）
 *
 *   ②  int * const p       p 自己只读（必须初始化，不能改指向），
 *                          但它指向的内容能改
 *
 *   ③  const int * const p 两个都只读
 *
 * 记法：const 在星号**左边** → 锁内容；const 在星号**右边** → 锁指针。
 *       （更准确的说法：const 修饰它左边的东西；左边没东西就修饰右边第一个）
 *
 * 今天只要记住一条实用结论：
 *   函数的指针形参，**只要不打算改它指向的内容，就加 const**。
 *   这样调用者敢把 const 数组传给你，编译器也能帮你抓出误写。
 */
#include <stdio.h>
#include <stddef.h>

/* const 纯粹是编译期的：下面两行在作用域外就能验证
 * （编译通过即证明 const 指针和普通指针一样大） */
_Static_assert(sizeof(const int *) == sizeof(int *),
               "const 不改变指针的大小");
_Static_assert(sizeof(int *const) == sizeof(int *),
               "const 修饰指针本身也不改变大小");

/* ============================================================
 * TODO 1：只读求和
 *
 * 形参是 const int *arr —— 你只能读 arr[i]，写 arr[i] = 0 会编译报错。
 * 要求：arr 为 NULL 或 n <= 0 返回 0。
 * ============================================================ */
int sum_readonly(const int *arr, int n)
{
    if (arr == NULL||n <= 0)
    {
        return 0;
    }
    int sum = 0;
    for (int i = 0; i < n; i++)
    {
        sum = sum + arr[i];
    }

    return sum;
}

/* ============================================================
 * TODO 2：改内容，但保证不改指向
 *
 * 形参是 int *const p —— p 本身锁死了（写 p = &other 编译报错），
 * 但 *p 能改。
 * 要求：p 为 NULL 时直接 return；否则 *p 乘以 k。
 * ============================================================ */
void scale_writer(int *const p, int k)
{
    if (p == NULL)
    {
        return;
    }

    *p = (*p) * k;
}

/* ============================================================
 * TODO 3：两个都只读，只取值
 *
 * 形参是 const int *const p —— 内容和指针都锁死，只能读。
 * 要求：p 为 NULL 返回 -1；否则返回 *p。
 *
 * 这种写法在真实代码里少见（形参上加 const 指针意义不大），
 * 但你要能一眼读出来：这是"纯读，谁都别想动"。
 * ============================================================ */
int read_both(const int *const p)
{
    if (p == NULL)
    {
        return -1;
    }
    return *p;

}

/* ============================================================
 * TODO 4：手写一个 const char * 的 strlen
 *
 * 字符串字面量 "hello" 放在只读段，所以所有字符串形参都该是 const char *。
 * 你自己写个 char *s = "hello"; s[0] = 'H'; —— 编译能过，运行时崩。
 * 加上 const 就是把这类事故提前到编译期。
 *
 * 要求：s 为 NULL 返回 0；否则数到 '\0' 之前的字符个数。
 * 不许用 strlen，也不许用 string.h。
 * ============================================================ */
int my_strlen_const(const char *s)
{
    if (s == NULL)
    {
        return 0;
    }

    int len = 0;
    while (s[len] != '\0')
    {
        len++;
    }
    return len;
}

/* ============================================================
 * 以下不用改
 * ============================================================ */
static int g_pass = 0, g_fail = 0;

static void check_int(const char *name, int got, int want)
{
    if (got == want) {
        printf("  [PASS] %-36s = %d\n", name, got);
        g_pass++;
    } else {
        printf("  [FAIL] %-36s 期望 %d，实际 %d\n", name, want, got);
        g_fail++;
    }
}

#if 0
/* ============================================================
 * 想看编译器怎么挡你，把上面的 #if 0 改成 #if 1，然后 `./build_run.sh -c`
 * 会报 **4 个 error**。看完记得改回 0。
 *
 * 每一行后面标的是编译器会说的话（大意）。
 *
 * ⚠ 最后一行 `s[0] = 'H'` 是**唯一不报错**的 —— 它要到运行时段错误。
 *   编译器能挡住的错都不可怕，可怕的是这种"能编译、跑起来才炸"的。
 * ============================================================ */
void try_illegal(void)
{
    int x = 1, y = 2;
    const int *cp = &x;

    *cp = 10;          /* 编译报错：内容只读（const 在星号左边） */
    cp = &y;           /* OK：指针本身能改指向 */

    int *const pc = &x;
    *pc = 20;          /* OK：内容能改 */
    pc = &y;           /* 编译报错：指针只读（const 在星号右边，且必须初始化） */

    const int *const cpc = &x;
    *cpc = 30;         /* 编译报错：内容只读 */
    cpc = &y;          /* 编译报错：指针只读 */

    char *s = "hello";
    s[0] = 'H';        /* 编译能过，运行时段错误：字面量在只读段 ← 最危险的一行 */
}
#endif

int main(void)
{
    printf("=== p03 const 与指针 ===\n\n");

    int arr[5] = {1, 2, 3, 4, 5};
    const int carr[3] = {10, 20, 30};

    printf("sizeof(int *)        = %zu\n", sizeof(int *));
    printf("sizeof(const int *)  = %zu   ← 一样大，const 不占地方\n\n",
           sizeof(const int *));

    printf("--- 测试 ---\n");

    check_int("sum_readonly 求和 = 15", sum_readonly(arr, 5), 15);
    check_int("sum_readonly 传 const 数组也行", sum_readonly(carr, 3), 60);
    check_int("sum_readonly n=0 返回 0", sum_readonly(arr, 0), 0);
    check_int("sum_readonly NULL 返回 0", sum_readonly(NULL, 5), 0);

    int v = 7;
    scale_writer(&v, 3);
    check_int("scale_writer(&v, 3) 后 v", v, 21);
    scale_writer(&v, 2);
    check_int("再来一次 (21*2)", v, 42);
    scale_writer(NULL, 3);
    printf("  [PASS] %-36s 不崩\n", "scale_writer(NULL)");
    g_pass++;

    int w = 99;
    check_int("read_both(&w) 读到 99", read_both(&w), 99);
    check_int("read_both(NULL) 返回 -1", read_both(NULL), -1);

    check_int("my_strlen_const(\"hello\")", my_strlen_const("hello"), 5);
    check_int("my_strlen_const(\"\")", my_strlen_const(""), 0);
    check_int("my_strlen_const(\"嵌入式\")",
              my_strlen_const("嵌入式"), 9);   /* UTF-8 一个汉字 3 字节 */
    check_int("my_strlen_const(NULL)", my_strlen_const(NULL), 0);

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
