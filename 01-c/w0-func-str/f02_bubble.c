/* f02_bubble.c —— 冒泡排序
 *
 * 题目：把 arr 升序排序，返回「交换次数」
 *
 * 要求：
 *   用冒泡（相邻两两比较、大的往后冒）
 *   加一个优化：某一轮从头到尾没发生交换 → 说明已经有序，立刻退出
 *   （不加这个优化，已排序的数组也要白跑 n² 次比较）
 *
 * 关于 g_compare_count：
 *   每次比较两个元素之前 ++ 一次。它不参与 PASS/FAIL，
 *   但最后会打印出来，用来判断你的提前退出优化有没有真的生效。
 */
#include <stdio.h>

int g_compare_count = 0;   /* 在实现里每次比较前 ++ */

/* ============================================================
 * TODO: 实现冒泡排序，返回交换次数
 * ============================================================ */
int bubble_sort(int arr[], int n)
{
    int i = 0;
    int count = 0;//交换次数
    while (i < n - 1)
    {
        int j = 0;
        int swapped = 0;
        while (j < n - 1 - i)
        {
            g_compare_count++;
            if(arr[j] > arr[j + 1])
            {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
                count++;
                swapped = 1;                
            }           
            j++;
        }
        if (swapped == 0)
        {
            return count;
        }
        
        i++;
    }
    
    return count;      
}

/* ============================================================
 * 以下不用改
 * ============================================================ */
static int g_pass = 0, g_fail = 0;

static void check_int(const char *name, int expect, int actual)
{
    if (expect == actual) {
        printf("  [PASS] %-30s = %d\n", name, actual);
        g_pass++;
    } else {
        printf("  [FAIL] %-30s 期望 %d，实际 %d\n", name, expect, actual);
        g_fail++;
    }
}

static void print_arr(const int a[], int n)
{
    printf("[");
    for (int i = 0; i < n; i++) printf(i ? " %d" : "%d", a[i]);
    printf("]");
}

static void run(const char *title, const int input[], int n,
                const int expect[], int expect_swaps)
{
    int buf[16];
    for (int i = 0; i < n; i++) buf[i] = input[i];

    printf("\n%s\n  输入 ", title);
    print_arr(buf, n);

    g_compare_count = 0;
    int swaps = bubble_sort(buf, n);

    printf("\n  输出 ");
    print_arr(buf, n);
    printf("   比较 %d 次\n", g_compare_count);

    int same = 1;
    for (int i = 0; i < n; i++) if (buf[i] != expect[i]) same = 0;

    if (same) {
        printf("  [PASS] 排序结果正确\n");
        g_pass++;
    } else {
        printf("  [FAIL] 排序结果不对，期望 ");
        print_arr(expect, n);
        printf("\n");
        g_fail++;
    }
    check_int("交换次数", expect_swaps, swaps);
}

int main(void)
{
    printf("=== 冒泡排序测试 ===\n");

    const int c1[] = { 5, 2, 9, 1, 5, 6 };
    const int e1[] = { 1, 2, 5, 5, 6, 9 };
    run("乱序 + 含重复", c1, 6, e1, 6);

    const int c2[] = { 1, 2, 3, 4, 5 };
    const int e2[] = { 1, 2, 3, 4, 5 };
    run("已排序（优化后应只比 4 次）", c2, 5, e2, 0);

    const int c3[] = { 5, 4, 3, 2, 1 };
    const int e3[] = { 1, 2, 3, 4, 5 };
    run("完全逆序（最坏情况）", c3, 5, e3, 10);

    const int c4[] = { 7 };
    const int e4[] = { 7 };
    run("单元素", c4, 1, e4, 0);

    const int c5[] = { 3, 3, 3 };
    const int e5[] = { 3, 3, 3 };
    run("全相同（优化后应只比 2 次）", c5, 3, e5, 0);

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);
    printf("提示：已排序 5 个元素，如果比较次数是 4 说明提前退出生效；是 10 说明没生效\n");
    return g_fail == 0 ? 0 : 1;
}
