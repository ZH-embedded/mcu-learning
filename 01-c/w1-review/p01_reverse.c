#include <stdio.h>
#include <stdlib.h>

/* ============================================================
 * p01：反转单链表
 *
 * 题目：给一条单链表的表头，把整条链的方向倒过来，返回新的表头。
 *       1 -> 2 -> 3 -> NULL   变成   3 -> 2 -> 1 -> NULL
 *
 * 约束：原地反转，不许 malloc 新节点，不许改 value。
 *
 * 为什么这题面试常考：它只考一件事 —— **改 next 之前，下一个节点的
 * 地址必须先存下来**。新手十有八九先改后存，链当场断掉，后面的节点
 * 全丢，而且找不回来（单链表没有"前一个"可以退）。
 *
 * 两个版本都要会：
 *   迭代版 —— 工程里就用这个，O(n) 时间、O(1) 空间、不占栈
 *   递归版 —— 面试官问"还有别的写法吗"时的答案，但**嵌入式里不能上**，
 *             它的栈深度等于链表长度（最后一节让你亲手量一次代价）
 * ============================================================ */

typedef struct node
{
    int value;
    struct node *next;
} node_t;

/* ============ 工具函数：已经写好，不用动 ============ */

/* 用数组建一条链，返回表头；n == 0 时返回 NULL */
static node_t *build(const int *v, int n)
{
    node_t *head = NULL;
    node_t *tail = NULL;

    for (int i = 0; i < n; i++)
    {
        node_t *p = malloc(sizeof(*p));
        if (p == NULL) { exit(1); }
        p->value = v[i];
        p->next = NULL;

        if (tail == NULL) { head = p; }
        else              { tail->next = p; }
        tail = p;
    }
    return head;
}

static void free_list(node_t *head)
{
    while (head != NULL)
    {
        node_t *next = head->next;
        free(head);
        head = next;
    }
}

/* 遍历把 value 抄进 out，最多抄 cap 个，返回实际个数。
 * 为什么要有 cap：万一你把链表接成了环，没有上限的遍历会卡死，
 * 测试永远不返回 —— 有上限才会变成一条失败断言，而不是一场事故。 */
static int collect(node_t *head, int *out, int cap)
{
    int n = 0;
    while (head != NULL && n < cap)
    {
        out[n] = head->value;
        n++;
        head = head->next;
    }
    return n;
}

static int seq_is(const int *got, int n, const int *want, int wn)
{
    if (n != wn) { return 0; }
    for (int i = 0; i < n; i++)
    {
        if (got[i] != want[i]) { return 0; }
    }
    return 1;
}

/* ============ 从这里开始写 ============ */

/* TODO 1：迭代版反转，返回新表头
 *
 *   - 三个指针：prev（已经反转好的那段的头）、cur（当前要处理的）、next（下一个）
 *   - 每轮四步，**顺序不能换**：
 *       ① next = cur->next        先记住下一个，不然改完就再也找不到
 *       ② cur->next = prev        把当前节点接到已反转段的头上
 *       ③ prev = cur              已反转段扩大一格
 *       ④ cur = next              处理下一个
 *   - cur 走到 NULL 时循环结束，prev 就是新表头
 *   - 空链表不用特判：循环一次都不进，prev 初值 NULL 天然正确
 *
 *   最常见的错：把 ① 和 ② 写反。改完 cur->next 之后 next 已经丢了，
 *   ② 之后再取 cur->next 拿到的是 prev，链从这里断成两截。
 */
node_t *reverse_list(node_t *head)
{
    if (head == NULL)
    {
        return NULL;
    }

    node_t* prev = NULL;
    node_t* cur = head;
    
    while (cur != NULL)
    {
        node_t* next = cur->next;
        if (prev == NULL)
        {
            head->next = NULL;
        }
        cur->next = prev;
        prev = cur; 
        cur = next;

    }
    return prev;
    
}

/* TODO 2：递归版反转，返回新表头
 *
 *   - 先递归到最后一个节点，它就是新表头，一路上原样往上传
 *   - 回程时做两件事：
 *       head->next->next = head    让"我后面的节点"指回我
 *       head->next = NULL          我断开原来的指向
 *   - 终止条件两个都要：head == NULL（空表）、head->next == NULL（只剩一个）
 *     少写 head == NULL 的话，传进来空表会在 head->next 上崩
 *
 *   代价：栈深度 = 链表长度。一个栈帧在 x86-64 上约 48 字节，
 *   500 个节点吃掉二十几 KB。PC 上随便跑，STM32 默认栈只有 1~2 KB ——
 *   所以这版**只是面试答案**，真项目里不用。
 */
node_t *reverse_list_rec(node_t *head)
{
    if (head == NULL || head->next == NULL)
    {
        return head;
    }

    node_t *new_head = reverse_list_rec(head->next);   /* ← 先沉到底，接住新表头 */

    head->next->next = head;
    head->next = NULL;
    return new_head;
    
    
}

/* ============ 测试 ============ */

static int pass = 0, fail = 0;
static void ck(const char *name, long long got, long long want)
{
    if (got == want)
    {
        pass++;
        printf("  [PASS] %-46s = %lld\n", name, got);
    }
    else
    {
        fail++;
        printf("  [FAIL] %-46s 期望 %lld，实际 %lld\n", name, want, got);
    }
}

int main(void)
{
    int buf[16];
    node_t *a;
    node_t *r;
    int n;

    printf("-- 迭代版：基本形状 --\n");
    {
        /* 空表：必须返回 NULL，不能崩 */
        ck("reverse_list(NULL) == NULL", reverse_list(NULL) == NULL, 1);

        /* 单节点：反转后还是它自己，且 next 必须为 NULL。
         * 写成 prev = head; head->next = prev; 就会自环，这条会挂。 */
        a = build((const int[]){7}, 1);
        r = reverse_list(a);
        ck("单节点：值不变", r != NULL ? r->value : -1, 7);
        ck("单节点：next 为 NULL", r != NULL && r->next == NULL, 1);
        free_list(r);

        /* 两节点 */
        a = build((const int[]){1, 2}, 2);
        r = reverse_list(a);
        n = collect(r, buf, 16);
        ck("两节点 [1,2] -> [2,1]", seq_is(buf, n, (const int[]){2, 1}, 2), 1);
        free_list(r);

        /* 三节点：最经典的用例，纸上就画它 */
        a = build((const int[]){1, 2, 3}, 3);
        r = reverse_list(a);
        n = collect(r, buf, 16);
        ck("三节点 [1,2,3] -> [3,2,1]", seq_is(buf, n, (const int[]){3, 2, 1}, 3), 1);
        free_list(r);

        /* 四节点：专查表尾。漏了 head->next = NULL 的实现在这里自环，
         * collect 撞到 cap 停下，下面"节点数"那条会变成 16 而不是 4。 */
        a = build((const int[]){1, 2, 3, 4}, 4);
        r = reverse_list(a);
        n = collect(r, buf, 16);
        ck("四节点 [1,2,3,4] -> [4,3,2,1]", seq_is(buf, n, (const int[]){4, 3, 2, 1}, 4), 1);
        ck("四节点：仍是 4 个节点（表尾 next 干净）", n, 4);
        free_list(r);
    }

    printf("-- 迭代版：没偷偷新造节点 --\n");
    {
        /* 题目要求原地改指针。如果实现是"边遍历边 malloc 新节点头插"，
         * 结果序列照样对，但节点地址全变了 —— 序列断言分辨不出来，地址能。 */
        a = build((const int[]){1, 2, 3}, 3);
        node_t *old_head = a;
        node_t *old_mid = a->next;
        node_t *old_tail = a->next->next;
        r = reverse_list(a);

        ck("新表头 == 原来的表尾（原地，没新造节点）", r == old_tail, 1);
        ck("新表头的 next == 原中间节点", r != NULL && r->next == old_mid, 1);
        ck("新表尾 == 原表头，且 next 已断开",
           r != NULL && r->next != NULL && r->next->next == old_head && old_head->next == NULL, 1);
        free_list(r);
    }

    printf("-- 迭代版：反两次回原样 --\n");
    {
        /* 注意这条是弱判据："什么都不做"的函数反两次也回原样，
         * 所以它只能当补充，不能当主判据（上周 reverse_bits8 就栽在这）。 */
        a = build((const int[]){1, 2, 3, 4, 5}, 5);
        r = reverse_list(reverse_list(a));
        n = collect(r, buf, 16);
        ck("反两次 [1..5] 恢复原序", seq_is(buf, n, (const int[]){1, 2, 3, 4, 5}, 5), 1);
        free_list(r);
    }

    printf("-- 递归版 --\n");
    {
        ck("reverse_list_rec(NULL) == NULL", reverse_list_rec(NULL) == NULL, 1);

        a = build((const int[]){7}, 1);
        r = reverse_list_rec(a);
        ck("递归：单节点 next 为 NULL", r != NULL && r->next == NULL, 1);
        free_list(r);

        a = build((const int[]){1, 2, 3, 4}, 4);
        r = reverse_list_rec(a);
        n = collect(r, buf, 16);
        ck("递归 [1,2,3,4] -> [4,3,2,1]", seq_is(buf, n, (const int[]){4, 3, 2, 1}, 4), 1);
        ck("递归：仍是 4 个节点", n, 4);
        free_list(r);

        a = build((const int[]){1, 2, 3, 4, 5}, 5);
        r = reverse_list_rec(reverse_list_rec(a));
        n = collect(r, buf, 16);
        ck("递归反两次恢复原序", seq_is(buf, n, (const int[]){1, 2, 3, 4, 5}, 5), 1);
        free_list(r);
    }

    printf("-- 递归版的栈代价（PC 上必过，板子上是另一回事）--\n");
    {
        int big[500];
        int *out = malloc(sizeof(int) * 500);
        if (out == NULL) { return 1; }
        for (int i = 0; i < 500; i++) { big[i] = i; }

        a = build(big, 500);
        r = reverse_list_rec(a);
        ck("500 个节点递归反转：节点数不变", collect(r, out, 500), 500);
        ck("500 个节点：新表头是原来的表尾", r != NULL ? r->value : -1, 499);
        free(out);
        free_list(r);
    }

    printf("结果: %d 通过, %d 失败\n", pass, fail);
    return fail != 0;
}
