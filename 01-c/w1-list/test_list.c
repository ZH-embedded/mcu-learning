/* test_list.c —— 单链表的测试 main（11-05）
 *
 * 这是你第一次写"测试文件"：它不是库的一部分，只负责把 list.c 的每个函数
 * 按顺序戳一遍，然后数 PASS / FAIL。真实项目的单元测试就是这个结构，
 * 只是把 check_int 换成 gtest / Unity 的断言宏。
 *
 * 测试顺序有意从简到难：init → 头插 → 尾插 → 混插 → 查找 → 删除 → 边界 → NULL 防御。
 * 每写完 list.c 里的一个 TODO 就跑一次，看 PASS 数量往上走。
 */
#include <stdio.h>

#include "list.h"

static int g_pass = 0;
static int g_fail = 0;

static void check_int(const char *name, int got, int want)
{
    if (got == want)
    {
        printf("  [PASS] %-46s = %d\n", name, got);
        g_pass++;
    }
    else
    {
        printf("  [FAIL] %-46s 期望 %d，实际 %d\n", name, want, got);
        g_fail++;
    }
}

/* 打印实际的链表，FAIL 时用来对照。
 * 加了 32 个结点的护栏：尾插写成"自己指向自己"会成环，单元测试不能被一个死循环卡住。 */
static void show_list(const list_t *l)
{
    const list_node_t *p = (l == NULL) ? NULL : l->head;
    int guard = 0;

    printf("[");
    while (p != NULL && guard < 32)
    {
        printf("%d", p->value);
        if (p->next != NULL)
        {
            printf(" -> ");
        }
        p = p->next;
        guard++;
    }
    if (guard >= 32)
    {
        printf(" ...(超过 32 个结点，可能成环)");
    }
    printf("]");
}

/* 一次检查三件事：值序列、结点个数（多一个少一个都算错）、count 字段。
 * 只比较 value 序列会漏掉"count 忘了维护"这种 bug。 */
static void check_list(const char *name, const list_t *l, const int *want, int want_n)
{
    const list_node_t *p = (l == NULL) ? NULL : l->head;
    int i = 0;
    int ok = 1;

    while (ok && i < want_n)
    {
        if (p == NULL || p->value != want[i])
        {
            ok = 0;
        }
        else
        {
            p = p->next;
            i++;
        }
    }
    if (ok && p != NULL)
    {
        ok = 0;                     /* 期望 3 个，实际 4 个 —— 多出来的也是错 */
    }
    if (ok && list_size(l) != want_n)
    {
        ok = 0;                     /* 序列对但 count 不对 */
    }

    if (ok)
    {
        printf("  [PASS] %-46s ", name);
        show_list(l);
        printf(" (n=%d)\n", list_size(l));
        g_pass++;
    }
    else
    {
        printf("  [FAIL] %-46s 期望 [", name);
        for (int k = 0; k < want_n; k++)
        {
            printf("%d%s", want[k], (k + 1 < want_n) ? " -> " : "");
        }
        printf("] (n=%d)，实际 ", want_n);
        show_list(l);
        printf(" (n=%d)\n", list_size(l));
        g_fail++;
    }
}

int main(void)
{
    list_t L;
    list_t E;                       /* 专门用来测"空链表"的分支 */
    list_node_t *f;

    int w321[]   = { 3, 2, 1 };
    int w123[]   = { 1, 2, 3 };
    int w01234[] = { 0, 1, 2, 3, 4 };
    int w0134[]  = { 0, 1, 3, 4 };
    int w134[]   = { 1, 3, 4 };
    int w13[]    = { 1, 3 };
    int w1[]     = { 1 };
    int w7[]     = { 7 };
    int w555[]   = { 5, 5, 5 };
    int w55[]    = { 5, 5 };
    int w5[]     = { 5 };

    printf("=== 11-05 单链表：头插 / 尾插 / 查找 / 删除 / 释放 ===\n\n");
    printf("--- 测试 ---\n");

    /* ==================== ① 初始化 ==================== */
    list_init(&L);
    check_int("list_init 后 head == NULL", L.head == NULL, 1);
    check_int("list_init 后 size == 0", list_size(&L), 0);

    /* ==================== ② 头插：后进先出 ==================== */
    check_int("push_front(1) 成功返回 1", list_push_front(&L, 1), 1);
    list_push_front(&L, 2);
    list_push_front(&L, 3);
    check_list("头插 1,2,3 → 3,2,1（后进先出）", &L, w321, 3);
    check_int("  head 是最后插进去的 3", (L.head != NULL) ? L.head->value : -1, 3);
    check_int("  size 跟着涨到 3", list_size(&L), 3);

    /* ==================== ③ 尾插：顺序不变 ==================== */
    list_destroy(&L);
    list_init(&L);
    check_int("空表上 push_back(1) 返回 1", list_push_back(&L, 1), 1);
    check_int("空表尾插后 head 不为 NULL（最容易漏的分支）", L.head != NULL, 1);
    list_push_back(&L, 2);
    list_push_back(&L, 3);
    check_list("尾插 1,2,3 → 1,2,3（顺序不变）", &L, w123, 3);
    check_int("  尾插后 size = 3", list_size(&L), 3);

    /* ==================== ④ 头尾混插 ==================== */
    list_push_front(&L, 0);
    list_push_back(&L, 4);
    check_list("头插 0 + 尾插 4 → 0,1,2,3,4", &L, w01234, 5);

    /* ==================== ⑤ 查找 ==================== */
    f = list_find(&L, 2);
    check_int("find(2) 命中，返回非 NULL", f != NULL, 1);
    check_int("  命中结点的 value = 2", (f != NULL) ? f->value : -1, 2);
    check_int("find(99) 未命中返回 NULL", list_find(&L, 99) == NULL, 1);

    list_init(&E);
    check_int("空链表 find 返回 NULL（不是崩）", list_find(&E, 1) == NULL, 1);
    list_destroy(&E);

    /* ==================== ⑥ 删除：中间 / 头 / 尾 / 不存在 ==================== */
    check_int("remove(2) 删中间结点，返回 1", list_remove(&L, 2), 1);
    check_list("  删完 → 0,1,3,4", &L, w0134, 4);
    check_int("remove(2) 再删一次返回 0（已经没了）", list_remove(&L, 2), 0);
    check_int("remove(0) 删头结点，返回 1", list_remove(&L, 0), 1);
    check_list("  删完 → 1,3,4（head 已经指向 1）", &L, w134, 3);
    check_int("remove(4) 删尾结点，返回 1", list_remove(&L, 4), 1);
    check_list("  删完 → 1,3", &L, w13, 2);
    check_int("remove(3) 返回 1", list_remove(&L, 3), 1);
    check_list("  删完 → 1", &L, w1, 1);
    check_int("remove(1) 删最后一个，返回 1", list_remove(&L, 1), 1);
    check_int("  删光后 size = 0", list_size(&L), 0);
    check_int("  删光后 head 必须回到 NULL", L.head == NULL, 1);

    /* ==================== ⑦ 删光以后再插（抓"head 没复位"） ==================== */
    list_push_front(&L, 7);
    check_list("删光后再头插 7 → 7（链表还能用）", &L, w7, 1);

    /* ==================== ⑧ 重复值：只删第一个 ==================== */
    list_destroy(&L);
    list_init(&L);
    list_push_back(&L, 5);
    list_push_back(&L, 5);
    list_push_back(&L, 5);
    check_list("尾插三个 5 → 5,5,5", &L, w555, 3);
    check_int("remove(5) 一次只删一个，返回 1", list_remove(&L, 5), 1);
    check_list("  还剩两个 5", &L, w55, 2);
    check_int("再 remove(5) 返回 1", list_remove(&L, 5), 1);
    check_list("  还剩一个 5", &L, w5, 1);

    /* ==================== ⑨ 释放：幂等 ==================== */
    list_destroy(&L);
    check_int("destroy 后 head == NULL", L.head == NULL, 1);
    check_int("destroy 后 size == 0", list_size(&L), 0);
    check_int("再 destroy 一次不崩（可重复调用）", (list_destroy(&L), 1), 1);

    /* ==================== ⑩ NULL 防御 ==================== */
    check_int("push_front(NULL) 返回 0", list_push_front(NULL, 1), 0);
    check_int("push_back(NULL) 返回 0", list_push_back(NULL, 1), 0);
    check_int("remove(NULL) 返回 0", list_remove(NULL, 1), 0);
    check_int("find(NULL) 返回 NULL", list_find(NULL, 1) == NULL, 1);
    check_int("size(NULL) 返回 0", list_size(NULL), 0);
    check_int("init(NULL) / destroy(NULL) 不崩", (list_init(NULL), list_destroy(NULL), 1), 1);

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
