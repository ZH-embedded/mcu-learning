#include <stdio.h>
#include <stdlib.h>

/* ============================================================
 * p02：合并两个有序单链表
 *
 * 题目：给两条升序单链表 a、b，合并成一条升序链表，返回表头。
 *       a: 1 -> 3 -> 5        b: 2 -> 4 -> 6
 *       结果: 1 -> 2 -> 3 -> 4 -> 5 -> 6
 *
 * 约束：复用原有节点，不许 malloc 新节点，不许改 value。
 *       （注意这和"合并两个数组"不一样 —— 链表合并是**破坏性**的，
 *        合并完 original 的 a、b 两条链就被拆散了，不能再遍历。）
 *
 * 面试官看三点：
 *   ① 会不会用"哨兵节点"省掉"结果链表为空"的特判
 *   ② 一条链先走完时，剩下那段是整段接上还是逐个搬（整段接才是 O(1)）
 *   ③ 返回的是不是栈上哨兵的地址（最常见的悬垂指针）
 * ============================================================ */

typedef struct node
{
    int value;
    struct node *next;
} node_t;

/* ============ 工具函数：已经写好，不用动 ============ */

static node_t *build(const int *v, int n)
{
    node_t *head = NULL;
    node_t *tail = NULL;

    for (int i = 0; i < n; i++)
    {
        node_t *p = malloc(sizeof(*p));
        if (p == NULL)
        {
            exit(1);
        }
        p->value = v[i];
        p->next = NULL;

        if (tail == NULL)
        {
            head = p;
        }
        else
        {
            tail->next = p;
        }
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
    if (n != wn)
    {
        return 0;
    }
    for (int i = 0; i < n; i++)
    {
        if (got[i] != want[i])
        {
            return 0;
        }
    }
    return 1;
}

/* 有序性检查：返回 1 表示整条链非降序 */
static int is_sorted(node_t *head)
{
    while (head != NULL && head->next != NULL)
    {
        if (head->value > head->next->value)
        {
            return 0;
        }
        head = head->next;
    }
    return 1;
}

/* 目标节点在链里出现了几次（查"有没有被重复使用 / 有没有丢"） */
static int count_node(node_t *head, const node_t *target)
{
    int c = 0;
    while (head != NULL)
    {
        if (head == target)
        {
            c++;
        }
        head = head->next;
    }
    return c;
}

/* ============ 从这里开始写 ============ */

/* TODO 1：哨兵节点版合并，返回真正的表头
 *
 *   - 在栈上造一个 dummy 哨兵：node_t dummy; node_t *tail = &dummy;
 *     dummy 不代表任何元素，只是让"第一节点该挂到哪"这件事不必特判
 *   - 循环：两条链都还有节点时，比大小，把小的那个接到 tail->next，两条各自前进
 *   - 循环结束必有一条先空：tail->next = 另一条剩下的那头（**整段接**，别用循环搬）
 *   - 返回 dummy.next
 *
 *   三个坑：
 *     ① 返回 &dummy 而不是 dummy.next —— dummy 是栈上局部变量，
 *        函数一返回它就是野地址，-Wall 会报 -Wreturn-local-addr
 *     ② 忘了 tail = tail->next —— 新节点接上了但尾巴没跟着走，
 *        第二轮会覆盖掉刚接的那个，最后一个元素前的东西全丢
 *     ③ 比较写 < 还是 <= —— 本题约定用 <=，让 a 的等值元素排在前面（稳定）
 */
node_t *merge_two_lists(node_t *a, node_t *b)
{
    node_t dummy;
    node_t *p = &dummy;
    p->next = NULL;
    while (a != NULL && b != NULL)
    {
        if (a->value <= b->value)
        {
            p->next = a;
            a = a->next;
            p = p->next;
        }
        else
        {
            p->next = b;
            b = b->next;
            p = p->next;
        }
    }
    if (a == NULL && b != NULL)
    {
        p->next = b;
    }
    if (b == NULL && a != NULL)
    {
        p->next = a;
    }

    return dummy.next; /* TODO */
}

/* TODO 2：二级指针版合并（不用哨兵），返回表头
 *
 *   - 核心想法：链表的"表头"和"某个节点的 next"在类型上是同一件东西 ——
 *     都是"指向 node 的指针"，都能用 node_t ** 指过去
 *   - 所以让 pp 先指向 head 变量本身，之后每接一个节点，就让它指向
 *     "刚接上那个节点的 next 字段"（写成 pp = &(*pp)->next）
 *   - 这样"插到表头"和"插到中间"变成同一句 *pp = 谁，一个分支都不用写
 *   - 一条链走空后：*pp = 另一条剩下的那头，收工
 *
 *   这是 11-02 二级指针的应用题。写它是为了下次面试官问
 *   "不用哨兵能不能写"时你能答得漂亮 —— 但可读性确实不如版本一，
 *   团队代码里我会选哨兵版。
 */
node_t *merge_two_lists_pp(node_t *a, node_t *b)
{
    node_t *head = NULL;
    node_t **pp = &head;              /* pp 表示"下一个节点该挂到哪" */

    while (a != NULL && b != NULL)
    {
        node_t **pick = (a->value <= b->value) ? &a : &b;
        *pp = *pick;                  /* 挂上 */
        *pick = (*pick)->next;        /* 那一条前进 */
        pp = &(*pp)->next;            /* pp 追到刚挂上节点的 next 字段 */
    }

    *pp = (a != NULL) ? a : b;
    return head;
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

/* 一条链的完整检查：序列 + 有序 + 节点数 + 每个原节点用了恰好一次 */
static void check_merged(const char *tag, node_t *r,
                         const int *want, int wn,
                         node_t **orig, int on)
{
    int buf[16];
    int n = collect(r, buf, 16);

    printf("  <%s>\n", tag);
    ck("序列正确", seq_is(buf, n, want, wn), 1);
    ck("整体有序（非降）", is_sorted(r), 1);
    ck("节点数 = 两条链之和", n, wn);

    int reuse_ok = 1;
    for (int i = 0; i < on; i++)
    {
        if (count_node(r, orig[i]) != 1)
        {
            reuse_ok = 0;
        }
    }
    ck("每个原节点都用了且只用了一次", reuse_ok, 1);
}

int main(void)
{
    printf("-- 边界：有一条是空表 --\n");
    {
        ck("merge(NULL, NULL) == NULL", merge_two_lists(NULL, NULL) == NULL, 1);

        /* 一条空表时必须**原样返回另一条**：返回指针相等说明是 O(1) 接力，
         * 要是重新 malloc 一份，序列也对但地址就变了 */
        node_t *a = build((const int[]){1, 3, 5}, 3);
        ck("merge(a, NULL) 返回 a 本身", merge_two_lists(a, NULL) == a, 1);
        free_list(a);

        node_t *b = build((const int[]){2, 4}, 2);
        ck("merge(NULL, b) 返回 b 本身", merge_two_lists(NULL, b) == b, 1);
        free_list(b);
    }

    printf("-- 等长交错 --\n");
    {
        node_t *a = build((const int[]){1, 3, 5}, 3);
        node_t *b = build((const int[]){2, 4, 6}, 3);
        node_t *orig[6] = {a, a->next, a->next->next, b, b->next, b->next->next};
        node_t *r = merge_two_lists(a, b);
        check_merged("a=1,3,5  b=2,4,6", r, (const int[]){1, 2, 3, 4, 5, 6}, 6, orig, 6);
        free_list(r);
    }

    printf("-- 一条先走完，剩下那段整段接上 --\n");
    {
        /* a 只有 3 个，b 有 6 个：a 走完后 b 剩下的 4 个必须原封不动跟上来。
         * 逐个搬也能得到对的序列，差别在 O(1) 与 O(n)，测不出来 ——
         * 但"每个节点只用一次"这条能抓住"搬的时候复制了"的实现。 */
        node_t *a = build((const int[]){1, 2, 3}, 3);
        node_t *b = build((const int[]){4, 5, 6, 7, 8, 9}, 6);
        node_t *orig[9] = {a, a->next, a->next->next,
                           b, b->next, b->next->next, b->next->next->next,
                           b->next->next->next->next,
                           b->next->next->next->next->next};
        node_t *r = merge_two_lists(a, b);
        check_merged("a=1,2,3  b=4..9", r, (const int[]){1, 2, 3, 4, 5, 6, 7, 8, 9}, 9, orig, 9);
        free_list(r);
    }
    {
        /* 反过来：a 长 b 短，且 b 的第一个就比 a 的表头大 */
        node_t *a = build((const int[]){5, 6, 7, 8, 9}, 5);
        node_t *b = build((const int[]){1}, 1);
        node_t *orig[6] = {a, a->next, a->next->next, a->next->next->next,
                           a->next->next->next->next, b};
        node_t *r = merge_two_lists(a, b);
        ck("b 的元素插到最前面", r != NULL && r == b, 1);
        check_merged("a=5..9  b=1", r, (const int[]){1, 5, 6, 7, 8, 9}, 6, orig, 6);
        free_list(r);
    }

    printf("-- 等值元素（稳定性）--\n");
    {
        /* 本题约定 <= ：a 的等值元素排在 b 的前面。
         * 写成 < 也算对，只是稳定性的选择不同；但既然约定了 <=，
         * 下面这条会明确告诉你用的是哪一个。 */
        node_t *a = build((const int[]){1, 1, 2}, 3);
        node_t *b = build((const int[]){1, 3}, 2);
        node_t *a1 = a, *a2 = a->next, *b1 = b;
        node_t *r = merge_two_lists(a, b);

        printf("  <等值 1 与 1 的先后>\n");
        ck("第一个是 a 的第一个 1", r == a1, 1);
        ck("第二个是 a 的第二个 1", r != NULL && r->next == a2, 1);
        ck("第三个才是 b 的 1（等值取 a）", r != NULL && r->next != NULL && r->next->next == b1, 1);

        int buf[16];
        ck("序列 1,1,1,2,3", seq_is(buf, collect(r, buf, 16), (const int[]){1, 1, 1, 2, 3}, 5), 1);
        free_list(r);
    }

    printf("-- 二级指针版 --\n");
    {
        ck("pp 版 merge(NULL, NULL) == NULL", merge_two_lists_pp(NULL, NULL) == NULL, 1);

        node_t *a = build((const int[]){1, 3, 5}, 3);
        node_t *b = build((const int[]){2, 4, 6}, 3);
        node_t *orig[6] = {a, a->next, a->next->next, b, b->next, b->next->next};
        node_t *r = merge_two_lists_pp(a, b);
        check_merged("pp 版 a=1,3,5  b=2,4,6", r, (const int[]){1, 2, 3, 4, 5, 6}, 6, orig, 6);
        free_list(r);

        /* b 整条都比 a 小：这种用例能把"pp 忘了跟着走"的实现打出来 ——
         * 那种实现在这里会一直往 head 上覆盖，最后只剩一个元素 */
        a = build((const int[]){7, 8, 9}, 3);
        b = build((const int[]){1, 2, 3}, 3);
        node_t *orig2[6] = {a, a->next, a->next->next, b, b->next, b->next->next};
        r = merge_two_lists_pp(a, b);
        check_merged("pp 版 b 全比 a 小", r, (const int[]){1, 2, 3, 7, 8, 9}, 6, orig2, 6);
        free_list(r);

        a = build((const int[]){1, 2, 3}, 3);
        r = merge_two_lists_pp(a, NULL);
        ck("pp 版 merge(a, NULL) 返回 a 本身", r == a, 1);
        free_list(r);
    }

    printf("结果: %d 通过, %d 失败\n", pass, fail);
    return fail != 0;
}
