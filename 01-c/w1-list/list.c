/* list.c —— 单链表的实现（11-05）
 *
 * 五个 TODO，难度从低到高：find → push_front → push_back → remove → destroy。
 * 建议就按这个顺序写，每写完一个就跑一次 ./build_run.sh，让 PASS 一条一条涨上来。
 *
 * 编译方式（和前几天不一样，今天是多文件）：
 *   gcc -Wall -Wextra list.c test_list.c -o test_list.exe
 * 或者直接用目录里的 ./build_run.sh，它会把两个 .c 一起编译。
 *
 * 内存从哪来：每个结点一次 malloc。这就是"嵌入式里慎用链表"的根源 ——
 * 动态分配的耗时不确定、堆会碎、失败还没有返回值可退（malloc 返回 NULL 时你只能返回 0，
 * 上层往往也没处理）。真实产品里常见的替代是**静态内存池**：
 * 一次性开一个 node_t pool[64] 数组，用空闲链把没人用的结点串起来，
 * 分配的复杂度变成 O(1) 且耗时可预测。这件事我们在项目一里会真做一次。
 */
#include <stdio.h>
#include <stdlib.h>

#include "list.h"

/* ==================== 给好的：置空 ====================
 * 它太基础了，留着当"照抄模板"：head 和 count 是一对，必须一起改。
 * 只把 count 置 0、忘了 head，则 count 说"空"、遍历还能走出结点 —— 两个字段说了两套话。
 */
void list_init(list_t *l)
{
    if (l == NULL)
    {
        return;
    }
    l->head = NULL;
    l->count = 0;
}

/* ============================================================
 * TODO 1：查找。返回第一个 value == v 的结点地址，没有就返回 NULL。
 *
 *   - l 为 NULL → 返回 NULL
 *   - 遍历骨架：for (list_node_t *p = l->head; p != NULL; p = p->next)
 *
 * 这个函数是 remove 的基础：能"找到"才谈得上"删掉"。
 * ============================================================ */
list_node_t *list_find(list_t *l, int v)
{
    if (l == NULL)
    {
        return NULL;
    }

    for (list_node_t* m = l->head; m != NULL; m = m->next)
    {
        if (v == m->value)
        {
            return m;
        }

    }
    return NULL;
}

/* ============================================================
 * TODO 2：头插。新结点成为新的头，O(1)。
 *
 *   1. malloc 一个结点：`list_node_t *n = malloc(sizeof(*n));`
 *      —— 写 sizeof(*n)，不要手写 16；类型改了它自动跟着改
 *   2. malloc 可能失败，返回 NULL。判空后返回 0，**不要解引用**
 *   3. 挂上去：先 n->next = l->head，再 l->head = n
 *      —— 这两行的顺序反了会怎样？先把 l->head 改成 n，原链表的头就永远找不到了
 *   4. count++
 *
 * 返回值：成功 1，失败 0
 * ============================================================ */
int list_push_front(list_t *l, int v)
{
    if (l == NULL)
    {
        return 0;
    }

    list_node_t* p = (list_node_t*)malloc(sizeof(list_node_t));
    if(p == NULL)
    {
        return 0;
    }
    p->value = v;
    p->next = l->head;
    l->head = p;
    l->count++;
    return 1;
}

/* ============================================================
 * TODO 3：尾插。需要走到最后一个结点，O(n)。
 *
 *   - 结点照样要先 malloc、判空
 *   - 空链表（l->head == NULL）：新结点既是头也是尾，**head 必须在这里被赋值**
 *   - 非空：从头走到 p->next == NULL 的那个 p，然后 p->next = n
 *   - count++
 *
 * 这里最容易漏的一处：只记得写 p->next = n，忘了空链表的 head。
 * 测试里"空表上第一次尾插"那条断言就是专门抓它的。
 *
 * 想一想：给 list_t 再加一个 tail 指针，尾插是不是就 O(1) 了？
 * 那为什么 Python 的 list、C++ 的 vector 用的还是数组？（提示：删除和随机访问）
 * ============================================================ */
int list_push_back(list_t *l, int v)
{
    if (l == NULL)
    {
        return 0;
    }

    list_node_t *p = (list_node_t *)malloc(sizeof(*p));
    if (p == NULL)
    {
        return 0;
    }
    p->value = v;              /* 先把新节点装修好 */
    p->next  = NULL;

    if (l->head == NULL)       /* 空表：它就是唯一的节点 */
    {
        l->head = p;
    }
    else                       /* 非空：走到尾巴挂上去 */
    {
        list_node_t *m = l->head;
        while (m->next != NULL)
        {
            m = m->next;
        }
        m->next = p;
    }

    l->count++;
    return 1;
}


/* ============================================================
 * TODO 4：按值删除第一个匹配的结点，返回 1；没有这个值返回 0。
 *
 * 单链表只能单向走，所以删除时手里必须同时握着"当前"和"前一个"：
 *
 *     list_node_t *prev = NULL;
 *     list_node_t *cur  = l->head;
 *     while (cur != NULL) {
 *         if (cur->value == v) { ...删掉... }
 *         prev = cur;
 *         cur  = cur->next;
 *     }
 *
 * 三个坑：
 *   1. 删的是**头结点**时 prev 是 NULL，不能写 prev->next（段错误）。要单独处理：
 *      l->head = cur->next;
 *   2. free 之前先把 cur->next 存下来。free 之后 cur 那块内存已经不归你了，
 *      再读 cur->next 是 use-after-free（-O0 常常"看起来正常"，-O2 会给你惊喜）
 *   3. count--，而且只在**真的删掉**的分支里减
 *
 * 返回值：删掉了返回 1，没找到返回 0
 * ============================================================ */
int list_remove(list_t *l, int v)
{
    if (l == NULL)
    {
        return 0;
    }

    list_node_t *prev = NULL;       /* 头结点没有前驱，用 NULL 当标记 */
    list_node_t *cur  = l->head;

    while (cur != NULL)
    {
        if (cur->value == v)
        {
            list_node_t *next = cur->next;   /* free 之前先存下来 */

            if (prev == NULL)
            {
                l->head = next;              /* 删的是头 */
            }
            else
            {
                prev->next = next;           /* 跨过 cur */
            }

            free(cur);
            l->count--;
            return 1;
        }

        prev = cur;                 /* 只是移动指针，不碰节点 */
        cur  = cur->next;
    }
    return 0;                       /* 走到底没找到 */
}


/* ============================================================
 * TODO 5：释放整条链，并把 head / count 复位。
 *
 *   - l 为 NULL → 直接 return
 *   - 遍历时：先存 next，再 free，再前进
 *         list_node_t *next = p->next;
 *         free(p);
 *         p = next;
 *     顺序反了就是 use-after-free
 *   - 最后 l->head = NULL; l->count = 0;
 *
 * 要求"可重复调用"：已经 destroy 过的链表再 destroy 一次，不能崩
 * （head 已经是 NULL，第二次循环直接不进）。
 * ============================================================ */
void list_destroy(list_t *l)
{
    if (l == NULL)
    {
        return;
    }

    list_node_t* cur = l->head;
    while (cur != NULL)
    {
        list_node_t* next = cur->next;
        free(cur);
        l->count--;
        cur = next;
    }
    l->head = NULL;
    l->count = 0;
}

/* ==================== 下面两个是给好的，不用改 ==================== */

int list_size(const list_t *l)
{
    if (l == NULL)
    {
        return 0;
    }
    return l->count;
}

void list_print(const list_t *l)
{
    const list_node_t *p = (l == NULL) ? NULL : l->head;

    printf("  [");
    while (p != NULL)
    {
        printf("%d", p->value);
        if (p->next != NULL)
        {
            printf(" -> ");
        }
        p = p->next;
    }
    printf("]  (n=%d)\n", list_size(l));
}
