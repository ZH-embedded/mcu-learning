/* p02_swap.c —— 指针版 swap：为什么必须传地址
 *
 * 本题是"用指针改函数实参"的最小例子，也是面试第一道手写题。
 *
 * 核心认知：
 *   C 是值传递。函数拿到的是实参的一份拷贝。
 *   swap_by_value 改的只是拷贝，调用者的变量毫发无损。
 *   想改到调用者那边，就得把"住址"告诉函数 —— 也就是传指针。
 *
 * 建议先写完 swap_by_value，看它 FAIL，再写 swap_by_ptr 看它 PASS。
 * 那个 FAIL 比任何解释都有说服力。
 */
#include <stdio.h>

/* ============================================================
 * TODO 1：值传递版 —— 让它"尽量正确"，但注定改不到外面
 *
 * 就照最自然的写法写，别想办法绕过去。写对就行。
 * ============================================================ */
void swap_by_value(int a, int b)
{
    int temp = a;
    a = b;
    b = temp;
}

/* ============================================================
 * TODO 2：指针版 —— 真正能改到调用者
 *
 * 注意别把 * 丢在中间某一步。写完后自己讲一遍：
 *   参数 px 的类型是什么？*px 呢？&x 传进去变成了什么？
 * ============================================================ */
void swap_by_ptr(int *px, int *py)
{
    int temp = *px;
    *px = *py;
    *py = temp;
}

/* ============================================================
 * 以下不用改
 * ============================================================ */
static int g_pass = 0, g_fail = 0;

static void check(const char *name, int got, int want)
{
    if (got == want) {
        printf("  [PASS] %-40s = %d\n", name, got);
        g_pass++;
    } else {
        printf("  [FAIL] %-40s 期望 %d，实际 %d\n", name, want, got);
        g_fail++;
    }
}

int main(void)
{
    int x = 1, y = 2;

    printf("=== p02 指针版 swap ===\n\n");

    printf("初始：x=%d y=%d\n", x, y);

    swap_by_value(x, y);
    printf("swap_by_value 之后：x=%d y=%d\n", x, y);
    printf("↑ 这里必须是【没变】。如果它变了，说明你的「值传递版」偷偷用了指针，\n"
           "  那就回到题目本意了——请把它改回纯值传递。\n\n");

    swap_by_ptr(&x, &y);
    printf("swap_by_ptr   之后：x=%d y=%d\n\n", x, y);

    printf("--- 测试 ---\n");

    x = 1; y = 2;
    swap_by_value(x, y);
    check("值传递版：x 保持 1（证明改不到外面）", x, 1);
    check("值传递版：y 保持 2", y, 2);

    x = 1; y = 2;
    swap_by_ptr(&x, &y);
    check("指针版：x 变成 2", x, 2);
    check("指针版：y 变成 1", y, 1);

    x = -5; y = -5;
    swap_by_ptr(&x, &y);
    check("两数相同时 x 仍为 -5", x, -5);
    check("两数相同时 y 仍为 -5", y, -5);

    int arr[2] = {7, 9};
    swap_by_ptr(&arr[0], &arr[1]);
    check("交换数组元素 arr[0]", arr[0], 9);
    check("交换数组元素 arr[1]", arr[1], 7);

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
