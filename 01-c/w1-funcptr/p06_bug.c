/* p06_bug.c —— 4 个真的会炸的指针 bug
 *
 * 这一题和前面不同：每题给一段**错误写法**（放在注释里），你自己写正确版本。
 * 错误写法大多编译能过 —— 这才是它危险的地方。
 *
 * 挑错的顺序建议：先看"这段代码想干什么"，再看"它凭什么以为自己做成了"。
 * 面试官问"你踩过什么坑"时，能讲清"为什么编译不报错"比"报什么错"值钱。
 */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

/* ============================================================
 * bug 1：没初始化的函数指针，直接调用
 *
 * 错误版本（面试时你可以讲这段）：
 *
 *     void (*fn)(int);      // 局部变量，没有初始化 → 值是栈上的垃圾
 *     fn(42);               // 于是 CPU 跳到一个随机地址
 *
 * 在 PC 上是 "Segmentation fault"；在 STM32 上是 HardFault 或者干脆
 * 跑飞到一个看起来正常的地址，然后你的程序再也不响应 —— 现场最难查的那种。
 *
 * 修法不是"记得初始化"（人一定会忘），而是**每个间接调用前都判空**。
 * 这也是为什么 HAL 里的回调都要先 if (HAL_XXX_Callback != NULL)。
 *
 * TODO：fn 不为 NULL 才调用 fn(v)；为 NULL 时什么也不做。
 * ============================================================ */
void call_if_set(void (*fn)(int), int v)
{
    if (fn == NULL)
    {
        return;
    }

    fn(v);
}

/* ============================================================
 * bug 2：回调想改外面的变量，但它只拿到了值
 *
 * 错误版本：
 *
 *     static int total = 0;
 *     void add_to(int v) { total += v; }    // 只能用全局变量
 *
 * 用全局能"跑通"，但立刻出三个问题：
 *   ① 两个地方同时累加就串了（可重入性没了）
 *   ② 这个回调只能服务一个累加器，第二个需求就得再写一个
 *   ③ 单元测试之间互相污染，跑两遍结果不一样
 *
 * 标准解法：回调多收一个 void *ctx，调用者把自己的东西塞进去。
 * 这样回调本身不碰任何全局变量，想累加到哪就传哪个地址。
 *
 * 这就是 POSIX 的 pthread_create(..., void *arg)、FreeRTOS 的
 * xTaskCreate(..., pvParameters)，以及各种 xxx_Callback(void *user)
 * 里那个 void * 的全部意义。
 *
 * TODO：遍历 arr，对每个元素调用 fn(arr[i], ctx)。
 *       arr / fn 为 NULL 或 n <= 0 时直接 return。
 * ============================================================ */
typedef void (*visit_fn)(int value, void *ctx);

void walk_with_ctx(const int *arr, int n, visit_fn fn, void *ctx)
{
    if (arr == NULL || fn == NULL || n <= 0)
    {
        return;
    }

    for (int i = 0; i < n; i++)
    {
        fn(arr[i], ctx);
    }
}

/* ============================================================
 * bug 3：结构体按值传，改的是副本
 *
 * 错误版本：
 *
 *     void bump(item_t it) { it.value++; }    // 改的是形参那份拷贝
 *     bump(s);                                // s.value 一点没变
 *
 * 结构体按值传 = 整块 memcpy 到栈上。函数里怎么改都只是改副本，
 * 函数返回时副本被丢掉。PC 上你觉得"没生效"就完事了，
 * 板子上还可能因为结构体太大直接把栈撑爆。
 *
 * TODO：通过指针把 it->value 加 1；it 为 NULL 直接 return。
 * ============================================================ */
typedef struct {
    int id;
    int value;
} item_t;

/* 错误写法的实体（留在这里让你亲手验证它真的改不到） */
void bump_by_value(item_t it)
{
    it.value++;
}

/* 正确写法的 TODO */
void bump_by_ptr(item_t *it)
{
    if (it == NULL)
    {
        return;
    }

    it->value++;
}

/* ============================================================
 * bug 4：比较器写成 return a - b，边界上溢出
 *
 * 错误版本：
 *
 *     int cmp(const void *a, const void *b) {
 *         return *(const int *)a - *(const int *)b;
 *     }
 *
 * 平时都对，直到碰上 INT_MIN 和正数相减 —— 有符号溢出是 UB，
 * 结果可能是正数，于是排序把大的排到前面。这种 bug 只在
 * 某个极端输入下出现，测试用例跑一百遍都是绿的。
 *
 * 正确写法只关心符号，不做减法：
 *     return (x > y) - (x < y);      // 得 -1 / 0 / 1
 *
 * 顺带注意两个细节：
 *   ① qsort 的比较器收到的是 const void *，必须先转成 const int * 再解引用
 *   ② 形参是 const void *，所以 (const int *)a 这个转换是必需的，不能省
 *
 * TODO：按上面的思路实现，返回负数 / 0 / 正数。
 * ============================================================ */
int cmp_safe(const void *a, const void *b)
{
    return (*(const int *)a > *(const int *)b) -
    (*(const int *)a < *(const int *)b);
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

/* bug 1 用的被调函数 */
static int g_got = -1;
static void record(int v) { g_got = v; }

/* bug 2 用的"把值累加进 ctx 指向的 int" */
static void sum_into(int v, void *ctx) { *(int *)ctx += v; }

int main(void)
{
    printf("=== p06 指针 bug 挑错 ===\n\n");

    printf("--- bug 1 未初始化的函数指针 ---\n");
    g_got = -1;
    call_if_set(record, 42);
    check_int("call_if_set(record, 42) 真的调了", g_got, 42);

    g_got = -1;
    call_if_set(NULL, 7);
    check_int("call_if_set(NULL) 不崩、也没调", g_got, -1);

    printf("\n--- bug 2 回调改不到外部变量 ---\n");
    int a[4] = {1, 2, 3, 4};
    int sum = 0;
    walk_with_ctx(a, 4, sum_into, &sum);
    check_int("walk_with_ctx 累加到 ctx 里", sum, 10);

    sum = 100;
    walk_with_ctx(a, 4, sum_into, &sum);
    check_int("换个 ctx 就累加到别处（不碰全局）", sum, 110);

    sum = 0;
    walk_with_ctx(NULL, 4, sum_into, &sum);
    check_int("arr=NULL 不崩", sum, 0);
    walk_with_ctx(a, 4, NULL, &sum);
    check_int("fn=NULL 不崩", sum, 0);

    printf("\n--- bug 3 结构体按值传改不到 ---\n");
    item_t it = {1, 10};
    bump_by_value(it);
    check_int("bump_by_value 改不到外面（这就是那个 bug）", it.value, 10);

    bump_by_ptr(&it);
    check_int("bump_by_ptr 改到了", it.value, 11);
    bump_by_ptr(NULL);
    printf("  [PASS] %-34s 不崩\n", "bump_by_ptr(NULL)");
    g_pass++;

    printf("\n--- bug 4 比较器减法溢出 ---\n");
    int big = 2000000000, small = -2000000000, zero = 0;

    check_int("cmp_safe(大, 小) > 0",
              cmp_safe(&big, &small) > 0 ? 1 : 0, 1);
    check_int("cmp_safe(小, 大) < 0",
              cmp_safe(&small, &big) < 0 ? 1 : 0, 1);
    check_int("cmp_safe(相等) == 0", cmp_safe(&zero, &zero), 0);

    /* 真的拿它去配标准库的 qsort，证明签名对得上 */
    int arr[5] = {5, -3, 9, 0, 2};
    qsort(arr, 5, sizeof(int), cmp_safe);
    int want[5] = {-3, 0, 2, 5, 9};
    check_arr("qsort(..., cmp_safe) 排序正确", arr, want, 5);

    printf("\n（对照）sizeof(item_t) = %zu —— 按值传一次就拷这么多字节\n",
           sizeof(item_t));

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
