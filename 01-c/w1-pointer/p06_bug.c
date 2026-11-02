/* p06_bug.c —— 四个真实会炸的指针 bug（挑错题）
 *
 * 前五题是"写"，这一题是"读 + 修"。嵌入式里大半时间在找 bug，
 * 能从代码里认出危险模式，比会写新代码更值钱。
 *
 * 每个函数我会给你一段【错误版本】（写在注释里，不进编译），
 * 你要做两件事：
 *   ① 在「我的分析」那行写清根因（一句话就够）
 *   ② 按注释里的约定，把正确版本实现出来，让测试全过
 *
 * 建议：先按错误版本的思路写一遍，看看测试怎么报你，
 *       那个报错信息比任何解释都好记。
 */
#include <stdio.h>

/* ============================================================
 * bug 1：返回局部变量的地址
 *
 * 【错误版本】
 *      int *get_local(void)
 *      {
 *          int local = 12345;
 *          return &local;          // ← 函数一返回，local 就没了
 *      }
 *
 * 现象：调用者拿到一个已经失效的地址，读出来是垃圾值，
 *       或者"有时候恰好是对的"——最坑的一类 bug。
 *
 * 我的分析：local 在栈上，函数返回时栈帧被回收，那块内存随时会被下一次函数调用覆盖。
 * 所以不是"有时候对"——是对错取决于返回后谁占了那块栈。
 *       这也是为什么它比"每次都读错"更危险：读错你能立刻发现，偶尔读对你会信它。
 *
 * 【约定】out == NULL 时返回 NULL；否则把 12345 写进 *out，返回 out。
 *        （这就是 HAL 库的标准做法：谁申请的空间，谁的调用者提供）
 * ============================================================ */
int *get_safe(int *out)
{
    if(out == NULL)
    {
        return NULL;
    }

    *out = 12345;
    return out;
}

/* ============================================================
 * bug 2：循环边界写错，越界写一个元素
 *
 * 【错误版本】
 *      for (i = 0; i <= n; i++) { arr[i] = i; }   // ← i == n 时越界
 *
 * 现象：有时候崩溃，有时候安静地改掉旁边那个变量。
 *       往只有 5 个元素的数组里写 0..5，第 6 次写的是 arr[5]。
 *
 * 我的分析：数组有效下标是 0 到 n-1，所以循环条件必须是 i < n。
 *       多写的那一次 arr[n] 已经不在数组里了，改掉的是紧挨着它的那块内存——
 *       可能是别的变量，可能是返回地址，所以现象时有时无。
 *       验边界最省事的办法：把 n=1 代进去，i<=1 会写 arr[0] 和 arr[1]，一眼看出多了一次。
 *
 * 【约定】把 arr[0]..arr[n-1] 依次填成 0..n-1；
 *        arr == NULL 或 n <= 0 时直接返回，不写任何内存。
 * ============================================================ */
void fill_ok(int *arr, int n)
{
    if ((arr == NULL) || (n <= 0))
    {
        return;
    }

    for (int i = 0; i < n; i++)
    {
        arr[i] = i;
    }
}

/* ============================================================
 * bug 3：没判空就解引用
 *
 * 【错误版本】
 *      int sum_or_zero(int *arr, int n)
 *      {
 *          return *arr;            // ← arr 为 NULL 时直接崩
 *      }
 *
 * 现象：PC 上是 Segmentation fault，板子上是 HardFault，
 *       而且定位到的位置常常不是真凶（栈已经乱了）。
 *
 * 我的分析：NULL 就是地址 0。PC 上触发段错误；但换到你的 STM32 上，地址 0 是向量表的第一个字（初始栈指针），
 * 是 Flash，可读 —— 所以 *arr 往往不崩，只是默默返回一个看着挺像数字的垃圾。
 *       这类 bug 比崩溃难查得多：崩溃至少当场知道出事了，读出错值却会一路传下去。
 *
 * 【约定】arr == NULL 或 n <= 0 时返回 0；否则返回首元素的值。
 * ============================================================ */
int sum_or_zero(int *arr, int n)
{
    if ((arr == NULL)||(n <= 0))
    {
        return 0;
    }
    return arr[0];

}

/* ============================================================
 * bug 4：用 sizeof 求函数参数里的数组长度
 *
 * 【错误版本】
 *      int count_wrong(int arr[])
 *      {
 *          return sizeof(arr) / sizeof(arr[0]);   // ← 得到 8/4 = 2，永远错
 *      }
 *
 * 现象：明明传了 7 个元素，函数里数出来只有 2 个。
 *       因为参数 `int arr[]` 早就退化成 `int *arr`，sizeof 得到的是指针大小。
 *
 * 我的分析：形参 int arr[] 等价于 int *arr，sizeof 得到的是指针大小。
 *       桌面上 8/4 = 2，你的 STM32 上是 4/4 = 1 —— 同一份代码，两个平台给出两个错数。
 *       所以长度只能由调用者传进来，函数自己算不出来。
 *
 * 【约定】长度只能由调用者传进来，所以签名是 (int *arr, int len)。
 *        这里返回 len 本身即可——重点是理解为什么"必须传"。
 * ============================================================ */
int count_ok(int *arr, int len)
{
    /* 长度只能由调用者给出；arr 留着是为了让签名自解释，这里确实用不到 */
    (void)arr;
    return len;
}

/* ============================================================
 * 以下不用改
 * ============================================================ */
static int g_pass = 0, g_fail = 0;

static void check(const char *name, int got, int want)
{
    if (got == want) {
        printf("  [PASS] %-46s = %d\n", name, got);
        g_pass++;
    } else {
        printf("  [FAIL] %-46s 期望 %d，实际 %d\n", name, want, got);
        g_fail++;
    }
}

int main(void)
{
    printf("=== p06 指针 bug 挑错 ===\n\n");

    printf("--- 测试 ---\n");

    /* bug 1 */
    {
        int out = 0;
        int *r = get_safe(&out);
        check("get_safe 写回 12345", out, 12345);
        check("get_safe 返回传入的地址", r == &out, 1);
        check("get_safe(NULL) 返回 NULL", get_safe(NULL) == NULL, 1);
    }

    /* bug 2 */
    {
        int a[5] = {-1, -1, -1, -1, -1};
        int i, ok = 1;
        fill_ok(a, 5);
        for (i = 0; i < 5; i++) if (a[i] != i) ok = 0;
        check("fill_ok 正确填满 0..4", ok, 1);

        /* n <= 0 和 NULL 都要安全 */
        fill_ok(a, 0);
        fill_ok(NULL, 5);
        check("fill_ok 遇 n=0 / NULL 不崩、不写内存", a[0], 0);
    }

    /* bug 3 */
    check("sum_or_zero(NULL, 5) = 0", sum_or_zero(NULL, 5), 0);
    check("sum_or_zero(arr, 0) = 0", sum_or_zero((int[]){1, 2}, 0), 0);
    {
        int b[3] = {11, 22, 33};
        check("sum_or_zero(b, 3) = 11", sum_or_zero(b, 3), 11);
    }

    /* bug 4 */
    {
        int c[7] = {0};
        check("count_ok 传入 7 就返回 7", count_ok(c, 7), 7);

        /* 这一行是给你对照的：错误版本会用 sizeof 算出 2。
         * 这里拆成两个变量来算，避免 -Wsizeof-pointer-div 警告——
         * 那句警告本身就是在提醒你"这样除以 sizeof 是错的"。 */
        {
            size_t sz_ptr = sizeof(int *);
            size_t sz_int = sizeof(int);
            printf("  （对照）sizeof(int*)/sizeof(int) = %zu/%zu = %zu  ← 错误版本得到的就是这个\n",
                   sz_ptr, sz_int, sz_ptr / sz_int);
        }
    }

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
