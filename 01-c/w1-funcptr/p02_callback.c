/* p02_callback.c —— 回调：把"做什么"当参数传进来
 *
 * 上一题你把函数指针存起来自己调。这一题反过来：
 * 别人调用**你**，而你调用**它传进来的那个函数**。这就是回调。
 *
 * 回调的价值在于"骨架固定、动作可换"：
 *   for_each 的循环只写一次，每次传入不同的 fn，就变成"每个元素加一"
 *   或者"每个元素打印"或者"每个元素发串口"。
 *
 * 嵌入式里到处都是这个形状：
 *   HAL_GPIO_EXTI_Callback(GPIO_Pin)     中断来了，HAL 调你写的函数
 *   xTaskCreate(task, ..., param)        调度器轮流调你的 task
 *   qsort(arr, n, size, cmp)             排序算法由库提供，比较规则由你提供
 *
 * 本题 4 个函数，全部自己写。cmp/pred 这类"小函数"已经给你了。
 */
#include <stdio.h>
#include <stddef.h>

/* ==================== 已经给你的小函数（不用改） ==================== */

/* 回调 A：加一（通过指针原地改） */
static void bump_one(int *p) { (*p)++; }

/* 回调 B：判定偶数 */
static int is_even(int v) { return (v % 2) == 0; }

/* 回调 C：判定负数 */
static int is_neg(int v) { return v < 0; }

/* 回调 D：判定"大于 100"——用它在本题里制造"一个都找不到"的情况 */
static int is_big(int v) { return v > 100; }

/* 比较器：升序 / 降序。
 * 注意返回值是"负数 / 0 / 正数"，不是差值 —— 用 (a>b)-(a<b) 永远不会溢出。
 * 写成 a-b 在 int 边界上会溢出，面试里被问过很多次。 */
static int cmp_asc(int a, int b)  { return (a > b) - (a < b); }
static int cmp_desc(int a, int b) { return (a < b) - (a > b); }

/* ============================================================
 * TODO 1：对数组每个元素调用 fn
 *
 * 要求：arr 为 NULL 或 n <= 0 时直接 return（什么都不做）；
 *       否则 for 循环，每个元素调用一次 fn(&arr[i])。
 *
 * 为什么要传 &arr[i] 而不是 arr[i]？
 *   传值的话回调改的是副本，外面数组一点没变 —— 这是 p06 第 4 题。
 * ============================================================ */
void for_each(int *arr, int n, void (*fn)(int *))
{
    if ((arr == NULL)||(n <= 0)||(fn == NULL))
    {
        return;
    }

    for (int i = 0; i < n; i++)
    {
        fn(&arr[i]);
    }
    return;
}

/* ============================================================
 * TODO 2：数一数有多少个元素满足 pred
 *
 * 要求：返回满足条件的个数；arr 或 pred 为 NULL、n <= 0 时返回 0。
 *
 * pred 是"收一个 int、返回 int（非 0 为真）"的函数。
 * 你不需要知道它是判偶数还是判负数 —— 这就是解耦。
 * ============================================================ */
int count_if(const int *arr, int n, int (*pred)(int))
{
    if ((arr == NULL)||(pred == NULL)||(n <= 0))
    {
        return 0;
    }

    int m = 0;
    for (int i = 0; i < n; i++)
    {
        if (pred(arr[i]))
        {
            m++;
        }
    }
    return m;
}

/* ============================================================
 * TODO 3：找到第一个满足 pred 的元素，返回它的地址
 *
 * 要求：找到就 return &arr[i]；找不到、或 arr/pred 为 NULL、或 n <= 0，
 *       一律返回 NULL。
 *
 * 返回 const int *，意思是"你能看，但不能通过这个指针改它"。
 * 调用者拿到的地址指向的还是原数组，不是副本。
 * ============================================================ */
const int *find_if(const int *arr, int n, int (*pred)(int))
{
    if ((arr == NULL)||(n <= 0)||(pred == NULL))
    {
        return NULL;
    }

    for (int i = 0; i < n; i++)
    {
        if (pred(arr[i]))
        {
            return &arr[i];
        }

    }
    return NULL;
}

/* ============================================================
 * TODO 4：用 cmp 决定顺序，把数组排序（手写插入排序就够）
 *
 * 要求：arr 为 NULL 或 n <= 0 直接 return；
 *       排序过程中"谁在前面"只能问 cmp：cmp(x, y) > 0 说明 x 该往后放。
 *
 * 这是今天最有价值的一题：**同一份排序代码，传 cmp_asc 就是升序，
 * 传 cmp_desc 就是降序，代码一个字都不用改。**
 * 插入排序骨架（照这个填，别自己另写一套）：
 *
 *     for (int i = 1; i < n; i++) {
 *         int key = arr[i];
 *         int j = i - 1;
 *         while (j >= 0 && cmp(arr[j], key) > 0) {
 *             arr[j + 1] = arr[j];
 *             j--;
 *         }
 *         arr[j + 1] = key;
 *     }
 *
 * 注意 while 里两个条件的前后顺序：j >= 0 必须写在前面，
 * 否则 j 变成 -1 时先去读 arr[-1]，越界。
 * ============================================================ */
void sort_by(int *arr, int n, int (*cmp)(int, int))
{
    if ((arr == NULL)||(n <= 0)||(cmp == NULL))
    {
        return;
    }

    for (int i = 1; i < n; i++) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && cmp(arr[j], key) > 0)
        {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
      }
    return;
}

/* ============================================================
 * 以下不用改
 * ============================================================ */
static int g_pass = 0, g_fail = 0;

static void check_int(const char *name, int got, int want)
{
    if (got == want) {
        printf("  [PASS] %-34s = %d\n", name, got);
        g_pass++;
    } else {
        printf("  [FAIL] %-34s 期望 %d，实际 %d\n", name, want, got);
        g_fail++;
    }
}

static void check_arr(const char *name, const int *got, const int *want, int n)
{
    for (int i = 0; i < n; i++) {
        if (got[i] != want[i]) {
            printf("  [FAIL] %-34s 第 %d 个是 %d，期望 %d\n",
                   name, i, got[i], want[i]);
            g_fail++;
            return;
        }
    }
    printf("  [PASS] %-34s 全对\n", name);
    g_pass++;
}

/* 比较"返回的地址是不是数组里第 idx 个元素" */
static void check_found(const char *name, const int *got,
                        const int *base, int idx)
{
    if (got == base + idx) {
        printf("  [PASS] %-34s 命中第 %d 个\n", name, idx);
        g_pass++;
    } else if (got == NULL) {
        printf("  [FAIL] %-34s 返回了 NULL，期望第 %d 个\n", name, idx);
        g_fail++;
    } else {
        printf("  [FAIL] %-34s 地址不对：%p vs %p\n",
               name, (const void *)got, (const void *)(base + idx));
        g_fail++;
    }
}

static void check_null(const char *name, const int *got)
{
    if (got == NULL) {
        printf("  [PASS] %-34s 是 NULL\n", name);
        g_pass++;
    } else {
        printf("  [FAIL] %-34s 期望 NULL，实际 %p\n",
               name, (const void *)got);
        g_fail++;
    }
}

/* 专门用来数"回调被调了几次" */
static int g_calls = 0;
static void counting_cb(int *p) { (void)p; g_calls++; }

int main(void)
{
    printf("=== p02 回调 ===\n\n");

    /* ---- for_each ---- */
    int a[4] = {1, 3, 5, 7};
    for_each(a, 4, bump_one);
    int want_a[4] = {2, 4, 6, 8};
    check_arr("for_each(bump_one) 每个都加一", a, want_a, 4);

    g_calls = 0;
    for_each(a, 4, counting_cb);
    check_int("for_each 调了 4 次回调", g_calls, 4);

    g_calls = 0;
    for_each(a, 0, counting_cb);
    check_int("for_each n=0 不调用回调", g_calls, 0);

    g_calls = 0;
    for_each(NULL, 4, counting_cb);
    check_int("for_each arr=NULL 不崩", g_calls, 0);

    /* ---- count_if ---- */
    int b[6] = {3, -2, 8, -7, 0, 5};
    check_int("count_if(is_even) = 3 个", count_if(b, 6, is_even), 3);
    check_int("count_if(is_neg)  = 2 个", count_if(b, 6, is_neg), 2);
    check_int("count_if n=0 返回 0", count_if(b, 0, is_even), 0);
    check_int("count_if NULL 返回 0", count_if(NULL, 6, is_even), 0);

    /* ---- find_if ---- */
    check_found("find_if(is_even) 命中第 1 个", find_if(b, 6, is_even), b, 1);
    check_found("find_if(is_neg) 命中第 1 个", find_if(b, 6, is_neg), b, 1);
    check_null("find_if(is_big) 找不到返回 NULL", find_if(b, 6, is_big));
    check_null("find_if n=0 返回 NULL", find_if(b, 0, is_even));
    check_null("find_if arr=NULL 返回 NULL", find_if(NULL, 6, is_even));

    /* ---- sort_by ---- */
    int s1[6] = {5, -3, 9, 0, -8, 2};
    sort_by(s1, 6, cmp_asc);
    int want_asc[6] = {-8, -3, 0, 2, 5, 9};
    check_arr("sort_by(cmp_asc) 升序", s1, want_asc, 6);

    int s2[6] = {5, -3, 9, 0, -8, 2};
    sort_by(s2, 6, cmp_desc);
    int want_desc[6] = {9, 5, 2, 0, -3, -8};
    check_arr("sort_by(cmp_desc) 降序", s2, want_desc, 6);

    int s3[1] = {42};
    sort_by(s3, 1, cmp_asc);
    check_int("sort_by 单元素不崩", s3[0], 42);

    sort_by(NULL, 6, cmp_asc);
    sort_by(s1, 0, cmp_asc);
    printf("  [PASS] %-34s 不崩\n", "sort_by NULL / n=0");
    g_pass++;

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
