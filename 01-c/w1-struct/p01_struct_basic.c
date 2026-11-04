/* p01_struct_basic.c —— 结构体：值语义、内存布局、按字段比较
 *
 * 结构体和数组是两种"聚合类型"，但方向相反：
 *   数组   —— 元素类型全一样，按下标访问，大小 = 元素大小 × 个数
 *   结构体 —— 成员类型可以不同，按名字访问，大小 = 各成员大小之和 + 填充
 *
 * 今天最该记住的一句话：**结构体的大小不等于成员大小之和**。
 * 编译器为了"每个成员都落在对齐的地址上"，会在成员之间塞看不见的填充字节。
 * 你在 PC 上写 sizeof 看不出来，到了板子上算 Flash / RAM 预算就出事了。
 *
 * 两个结构体，成员一模一样，只是顺序不同，大小就不一样 —— 这就是今天要能当面讲清的东西。
 */
#include <stdio.h>
#include <stddef.h>

/* 名字最长 11 个字符 + '\0'。
 * 为什么用定长数组、不用 char *？今天 p06 会踩这个坑，这里先记住结论：
 *   定长数组 = 结构体自带存储，赋值时整块拷贝，不用管内存从哪来；
 *   char *   = 结构体只存一个地址，赋值时两个结构体指向同一块内存（浅拷贝）。 */
#define NAME_LEN 12

typedef struct {
    char name[NAME_LEN];   /* 偏移 0，占 12 字节 */
    int  id;               /* 偏移 12 —— 12 能被 4 整除，不用填充 */
    int  value;            /* 偏移 16 */
} sensor_t;                /* sizeof = 20 */

/* 同样是 1 + 4 + 1 三个成员，换个顺序就完全不是一回事 */
typedef struct {
    char a;                /* 偏移 0，占 1 字节 */
    int  b;                /* 偏移 4 —— 中间塞了 3 字节填充 */
    char c;                /* 偏移 8 */
} probe_t;                 /* sizeof = 12，末尾又补了 3 字节 */

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

static void check_size(const char *name, size_t got, size_t want)
{
    if (got == want)
    {
        printf("  [PASS] %-40s = %zu\n", name, got);
        g_pass++;
    }
    else
    {
        printf("  [FAIL] %-40s 期望 %zu，实际 %zu\n", name, want, got);
        g_fail++;
    }
}

/* 自己写比较，不引 string.h —— 所有字符串都会在 '\0' 处自然结束 */
static int str_same(const char *a, const char *b)
{
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0')
    {
        if (a[i] != b[i])
        {
            return 0;
        }
        i++;
    }
    return a[i] == '\0' && b[i] == '\0';
}

static void check_str(const char *name, const char *got, const char *want)
{
    if (got != NULL && str_same(got, want))
    {
        printf("  [PASS] %-40s \"%s\"\n", name, got);
        g_pass++;
    }
    else
    {
        printf("  [FAIL] %-40s 期望 \"%s\"，实际 \"%s\"\n",
               name, want, got != NULL ? got : "(NULL)");
        g_fail++;
    }
}

/* ============================================================
 * TODO 1：按值构造并返回一个结构体
 *
 * 要求：
 *   - s.id = id，s.value = value
 *   - name 为 NULL 时，s.name 置成空串（name[0] = '\0'）
 *   - 否则把 name 拷进 s.name，**最多写 NAME_LEN - 1 个字符**，最后一个位置必须留给 '\0'
 *   - 不许用 strcpy / string.h，自己写循环
 *
 * 这一题要体会两件事：
 *   ① 结构体可以按值返回（`return s;`），不是只能返回指针
 *   ② 长名字必须截断，否则就是缓冲区溢出 —— p06 会专门考这个
 * ============================================================ */
sensor_t make_sensor(int id, int value, const char *name)
{
    sensor_t s;
    s.name[0] = '\0';

    s.id = id;
    s.value = value;
    if(name != NULL)
    {
        int i = 0;
        for (i = 0; (i < NAME_LEN - 1) &&(name[i] != '\0'); i++)
        {            
            s.name[i] = name[i];
        }
        s.name[i] = '\0';
    }

    return s;
}

/* ============================================================
 * TODO 2：算出 probe_t 里"被填充浪费掉"的字节数
 *
 * 别把 12 写死，也别把 6 写死 —— 用 sizeof(probe_t) 减去各成员大小之和。
 * 这样以后改成员类型，这个函数自动跟着对。
 *
 * 提示：sizeof(probe_t) 是整个结构体，sizeof(probe_t::b) 这种写法不存在，
 *       成员各自的大小用 sizeof(char)、sizeof(int) 拿。
 * ============================================================ */
int padding_of_probe(void)
{
    probe_t m;
    return sizeof(m) - sizeof(m.a) - sizeof(m.b) - sizeof(m.c);
}

/* ============================================================
 * TODO 3：返回 sensor_t 里 value 成员的偏移
 * 用 offsetof 宏（在 <stddef.h> 里），别手数。
 * 顺便想一想：偏移等于前面所有成员大小之和吗？什么时候会不等？
 * ============================================================ */
 size_t offset_of_value(void)
 {
    return offsetof(sensor_t, value);   
 }

/* ============================================================
 * TODO 4：按 id 比较两个结构体
 *
 * 返回 -1（a 的 id 小）/ 0（相等）/ 1（a 的 id 大）。
 * a 或 b 为 NULL 时返回 0。
 *
 * 为什么要写这个函数，而不是直接 `if (*a == *b)`？
 *   ① C 里结构体不能用 == 比较（编译器直接报错），只能逐成员比
 *   ② 就算能比，也不该比 —— 比什么字段是业务决定的：
 *      这里认为 id 相同就是同一条记录，value 不同不算不同记录
 *   ③ 这个签名正好能直接喂给排序函数（p05 要用）
 *
 * 注意别写 a->id - b->id，昨天那个溢出坑还在。
 * ============================================================ */
int cmp_by_id(const sensor_t *a, const sensor_t *b)
{
    if (a == NULL || b == NULL)
    {
        return 0;
    }
    
    return (a->id > b->id) - (a->id < b->id);
}

/* ==================== 布局示意图（只打印，不用改） ==================== */
static void dump_layout(void)
{
    printf("sensor_t 的 20 字节长这样（偏移从 0 开始）：\n\n");
    printf("  +-------------+\n");
    printf("  | 0           |  name[0]  ~ name[11]   12 字节\n");
    printf("  | 1 ... 11    |\n");
    printf("  +-------------+\n");
    printf("  | 12          |  id                    4 字节\n");
    printf("  +-------------+\n");
    printf("  | 16          |  value                 4 字节\n");
    printf("  +-------------+  ← sizeof = 20，没有填充\n\n");

    printf("probe_t 的 12 字节长这样（同样的 1+4+1，却占 12）：\n\n");
    printf("  +---+---+---+---+---+---+---+---+---+---+---+---+\n");
    printf("  | a | 填充(3字节) |     b (4字节)     | c | 填充  |\n");
    printf("  +---+---+---+---+---+---+---+---+---+---+---+---+\n");
    printf("  0   1   2   3   4   5   6   7   8   9  10  11\n");
    printf("       ↑ 偏移 4 才是 int 的合法位置        ↑ 末尾也要补\n\n");
}

int main(void)
{
    printf("=== p01 结构体：值语义与内存布局 ===\n\n");

    printf("sizeof(sensor_t) = %zu   sizeof(probe_t) = %zu\n",
           sizeof(sensor_t), sizeof(probe_t));
    printf("offsetof 实测：name=%zu  id=%zu  value=%zu\n",
           offsetof(sensor_t, name), offsetof(sensor_t, id),
           offsetof(sensor_t, value));
    printf("offsetof 实测：a=%zu  b=%zu  c=%zu\n\n",
           offsetof(probe_t, a), offsetof(probe_t, b),
           offsetof(probe_t, c));

    dump_layout();

    printf("--- 测试 ---\n");

    check_size("sizeof(sensor_t) 是 20", sizeof(sensor_t), 20);
    check_size("sizeof(probe_t) 是 12（1+4+1 却占 12）", sizeof(probe_t), 12);
    check_size("offset_of_value()", offset_of_value(), 16);
    check_int("padding_of_probe() 填充字节数", padding_of_probe(), 6);

    sensor_t s = make_sensor(1, 100, "temp");
    check_int("make_sensor 的 id", s.id, 1);
    check_int("make_sensor 的 value", s.value, 100);
    check_str("make_sensor 的 name", s.name, "temp");

    sensor_t e = make_sensor(2, 0, NULL);
    check_str("name 传 NULL 时是空串", e.name, "");
    check_int("name 传 NULL 时 id 仍然要赋值", e.id, 2);

    sensor_t t = make_sensor(3, 7, "0123456789ABCDEF");
    check_str("超长名字被截断到 11 个字符", t.name, "0123456789A");

    sensor_t lo = make_sensor(1, 0, "a");
    sensor_t hi = make_sensor(9, 0, "b");
    sensor_t lo2 = make_sensor(1, 999, "c");
    check_int("cmp_by_id(1, 9) 返回 -1", cmp_by_id(&lo, &hi), -1);
    check_int("cmp_by_id(9, 1) 返回 1", cmp_by_id(&hi, &lo), 1);
    check_int("cmp_by_id 只比 id，value 不同也算相等",
              cmp_by_id(&lo, &lo2), 0);
    check_int("cmp_by_id(NULL, x) 返回 0", cmp_by_id(NULL, &lo), 0);

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
