/* p03_walk.c —— 用指针遍历数组：下标和指针是同一件事的两种写法
 *
 * 关键恒等式（一定要记牢，后面看 HAL 库源码全靠它）：
 *      arr[i]   ≡   *(arr + i)
 *
 * 所以下面三种写法编译出来的机器码通常完全一样：
 *      for (i = 0; i < n; i++)  sum += arr[i];
 *      for (i = 0; i < n; i++)  sum += *(arr + i);
 *      for (p = arr; p < arr + n; p++)  sum += *p;
 *
 * 但有一个陷阱：`arr` 是数组名，它不是变量，不能自增。
 *      arr++;        // ❌ 编译错误
 *      p = arr; p++; // ✅ 把首地址拷进指针变量再动
 *
 * 为什么嵌入式里要关心这个：
 *   指针遍历少一次乘法（下标要算 base + i*sizeof(T)），
 *   在 -O0 调试构建下（板子上单步调试时优化是关的）这个差别是真实的。
 */
#include <stdio.h>

/* ============================================================
 * TODO 1：用指针加法求数组元素之和
 *
 * 禁止 arr[i] 写法。用 *(arr + i) 或 *(p + i)。
 * n 是元素个数，n == 0 时返回 0。
 * ============================================================ */
int sum_by_ptr(const int *arr, int n)
{
    if (n == 0)
    {
        return 0;
    }

    int sum = 0;
    for (int i = 0; i < n; i++)
    {
        sum = sum + *(arr + i);
    }
    return sum;
}

/* ============================================================
 * TODO 2：用指针自增找最大值，返回【指向最大值元素的指针】
 *
 * 返回指针而不是返回下标，是 C 里常见的设计（能直接改到那个元素）。
 * 约定：n <= 0 时返回 NULL。
 * 多个相同最大值时，返回【第一个】。想清怎么保证。
 * ============================================================ */
int *max_ptr(int *arr, int n)
{
    if (n <= 0)
    {
        return NULL;
    }
    int* max_p = arr;
    for (int i = 1; i < n; i++)
    {
        if (*( arr + i ) > *max_p)
        {
            max_p = arr + i;
        }
    }
    return max_p;
}

/* ============================================================
 * TODO 3：用指针自增反转数组（原地，不借助第二个数组）
 *
 * 两头往中间走：left 从头、right 从尾，交换后同时往中间挪，
 * 直到 left >= right 停下。注意奇数个元素时中间那个不用动。
 * ============================================================ */
void reverse_inplace(int *arr, int n)
{
    if (arr == NULL || n <= 1)
    return;

    int* left = arr;
    int* right = arr + n - 1;

    while (left < right)
    {
        int temp = *left;
        *left = *right;
        *right = temp;
        left++;
        right--;
    }
    return;
}

/* ============================================================
 * TODO 4：二级指针改指针 —— 把 *pp 指向目标值
 *
 * 题目：在 arr 中找 value，找到就把调用者传进来的那个指针变量
 *       改指向它（也就是 *pp = 找到的地址）。
 * 返回 1 表示找到，0 表示没找到（没找到时不许改 *pp）。
 *
 * 为什么需要二级指针：
 *   我们要改的是"调用者的指针变量本身"，而改变量需要它的地址，
 *   指针变量的地址就是 int **。
 *   12 月写链表删除头节点时，第一次删头就靠这个模式。
 * ============================================================ */
int find_set(int *arr, int n, int value, int **pp)
{
    for (int i = 0; i < n; i++)
    {
        if (*( arr + i) == value)
        {
            *pp = arr + i;
            return 1;
        }
    }
    return 0;
}

/* ============================================================
 * 以下不用改
 * ============================================================ */
static int g_pass = 0, g_fail = 0;

static void check(const char *name, int got, int want)
{
    if (got == want)
    {
        printf("  [PASS] %-42s = %d\n", name, got);
        g_pass++;
    }
    else
    {
        printf("  [FAIL] %-42s 期望 %d，实际 %d\n", name, want, got);
        g_fail++;
    }
}

static void check_ptr(const char *name, const int *got, const int *want)
{
    if (got == want)
    {
        printf("  [PASS] %-42s 指向正确的元素\n", name);
        g_pass++;
    }
    else
    {
        printf("  [FAIL] %-42s 地址不符：%p vs %p\n",
               name, (const void *)got, (const void *)want);
        g_fail++;
    }
}

int main(void)
{
    int a[5] = {3, 1, 4, 1, 5};
    int b[4] = {10, 20, 30, 40};
    int c[1] = {99};

    printf("=== p03 指针遍历数组 ===\n\n");

    printf("--- 测试 ---\n");

    check("sum a[5] = 14", sum_by_ptr(a, 5), 14);
    check("sum b[4] = 100", sum_by_ptr(b, 4), 100);
    check("sum 空数组(n=0) = 0", sum_by_ptr(a, 0), 0);
    check("sum 单元素 = 99", sum_by_ptr(c, 1), 99);

    check_ptr("max_ptr a → a[4]=5", max_ptr(a, 5), &a[4]);
    check_ptr("max_ptr b → b[3]=40", max_ptr(b, 4), &b[3]);
    check_ptr("max_ptr 单元素 → c[0]", max_ptr(c, 1), &c[0]);
    check("max_ptr n=0 返回 NULL", max_ptr(a, 0) == NULL, 1);

    /* 多个相同最大值，必须返回第一个 */
    {
        int dup[4] = {9, 2, 9, 9};
        check_ptr("max_ptr 重复值 → 取第一个 (dup[0])", max_ptr(dup, 4), &dup[0]);
    }

    /* 原地反转 */
    reverse_inplace(a, 5);
    check("反转 5 元素 a[0]", a[0], 5);
    check("反转 5 元素 a[1]", a[1], 1);
    check("反转 5 元素 a[2] (中间不动)", a[2], 4);
    check("反转 5 元素 a[3]", a[3], 1);
    check("反转 5 元素 a[4]", a[4], 3);

    reverse_inplace(b, 4);
    check("反转偶数个 b[0]", b[0], 40);
    check("反转偶数个 b[3]", b[3], 10);

    reverse_inplace(c, 1);
    check("反转单元素 = 99", c[0], 99);

    /* 找到 / 没找到 */
    {
        int buf[4] = {5, 6, 7, 8};
        int *ptr = &buf[0];

        check("find_set 找到 7 → 返回 1", find_set(buf, 4, 7, &ptr), 1);
        check_ptr("find_set 后 ptr 指向 buf[2]", ptr, &buf[2]);

        ptr = &buf[0];
        check("find_set 找不到 100 → 返回 0", find_set(buf, 4, 100, &ptr), 0);
        check_ptr("找不到时 ptr 不许改（仍指向 buf[0]）", ptr, &buf[0]);
    }

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
