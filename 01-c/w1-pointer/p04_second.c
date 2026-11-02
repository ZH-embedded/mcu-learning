/* p04_second.c —— 二级指针：指针变量自己也有地址
 *
 * 这一题是今天的重点，也是 12 月链表删除头节点时会用到的模式。
 *
 * 三层关系，把它想成"房间和门牌"：
 *      int  v;        v  是一个房间，里面放着整数
 *      int *p = &v;   p  是一张纸条，上面写着 v 的房间号
 *      int **pp = &p; pp 是另一张纸条，上面写着 p 这张纸条放在哪
 *
 *      *pp   →  p        （读出那张纸条）
 *      **pp  →  v        （顺着两张纸条走到房间）
 *
 * 什么时候真的需要二级指针？
 *   当函数要【修改调用者的指针变量本身】时。
 *   传 int *  只能改"指针指向的那个内容"；
 *   传 int ** 才能改"指针指向哪里"。
 */
#include <stdio.h>

/* ============================================================
 * TODO 1：通过二级指针给指针"换目标"
 *
 * 把 **pp 指向 arr[idx]。
 * 注意先判空（pp 为 NULL、*pp 为 NULL 都要防），越界返回 0。
 * 成功返回 1，失败返回 0（失败时不改 *pp）。
 * ============================================================ */
int point_to(int **pp, int *arr, int n, int idx)
{
    if ((pp == NULL)||(arr == NULL)||(idx >= n)||(n <= 0)||(idx < 0))
    {
        return 0;
    }
    *pp = arr + idx;
    return 1;

}

/* ============================================================
 * TODO 2：二级指针"取走"一个指针，并把它设为 NULL
 *
 * 语义：读取 *pp 的值作为返回值，然后把 *pp 置为 NULL。
 *       pp 为 NULL 时直接返回 NULL。
 *
 * 这个"读走并置空"的写法在链表 pop / 释放后防野指针时很常见：
 * 谁交出去，谁就置空，之后不会再有人误用。
 * ============================================================ */
int *take_and_clear(int **pp)
{
    if (pp == NULL)
    {
        return NULL;
    }

    int* a = *pp;
    *pp = NULL;
    return a;
}

/* ============================================================
 * TODO 3：二级指针数组 —— 遍历一堆指针
 *
 * names 是 "指向指针的指针"，也就是 int **。
 * 求所有指针所指元素的和，共 n 个。n <= 0 返回 0。
 *
 * 提示：names[i] 是 int *，所以 *names[i] 才是值。
 *       写成 **names 是错的（那只是第一个）。
 * ============================================================ */
int sum_ptr_list(int *const *names, int n)
{
    if ((names == NULL) || (n <= 0))
    {
        return 0;
    }

    int num = 0;
    int i = 0;
    while (i < n)
    {
        num = num + **(names + i);
        i++;
    }
    return num;
}

/* ============================================================
 * 以下不用改
 * ============================================================ */
static int g_pass = 0, g_fail = 0;

static void check(const char *name, int got, int want)
{
    if (got == want) {
        printf("  [PASS] %-44s = %d\n", name, got);
        g_pass++;
    } else {
        printf("  [FAIL] %-44s 期望 %d，实际 %d\n", name, want, got);
        g_fail++;
    }
}

static void check_ptr(const char *name, const int *got, const int *want)
{
    if (got == want) {
        printf("  [PASS] %-44s 指向正确\n", name);
        g_pass++;
    } else {
        printf("  [FAIL] %-44s 地址不符：%p vs %p\n",
               name, (const void *)got, (const void *)want);
        g_fail++;
    }
}

int main(void)
{
    int arr[4] = {100, 200, 300, 400};
    int *p = NULL;
    int **pp = &p;

    int v1 = 1, v2 = 2, v3 = 3;
    int *list[3] = {&v1, &v2, &v3};

    printf("=== p04 二级指针 ===\n\n");

    printf("&p  = %p   p 这个变量自己的地址\n", (void *)&p);
    printf("pp  = %p   ← 和上面一样\n", (void *)pp);
    printf("p   = %p   （还没赋值，是 NULL）\n\n", (void *)p);

    printf("--- 测试 ---\n");

    check("point_to 到 idx=2 → 返回 1", point_to(pp, arr, 4, 2), 1);
    check_ptr("p 现在指向 arr[2]", p, &arr[2]);
    check("顺着 p 取值 = 300", p ? *p : -1, 300);
    check("**pp = 300（两层解引用）", (pp && *pp) ? **pp : -1, 300);

    check("point_to 越界 idx=4 → 返回 0", point_to(pp, arr, 4, 4), 0);
    check_ptr("越界时 p 不许改（仍指向 arr[2]）", p, &arr[2]);

    check("point_to 负下标 → 返回 0", point_to(pp, arr, 4, -1), 0);

    /* 空指针防御 */
    {
        int *q = &arr[0];
        check("point_to pp=NULL → 返回 0", point_to(NULL, arr, 4, 1), 0);
        check("point_to arr=NULL → 返回 0", point_to(&q, NULL, 4, 1), 0);
        check_ptr("上面两次失败都没改 q", q, &arr[0]);
    }

    /* take_and_clear */
    {
        int *r = &arr[3];
        int **rr = &r;
        int *taken = take_and_clear(rr);
        check_ptr("take_and_clear 返回原指针", taken, &arr[3]);
        check("take_and_clear 后 r 被置 NULL", r == NULL, 1);
        check("take_and_clear(NULL) 返回 NULL",
              take_and_clear(NULL) == NULL, 1);
    }

    /* 指针数组求和 */
    check("sum_ptr_list = 6", sum_ptr_list(list, 3), 6);
    check("sum_ptr_list n=0 = 0", sum_ptr_list(list, 0), 0);

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
