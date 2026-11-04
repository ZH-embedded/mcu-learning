/* p02_struct_array.c —— 结构体数组：遍历、统计、查找
 *
 * 结构体数组是嵌入式里最常见的存数据方式：一张定长的表（RAM 或 Flash 里），
 * 用下标访问。它和"链表"的取舍是今天要能讲清的一点：
 *
 *   数组：长度固定、连续内存、按下标 O(1) 访问、插入删除要搬家
 *   链表：长度可变、节点分散、查找要一个个走、插入删除只改指针
 *
 * 传感器记录这种"条数基本不变、经常整表遍历、偶尔按 id 查一条"的场景，
 * 数组几乎总是对的。链表留到 11-05 再讲。
 *
 * 另外注意这一题的所有形参都是 `const sensor_t *` —— 只读承诺，
 * 你只能读 arr[i].value，写就编不过。
 */
#include <stdio.h>
#include <stddef.h>

#define NAME_LEN 12
#define A_N 5

typedef struct {
    char name[NAME_LEN];
    int  id;
    int  value;
} sensor_t;

/* 一份固定的测试数据（ROM 里的表，谁也别想改） */
static const sensor_t A[A_N] = {
    { "temp",  3, 25 },
    { "hum",   1, 60 },
    { "pres",  4, 25 },
    { "light", 2, 90 },
    { "volt",  5, 12 },
};

static int g_pass = 0;
static int g_fail = 0;

static void check_int(const char *name, int got, int want)
{
    if (got == want)
    {
        printf("  [PASS] %-40s = %d\n", name, got);
        g_pass++;
    }
    else
    {
        printf("  [FAIL] %-40s 期望 %d，实际 %d\n", name, want, got);
        g_fail++;
    }
}

/* ============================================================
 * TODO 1：数一数有多少条记录的 value 严格大于 limit
 *
 * arr 为 NULL 或 n <= 0 返回 0。
 * ============================================================ */
int count_above(const sensor_t *arr, int n, int limit)
{
    if (arr == NULL || n <= 0)
    {
        return 0;
    }

    int num = 0;    
    for (int i = 0; i < n; i++)
    {
        if (arr[i].value > limit)
        {
            num++;
        }
        
    }
    
    return num;   /* 占位 */
}

/* ============================================================
 * TODO 2：返回 value 最大那条记录的下标
 *
 * arr 为 NULL 或 n <= 0 返回 -1。
 * 有并列最大时，返回**下标最小**的那个（先遇到的先算）。
 *
 * 为什么返回下标而不是返回指针？
 *   返回下标的调用者还能干别的：`A[i].name`、`A[i+1]`、把 i 存起来以后再用；
 *   返回指针就只能用那一处，而且数组一挪位置指针就废了。
 *   反过来，如果记录很大、又只想读不关心位置，返回指针更省事（不拷贝）。
 *   两种都对，知道自己在选什么就行。
 * ============================================================ */
int max_index(const sensor_t *arr, int n)
{
    if (arr == NULL || n <= 0)
    {
        return -1;
    }

    int max = arr[0].value;
    int m = 0;
    for (int i = 1; i < n; i++)
    {
        if (arr[i].value > max)
        {
            max = arr[i].value;
            m = i;
        }
        
    }
    return m;
    
}

/* ============================================================
 * TODO 3：按 id 查下标
 *
 * 找到返回下标，找不到返回 -1；arr 为 NULL 或 n <= 0 也返回 -1。
 *
 * 注意这里有个容易犯的错：**下标 0 是合法结果**，
 * 所以不能用"返回值 == 0"表示失败，必须用 -1。
 * ============================================================ */
int index_of_id(const sensor_t *arr, int n, int id)
{
    if (arr == NULL || n <= 0)
    {
        return -1;
    }

    for (int i = 0; i < n; i++)
    {
        if (arr[i].id == id)
        {
            return i;
        }
        
    }
    
    return -1;   /* 占位 */
}

/* ============================================================
 * TODO 4：求所有 value 的整数平均
 *
 * arr 为 NULL 或 n <= 0 返回 0。
 * 整数除法直接截断（212/5 = 42），不用考虑四舍五入。
 *
 * 顺便想一件事：求和用的变量如果写成 `short` 会怎样？
 * 10 条记录、每条 3000，和就是 30000，short 早就溢出了。
 * ============================================================ */
int average_value(const sensor_t *arr, int n)
{
    if (arr == NULL || n <= 0)
    {
        return 0;
    }

    int sum = 0;
    for (int i = 0; i < n; i++)
    {
        sum = sum + arr[i].value;
    }
    
    return sum/n;   /* 占位 */
}

int main(void)
{
    printf("=== p02 结构体数组：遍历与查找 ===\n\n");

    printf("表里 %d 条记录：\n", A_N);
    for (int i = 0; i < A_N; i++)
    {
        printf("  [%d] id=%-2d %-6s value=%d\n",
               i, A[i].id, A[i].name, A[i].value);
    }
    printf("\n");

    printf("--- 测试 ---\n");

    check_int("count_above(limit=25) 严格大于才算", count_above(A, A_N, 25), 2);
    check_int("count_above(limit=0) 全部大于", count_above(A, A_N, 0), 5);
    check_int("count_above(limit=1000) 一个都不大于",
              count_above(A, A_N, 1000), 0);
    check_int("count_above(NULL) 返回 0", count_above(NULL, A_N, 0), 0);
    check_int("count_above n=0 返回 0", count_above(A, 0, 0), 0);

    check_int("max_index 最大值在下标 3", max_index(A, A_N), 3);
    check_int("max_index 只看第 1 条时是 0", max_index(A, 1), 0);
    check_int("max_index(NULL) 返回 -1", max_index(NULL, A_N), -1);
    check_int("max_index n=0 返回 -1", max_index(A, 0), -1);

    check_int("index_of_id(4) 在下标 2", index_of_id(A, A_N, 4), 2);
    check_int("index_of_id(1) 在下标 1（下标 0 才是真陷阱）",
              index_of_id(A, A_N, 1), 1);
    check_int("index_of_id(99) 找不到返回 -1", index_of_id(A, A_N, 99), -1);
    check_int("index_of_id(NULL) 返回 -1", index_of_id(NULL, A_N, 1), -1);

    check_int("average_value = (25+60+25+90+12)/5", average_value(A, A_N), 42);
    check_int("average_value n=0 返回 0", average_value(A, 0), 0);
    check_int("average_value(NULL) 返回 0", average_value(NULL, A_N), 0);

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
