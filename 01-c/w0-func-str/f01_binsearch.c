/* f01_binsearch.c —— 二分查找
 *
 * 题目：在升序数组 arr 中查找 target
 *   找到 → 返回下标
 *   找不到 → 返回 -1
 *
 * 约束：
 *   不许从头扫到尾（那次是 O(n)，白学）
 *   必须是每次排除一半，O(log n)
 *
 * 两个必须想清楚的点：
 *   1) loop 条件是 left <= right 还是 left < right ？
 *      取决于你把区间定义成 [left, right] 还是 [left, right)
 *   2) mid 怎么算不会溢出？（left + right 在 int 上会炸）
 */
#include <stdio.h>

/* ============================================================
 * TODO: 在这里实现你要求的函数
 * ============================================================ */
int bin_search(const int arr[], int n, int target)
{
    int left = 0;
    int right = n;
    while (left < right)
        {
            int i = (left + right)/2;
            if (arr[i] == target)
            {
                return i;
            }
            
            if(arr[i] < target)
            {
                left = i + 1;
            }
            if(arr[i] > target)
            {
                right = i;
            }
        }   
    return -1;
}

/* ============================================================
 * 以下不用改（想改也行，保持 PASS/FAIL 输出格式即可）
 * ============================================================ */
static int g_pass = 0, g_fail = 0;

static void check_int(const char *name, int expect, int actual)
{
    if (expect == actual) {
        printf("  [PASS] %-28s = %d\n", name, actual);
        g_pass++;
    } else {
        printf("  [FAIL] %-28s 期望 %d，实际 %d\n", name, expect, actual);
        g_fail++;
    }
}

int main(void)
{
    const int a[] = { 1, 3, 5, 7, 9, 11, 13, 15, 17, 19 };   /* 10 个元素 */
    const int n = (int)(sizeof(a) / sizeof(a[0]));

    printf("数组: ");
    for (int i = 0; i < n; i++) printf("%d ", a[i]);
    printf("  (n=%d)\n\n", n);

    /* 命中：首、尾、中间、任意 */
    check_int("查 1  (第一个)",        0,  bin_search(a, n, 1));
    check_int("查 19 (最后一个)",      9,  bin_search(a, n, 19));
    check_int("查 7  (中间偏左)",      3,  bin_search(a, n, 7));
    check_int("查 11 (中间偏右)",      5,  bin_search(a, n, 11));

    /* 未命中：落在中间空隙、比最小还小、比最大还大 */
    check_int("查 8  (夹在 7 和 9 之间)", -1, bin_search(a, n, 8));
    check_int("查 0  (比最小还小)",     -1, bin_search(a, n, 0));
    check_int("查 21 (比最大还大)",     -1, bin_search(a, n, 21));

    /* 边界：空数组 */
    check_int("空数组 n=0",             -1, bin_search(a, 0, 1));

    /* 边界：单元素 */
    const int one[] = { 42 };
    check_int("单元素 查 42",           0,  bin_search(one, 1, 42));
    check_int("单元素 查 41",          -1,  bin_search(one, 1, 41));

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
