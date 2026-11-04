/* list.h —— 单链表的接口（11-05）
 *
 * 这是本项目**第一个真正的"模块"**：.h 放接口，.c 放实现，测试单独一个文件。
 * 前两天 p01..p06 是"一个 .c 一个 main"，写多少都只给自己看；
 * 从今天起代码要能被别的文件 include、被别人调用，所以规矩变了：
 *
 *   头文件里只放"调用者必须知道的东西"——类型定义、函数声明、每条函数的约定。
 *   实现细节（怎么找前驱、什么时候 free）一律留在 .c 里。
 *
 * 为什么 list_node_t 的 next 指针要暴露在头文件里？
 *   因为调用者要能遍历（for (p = l->head; p; p = p->next)）。
 *   真实项目里通常会再包一层迭代器（list_begin / list_next），把 next 藏起来，
 *   代价是每个元素多一次函数调用。嵌入式上这个代价不小，所以裸链表很常见。
 *
 * 节点是 malloc 出来的。为什么嵌入式里要慎用，README 里单独一节讲。
 */
#ifndef LIST_H
#define LIST_H

typedef struct list_node
{
    int value;
    struct list_node *next;   /* 指向下一个结点；最后一个结点的 next 是 NULL */
} list_node_t;

typedef struct
{
    list_node_t *head;        /* 空链表时是 NULL —— 这是"空"的唯一判据 */
    int count;                /* 结点个数，跟着 head 一起维护，避免每次 O(n) 去数 */
} list_t;

/* 全部函数的统一约定：
 *   l 为 NULL → 不崩，返回值见各自说明（插入/删除返回 0，find 返回 NULL，size 返回 0）
 *   插入 / 删除成功返回 1，失败返回 0
 *   只有 list_init / list_destroy 改的是"整条链"，其余都是改 l 指向的那条链
 */

void list_init(list_t *l);                    /* 置空，可以重复调用 */
int  list_push_front(list_t *l, int v);       /* 头插，O(1) */
int  list_push_back(list_t *l, int v);        /* 尾插，O(n) */
int  list_remove(list_t *l, int v);           /* 删第一个等于 v 的结点，返回删掉的个数 */
list_node_t *list_find(list_t *l, int v);     /* 找到返回结点地址，找不到返回 NULL */
int  list_size(const list_t *l);              /* 结点个数，l 为 NULL 返回 0 */
void list_destroy(list_t *l);                 /* 释放全部结点，可重复调用 */
void list_print(const list_t *l);             /* 打印成 [1 -> 2 -> 3]，给你了 */

#endif /* LIST_H */
