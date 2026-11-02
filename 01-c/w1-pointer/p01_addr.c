/* p01_addr.c —— 地址、解引用、指针变量本身占多大
 *
 * 今天的第一件事：把"指针"这个词拆开。
 *   指针是一个变量，它的值是一个地址。
 *   &x   取地址     —— "x 住在哪"
 *   *p   解引用     —— "p 指向的那个房间里放着什么"
 *
 * 先读一遍 main 里的打印，再填两个 TODO。
 */
#include <stdio.h>
#include <stddef.h>

/* ============================================================
 * TODO 1：返回数组首元素的地址
 *
 * 提示：数组名 arr 在表达式里就等价于 &arr[0]，所以两者是同一个值。
 *      你写 return &arr[0]; 或 return arr; 都对——但要知道为什么等价。
 * ============================================================ */
int *first_elem(int *arr)
{

    return arr;
}

/* ============================================================
 * TODO 2：把 *p 改成 new_val，并返回被覆盖掉的旧值
 *
 * 提示：先把旧值存进一个局部变量，再写新值，最后 return 那个局部变量。
 *      顺序错了（先写后读）就永远返回 new_val。
 * ============================================================ */
int replace(int *p, int new_val)
{
    int val = *p;
    *p = new_val;
    return val;
}

/* ============================================================
 * 以下不用改
 * ============================================================ */
static int g_pass = 0, g_fail = 0;

static void check_int(const char *name, int got, int want)
{
    if (got == want) {
        printf("  [PASS] %-32s = %d\n", name, got);
        g_pass++;
    } else {
        printf("  [FAIL] %-32s 期望 %d，实际 %d\n", name, want, got);
        g_fail++;
    }
}

static void check_same(const char *name, const int *got, const int *want)
{
    if (got == want) {
        printf("  [PASS] %-32s 指向同一处\n", name);
        g_pass++;
    } else {
        printf("  [FAIL] %-32s 地址不同：%p vs %p\n",
               name, (const void *)got, (const void *)want);
        g_fail++;
    }
}

int main(void)
{
    int  a = 42;
    int  arr[4] = {10, 20, 30, 40};
    int *p;

    printf("=== p01 地址与解引用 ===\n\n");

    /* 先看懂这几行输出，再往下做题 */
    printf("a 的值        = %d\n", a);
    printf("&a 的地址     = %p\n", (void *)&a);
    printf("arr 的值      = %p   ← 和 &arr[0] 是同一个地址\n", (void *)arr);
    printf("&arr[0]       = %p\n", (void *)&arr[0]);
    printf("sizeof(int)   = %zu 字节\n", sizeof(int));
    printf("sizeof(int *) = %zu 字节   ← 指针要装下一个地址，64 位机器上就是 8\n\n",
           sizeof(int *));

    printf("--- 测试 ---\n");

    p = first_elem(arr);
    check_same("first_elem 返回 arr[0] 的地址", p, &arr[0]);
    check_int("first_elem 解引用得 10", p ? *p : -1, 10);

    check_int("replace 返回旧值 42", replace(&a, 100), 42);
    check_int("a 已被改成 100", a, 100);

    check_int("replace 返回 arr[3] 旧值 40", replace(&arr[3], 7), 40);
    check_int("arr[3] 已是 7", arr[3], 7);

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
