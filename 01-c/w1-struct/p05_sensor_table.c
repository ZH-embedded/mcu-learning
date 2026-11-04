/* p05_sensor_table.c —— 当天硬产出：10 条记录，排序，打印
 *
 * 这就是计划里"定义传感器数据结构体，用数组存 10 条记录并排序打印"。
 * 前面四题都是零件，这一题把它们装成一台能用的机器。
 *
 * 结构：
 *   SENSOR_ROM   —— 10 条原始数据，const，放 Flash（板子上不占 RAM）
 *   load_table   —— 从 ROM 拷一份到 RAM 的工作表（要排序就得能改）
 *   sort_table   —— 插入排序，比较规则由参数传进来（昨天的函数指针没白学）
 *   filter_above —— 筛选，用输出参数回传条数
 *   total_value  —— 汇总
 *   print_table  —— 打印，已经给你了
 *
 * 注意 ROM 表和 RAM 表是**两份独立的数据**：改 RAM 表不会影响 ROM 表。
 * 这就是 const 的价值 —— 原始数据永远有一份干净的备份。
 */
#include <stdio.h>
#include <stddef.h>

#define NAME_LEN 12
#define ROM_N 10
#define TABLE_CAP 12

typedef struct
{
    char name[NAME_LEN];
    int id;
    int value;
} sensor_t;

/* 比较器类型：收两个记录的地址，返回 -1 / 0 / 1。
 * 和昨天的 binop_t 是同一套东西 —— 只是指向的函数换了个形状。 */
typedef int (*sensor_cmp_t)(const sensor_t *a, const sensor_t *b);

/* ==================== 10 条原始数据（给了，不用改） ====================
 * 故意不是按 id 排好的，也不是按 value 排好的 —— 否则排序写错了你也看不出来。
 */
static const sensor_t SENSOR_ROM[ROM_N] = {
    {"temp", 104, 25},
    {"hum", 101, 61},
    {"pres", 109, 12},
    {"light", 103, 88},
    {"volt", 107, 33},
    {"co2", 102, 47},
    {"dist", 110, 5},
    {"flow", 105, 72},
    {"ph", 108, 19},
    {"rpm", 106, 54},
};

static int g_pass = 0;
static int g_fail = 0;

static void check_int(const char *name, int got, int want)
{
    if (got == want)
    {
        printf("  [PASS] %-42s = %d\n", name, got);
        g_pass++;
    }
    else
    {
        printf("  [FAIL] %-42s 期望 %d，实际 %d\n", name, want, got);
        g_fail++;
    }
}

/* ==================== 两个比较器（给了） ==================== */
static int cmp_by_id(const sensor_t *a, const sensor_t *b)
{
    if (a->id < b->id)
    {
        return -1;
    }
    if (a->id > b->id)
    {
        return 1;
    }
    return 0;
}

static int cmp_by_value_desc(const sensor_t *a, const sensor_t *b)
{
    if (a->value > b->value)
    {
        return -1;
    }
    if (a->value < b->value)
    {
        return 1;
    }
    return 0;
}

/* ==================== 打印（给了，不用改） ==================== */
static void print_table(const sensor_t *arr, int n)
{
    printf("  +-----+-------+----------+\n");
    printf("  | id  | value | name     |\n");
    printf("  +-----+-------+----------+\n");
    for (int i = 0; i < n; i++)
    {
        printf("  | %-3d | %-5d | %-8s |\n", arr[i].id, arr[i].value, arr[i].name);
    }
    printf("  +-----+-------+----------+\n");
}

/* ============================================================
 * TODO 1：从 ROM 表拷一份到 dst，返回实际拷了几条
 *
 *   - dst 为 NULL 或 cap <= 0 → 返回 0
 *   - 要拷的条数 = ROM_N 和 cap 里较小的那个（cap 不够就截断，别越界写）
 *   - 拷贝直接用 `dst[i] = SENSOR_ROM[i];` —— 结构体赋值是整块拷贝
 *
 * 想一想：为什么必须拷一份，不能直接在 SENSOR_ROM 上排序？
 * （提示：它前面有 const）
 * ============================================================ */
int load_table(sensor_t *dst, int cap)
{
    if (dst == NULL || cap <= 0)
    {
        return 0;
    }

    int min = ROM_N;
    if (ROM_N > cap)
    {
        min = cap;
    }

    for (int i = 0; i < min; i++)
    {
        dst[i] = SENSOR_ROM[i];
    }

    return min;
}

/* ============================================================
 * TODO 2：按 cmp 决定的顺序排序（插入排序，照昨天的骨架填）
 *
 *   - arr 为 NULL、n <= 0、cmp 为 NULL → 直接 return
 *   - 排序过程中"谁在前面"只能问 cmp：cmp(x, y) > 0 说明 x 该往后放
 *
 * 骨架（和昨天一模一样，只是元素从 int 变成了 sensor_t）：
 *
 *     for (int i = 1; i < n; i++) {
 *         sensor_t key = arr[i];        ← 整个结构体拷一份，20 字节
 *         int j = i - 1;
 *         while (j >= 0 && cmp(&arr[j], &key) > 0) {
 *             arr[j + 1] = arr[j];
 *             j--;
 *         }
 *         arr[j + 1] = key;
 *     }
 *
 * 注意 cmp 收的是**地址**，所以是 `&arr[j]` 和 `&key`。
 * 这也顺便说明一件事：传结构体时传指针，传 int 时传值 ——
 * 20 字节的拷贝在 MCU 上不是免费的。
 * ============================================================ */
void sort_table(sensor_t *arr, int n, sensor_cmp_t cmp)
{
    if (arr == NULL || cmp == NULL || n <= 0)
    {
        return;
    }

    for (int i = 1; i < n; i++)
    {
        sensor_t key = arr[i];

        int j = i - 1;
        while (j >= 0 && cmp(&arr[j], &key) > 0)
        {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }


}

/* ============================================================
 * TODO 3：把 src 里 value 严格大于 limit 的记录筛到 dst
 *
 *   - dst_n 为 NULL → 返回 0。这是**唯一**一个"不能写 *dst_n"的情况：
 *     指针本身是空的，没有落地的地方，写下就是段错误
 *   - 其它任何失败（dst 或 src 为 NULL、n <= 0）→ 先把 *dst_n 置 0，再返回 0
 *
 *   注意上面两条的顺序：先判 dst_n（能不能写），再判数据（写什么）。
 *   推荐写法是判完 dst_n 立刻 *dst_n = 0，后面所有失败分支都不用再管它。
 *
 * 返回筛出来的条数，同时通过 *dst_n 回传同一个数。
 *
 * 为什么要有 dst_n 这个输出参数？
 *   函数只能 return 一个值，可这里有两个信息要出去：
 *   "筛出来几条" 和 "每条是什么"。条数走返回值也行，
 *   但调用者拿到 dst 之后还得知道有效长度 —— 所以条数必须写在
 *   调用者能读到的地方。这就是输出参数存在的唯一理由。
 *
 * 调用时会这么写：
 *     int m = 0;
 *     filter_above(out, &m, table, TABLE_CAP, 50);
 *     print_table(out, m);
 * ============================================================ */
int filter_above(sensor_t *dst, int *dst_n,
                 const sensor_t *src, int n, int limit)
{
    if (dst_n == NULL)
    {
        return 0;
    }

    if (dst == NULL || src == NULL || n <= 0)
    {
        *dst_n = 0;
        return 0;
    }

    *dst_n = 0;
    int j = 0;
    for (int i = 0; i < n; i++)
    {
        if (src[i].value > limit)
        {
            dst[j++] = src[i];
            (*dst_n)++;
        }

    }

    return *dst_n;
}

/* ============================================================
 * TODO 4：求所有记录 value 的总和
 *
 *   - arr 为 NULL 或 n <= 0 → 返回 0
 * ============================================================ */
int total_value(const sensor_t *arr, int n)
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

    return sum;
}

int main(void)
{
    sensor_t ram[TABLE_CAP];
    int have;

    printf("=== p05 传感器表：10 条记录，排序，打印 ===\n\n");

    printf("① 原始顺序（从 ROM 拷过来的）\n");
    have = load_table(ram, TABLE_CAP);
    printf("   拷了 %d 条\n", have);
    print_table(ram, have);

    printf("\n② 按 id 升序\n");
    sort_table(ram, have, cmp_by_id);
    print_table(ram, have);

    printf("\n③ 按 value 降序（同一份排序代码，换个比较器）\n");
    sort_table(ram, have, cmp_by_value_desc);
    print_table(ram, have);

    sensor_t big[TABLE_CAP];
    int big_n = -1;
    int m = filter_above(big, &big_n, ram, have, 50);
    printf("\n④ 只留 value > 50 的记录（筛出 %d 条）\n", m);
    print_table(big, big_n);

    printf("\n--- 测试 ---\n");

    check_int("load_table(cap=12) 拷了 10 条", load_table(ram, TABLE_CAP), ROM_N);
    check_int("  拷过来的第 1 条 id 是 104（原顺序）", ram[0].id, 104);
    check_int("  拷过来的最后 1 条 id 是 106（rpm）", ram[ROM_N - 1].id, 106);
    check_int("load_table(cap=3) 截断成 3 条", load_table(ram, 3), 3);
    check_int("load_table(NULL) 返回 0", load_table(NULL, TABLE_CAP), 0);
    check_int("load_table(cap=0) 返回 0", load_table(ram, 0), 0);

    have = load_table(ram, TABLE_CAP);
    sort_table(ram, have, cmp_by_id);
    check_int("按 id 排序后第 1 条 id", ram[0].id, 101);
    check_int("  它的名字是 hum", ram[0].name[0] == 'h' && ram[0].name[1] == 'u', 1);
    check_int("按 id 排序后最后 1 条 id", ram[ROM_N - 1].id, 110);
    check_int("  全部 id 严格递增（排序真的生效了）",
              (ram[0].id < ram[1].id) && (ram[1].id < ram[2].id) &&
                  (ram[2].id < ram[3].id) && (ram[3].id < ram[4].id) &&
                  (ram[4].id < ram[5].id) && (ram[5].id < ram[6].id) &&
                  (ram[6].id < ram[7].id) && (ram[7].id < ram[8].id) &&
                  (ram[8].id < ram[9].id),
              1);

    sort_table(ram, have, cmp_by_value_desc);
    check_int("按 value 降序第 1 条的 value", ram[0].value, 88);
    check_int("按 value 降序最后 1 条的 value", ram[ROM_N - 1].value, 5);

    check_int("sort_table 单元素不崩", (sort_table(ram, 1, cmp_by_id), 1), 1);
    check_int("sort_table n=0 不崩", (sort_table(ram, 0, cmp_by_id), 1), 1);
    check_int("sort_table cmp=NULL 不崩", (sort_table(ram, have, NULL), 1), 1);

    check_int("total_value = 416", total_value(ram, have), 416);
    check_int("total_value n=0 返回 0", total_value(ram, 0), 0);
    check_int("total_value NULL 返回 0", total_value(NULL, have), 0);

    have = load_table(ram, TABLE_CAP);
    big_n = -1;
    check_int("filter_above(>50) 返回 4 条",
              filter_above(big, &big_n, ram, have, 50), 4);
    check_int("  通过 dst_n 回传的也是 4", big_n, 4);
    check_int("  第 1 条 value 是 61", big[0].value, 61);
    check_int("  第 4 条 value 是 54", big[3].value, 54);
    check_int("filter_above(>100) 一条都不剩",
              filter_above(big, &big_n, ram, have, 100), 0);
    check_int("  此时 dst_n 被置 0", big_n, 0);
    big_n = -1;
    check_int("filter_above(src=NULL) 返回 0",
              filter_above(big, &big_n, NULL, have, 50), 0);
    check_int("  此时 dst_n 被清成 0（不是 -1）", big_n, 0);

    big_n = -1;
    check_int("filter_above(n=0) 返回 0",
              filter_above(big, &big_n, ram, 0, 50), 0);
    check_int("  此时 dst_n 被清成 0（不是 -1）", big_n, 0);

    check_int("filter_above(dst_n=NULL) 返回 0 且不崩",
              filter_above(big, NULL, ram, have, 50), 0);

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
