/* p06_bug.c —— 结构体 / 枚举 / 共用体上四个真会咬人的 bug
 *
 * 这一题和昨天那道一样：错误版本全部写在注释里，正解留给你写。
 * 这些 bug 的共同点是**能编译、能跑、结果错**，编译器一句话都不说。
 */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#define NAME_LEN 8

typedef struct {
    char name[NAME_LEN];
    int  id;
} rec_t;

/* 枚举当数组下标用 —— 后面 name_of 要用 */
static const char *const NAMES[] = { "IDLE", "RUN", "FAULT" };
#define NAMES_N 3

static int g_pass = 0;
static int g_fail = 0;

static void check_int(const char *name, int got, int want)
{
    if (got == want)
    {
        printf("  [PASS] %-44s = %d\n", name, got);
        g_pass++;
    }
    else
    {
        printf("  [FAIL] %-44s 期望 %d，实际 %d\n", name, want, got);
        g_fail++;
    }
}

static int str_same(const char *a, const char *b)
{
    int i = 0;
    if (a == NULL || b == NULL)
    {
        return 0;
    }
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
        printf("  [PASS] %-44s \"%s\"\n", name, got);
        g_pass++;
    }
    else
    {
        printf("  [FAIL] %-44s 期望 \"%s\"，实际 \"%s\"\n",
               name, want, got != NULL ? got : "(NULL)");
        g_fail++;
    }
}

/* qsort 用的比较器（给了，注意它是 const void * 形态） */
static int cmp_rec(const void *pa, const void *pb)
{
    const rec_t *a = (const rec_t *)pa;
    const rec_t *b = (const rec_t *)pb;
    return (a->id > b->id) - (a->id < b->id);
}

/* ============================================================
 * bug 1：用 memcmp 比较结构体
 *
 * 错误版本：
 *
 *     int same(const rec_t *a, const rec_t *b) {
 *         return memcmp(a, b, sizeof(*a)) == 0;      // 看起来天经地义
 *     }
 *
 * 为什么错：memcmp 比的是**字节**，包括那些填充字节。
 * rec_t 是 char[8] + int，刚好没填充，所以这个版本在本题里碰巧能用。
 * 一旦结构体变成 `{ char a; int id; }`（中间 3 字节填充），
 * 那两个"逻辑上相等"的记录，填充区里可能是上次栈上留下的垃圾，
 * memcmp 就直接判不相等 —— 而且是有时有、有时没有，查到你怀疑人生。
 *
 * 另外 memcmp 比的是"全部字段都相同"，可业务上往往只需要比 id。
 *
 * TODO：id 相同就返回 1，否则返回 0；a 或 b 为 NULL 返回 0。
 *       不许用 memcmp。
 * ============================================================ */
int same_id(const rec_t *a, const rec_t *b)
{
    if (a == NULL || b == NULL)
    {
        return 0;
    }

    return (a->id == b->id);

}

/* ============================================================
 * bug 2：枚举值当数组下标，忘了判范围
 *
 * 错误版本：
 *
 *     const char *name_of(int idx) {
 *         return NAMES[idx];                          // idx 是 99 呢？
 *     }
 *
 * 为什么错：NAMES 只有 3 个元素，NAMES[99] 读到的是数组外面的一段内存，
 * 它被当成 const char * 返回，调用者一 printf("%s") 就去解引用一个
 * 随机地址 —— 有时打出乱码，有时直接崩。而且这类 bug 在现场表现为
 * "偶尔重启"，你根本不会想到是这里。
 *
 * TODO：0 <= idx < NAMES_N 时返回 NAMES[idx]，否则返回 NULL。
 * ============================================================ */
const char *name_of(int idx)
{
    if ((0 <= idx)&&(idx < NAMES_N))
    {
        return NAMES[idx];
    }
    return NULL;
}

/* ============================================================
 * bug 3：往定长数组里拷字符串，不限长度
 *
 * 错误版本：
 *
 *     void set_name(rec_t *s, const char *src) {
 *         strcpy(s->name, src);                       // 名字长了就溢出
 *     }
 *
 * 为什么错：s->name 只有 8 字节。src 是 "temperature"（11 字符 + '\0'），
 * strcpy 会写 12 个字节 —— 多出来的 4 个字节盖掉紧跟在 name 后面的
 * **id 字段**。程序不会崩，只是 id 变成了几个字符的 ASCII 码拼起来的
 * 垃圾数。这是嵌入式里最典型的"改一个字段，另一个字段莫名变了"。
 *
 * 正确做法：最多写 NAME_LEN - 1 个字符，最后一个位置强制写 '\0'。
 *
 * TODO：
 *   - s 为 NULL → 直接 return
 *   - src 为 NULL → s->name 置空串
 *   - 否则拷最多 NAME_LEN - 1 个字符，然后补 '\0'
 *   - 不许用 strcpy / string.h
 * ============================================================ */
void set_name(rec_t *s, const char *src)
{
    if (s == NULL)
    {
        return;
    }
    if (src == NULL)
    {
        s->name[0] = '\0';
        return;
    }

    int i = 0;
    while ((i < NAME_LEN - 1)&&(src[i] != '\0'))
    {
        s->name[i] = src[i];
        i++;
    }
    s->name[i] = '\0';
}

/* ============================================================
 * bug 4：qsort 的第三个参数传错
 *
 * 错误版本：
 *
 *     qsort(arr, n, sizeof(arr), cmp_rec);            // sizeof(指针) = 8
 *     qsort(arr, n, sizeof(rec_t *), cmp_rec);        // 同一个错，写法更明显
 *
 * 为什么错：qsort 的第三个参数是**一个元素占多少字节**。传成 8（指针大小）
 * 而 rec_t 实际是 12 字节，qsort 就会按 8 字节一格去搬数据 ——
 * 元素边界全错，排序结果是一堆半截记录。它不崩，因为始终在自己那块内存里读写。
 *
 * 而且 `sizeof(arr)` 在函数里算的是**形参指针**的大小，不是数组的大小 ——
 * 这就是你在 11-02 学过的"数组形参退化成指针"。sizeof 在这里骗你一次。
 *
 * TODO：用 qsort 按 id 升序排。arr 为 NULL 或 n <= 0 直接 return。
 *       第三个参数必须是 sizeof(rec_t)。
 * ============================================================ */
void sort_records(rec_t *arr, int n)
{
    if (arr == NULL || n <= 0)
    {
        return;
    }

    qsort(arr, (size_t)n, sizeof(rec_t), cmp_rec);
}

int main(void)
{
    printf("=== p06 结构体上的四个 bug ===\n\n");

    printf("sizeof(rec_t) = %zu（char[8] + int，无填充）\n\n", sizeof(rec_t));

    printf("--- 测试 ---\n");

    rec_t r1 = { "a", 7 };
    rec_t r2 = { "b", 7 };
    rec_t r3 = { "c", 9 };
    check_int("same_id 只比 id：名字不同但 id 相同 → 1", same_id(&r1, &r2), 1);
    check_int("same_id id 不同 → 0", same_id(&r1, &r3), 0);
    check_int("same_id(NULL, x) → 0", same_id(NULL, &r1), 0);
    check_int("same_id(x, NULL) → 0", same_id(&r1, NULL), 0);

    check_str("name_of(0)", name_of(0), "IDLE");
    check_str("name_of(2)", name_of(2), "FAULT");
    check_int("name_of(3) 真的是 NULL", name_of(3) == NULL, 1);
    check_int("name_of(-1) 真的是 NULL", name_of(-1) == NULL, 1);
    check_int("name_of(999) 真的是 NULL", name_of(999) == NULL, 1);

    rec_t s = { "", 0 };
    s.id = 1234;
    set_name(&s, "temp");
    check_str("set_name 正常拷", s.name, "temp");
    set_name(&s, "temperature");
    check_str("超长名字被截断到 7 个字符", s.name, "tempera");
    check_int("  截断时 id 没有被冲掉（溢出会盖掉它）", s.id, 1234);
    set_name(&s, NULL);
    check_str("src 为 NULL 时变成空串", s.name, "");
    set_name(NULL, "x");
    check_int("set_name(NULL) 不崩", 1, 1);

    rec_t arr[5] = {
        { "e", 5 }, { "c", 3 }, { "a", 1 }, { "d", 4 }, { "b", 2 }
    };
    sort_records(arr, 5);
    check_int("qsort 后第 1 条 id 是 1", arr[0].id, 1);
    check_int("qsort 后第 5 条 id 是 5", arr[4].id, 5);
    check_int("五条 id 严格递增（元素边界没错位）",
              (arr[0].id < arr[1].id) && (arr[1].id < arr[2].id) &&
              (arr[2].id < arr[3].id) && (arr[3].id < arr[4].id), 1);
    check_int("名字跟着 id 一起搬走了（不是只搬了半个元素）",
              arr[0].name[0] == 'a' && arr[4].name[0] == 'e', 1);
    check_int("sort_records(NULL) 不崩", (sort_records(NULL, 5), 1), 1);
    check_int("sort_records n=0 不崩", (sort_records(arr, 0), 1), 1);

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
