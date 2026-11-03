/* p04_struct.c —— 结构体指针：->、const 结构体、返回结构体指针
 *
 * 两条等价的写法，编译出来一样：
 *     (*p).id     ← 先解引用，再取成员。括号不能省：*p.id 是 *(p.id)
 *     p->id       ← 语法糖，专门为指针准备的
 *
 * 为什么必须用指针？
 *   1. 结构体按值传参是**整块拷贝**。传感器结构体 16 字节，拷 10 个就是 160 字节，
 *      栈上还要再开一份。板子上栈只有几 KB，这是真实会炸的地方。
 *   2. 按值传进去改的是副本，外面一点没变 —— 想改必须传地址。
 *
 * 所以真实代码里的结构体形参只有两种写法：
 *     void f(const sensor_t *s)    只读，加 const 防止误写
 *     void f(sensor_t *s)          要改
 * 你几乎见不到 void f(sensor_t s)。
 */
#include <stdio.h>
#include <stddef.h>

#define SENSOR_NAME_LEN 8

typedef struct {
    int  id;
    int  value;
    char name[SENSOR_NAME_LEN];
} sensor_t;

/* ============================================================
 * TODO 1：初始化一个传感器
 *
 * 要求：s 为 NULL 直接 return；
 *       id、value 赋值；name 清空（只写 name[0] = '\0' 就够，
 *       这是 C 字符串的"空串"表示，后面的字节不重要，别去 memset）。
 *
 * 全部用 -> 写，别写 (*s).id。
 * ============================================================ */
void sensor_init(sensor_t *s, int id, int value)
{
    if (s == NULL)
    {
        return;
    }

    s->id = id;
    s->value = value;
    s->name[0] = '\0';

}

/* ============================================================
 * TODO 2：求所有传感器 value 的总和
 *
 * 形参是 const sensor_t * —— 遍历时只能读。
 * 要求：arr 为 NULL 或 n <= 0 返回 0。
 * ============================================================ */
int sensor_total(const sensor_t *arr, int n)
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

/* ============================================================
 * TODO 3：按 id 查找，返回指向那条记录的指针
 *
 * 返回的是数组里那个元素本身的地址（不是副本），
 * 所以调用者拿到之后既能读也能改，改动会落到原数组上。
 *
 * 要求：找不到、或 arr 为 NULL、或 n <= 0，返回 NULL。
 * ============================================================ */
sensor_t *sensor_find(sensor_t *arr, int n, int id)
{
    if (arr == NULL || n <= 0)
    {
        return NULL;
    }

    for (int i = 0; i < n; i++)
    {
        if (arr[i].id == id)
        {
            return &arr[i];
        }

    }
    return NULL;

}

/* ============================================================
 * TODO 4：所有 value 乘以 k
 *
 * 要求：arr 为 NULL 或 n <= 0 直接 return。
 *
 * 写完之后对照一下 sensor_total：一个收 const 指针、一个收普通指针。
 * 编译器就是靠这个区分"这个函数会不会改我的数组"。
 * ============================================================ */
void sensor_scale_all(sensor_t *arr, int n, int k)
{
    if (arr == NULL || n <= 0)
    {
        return;
    }

    for (int i = 0; i < n; i++)
    {
        arr[i].value = arr[i].value * k;
    }
}

/* ============================================================
 * 以下不用改
 * ============================================================ */
static int g_pass = 0, g_fail = 0;

static void check_int(const char *name, int got, int want)
{
    if (got == want) {
        printf("  [PASS] %-36s = %d\n", name, got);
        g_pass++;
    } else {
        printf("  [FAIL] %-36s 期望 %d，实际 %d\n", name, want, got);
        g_fail++;
    }
}

static void check_ptr(const char *name, const sensor_t *got,
                      const sensor_t *want)
{
    if (got == want) {
        printf("  [PASS] %-36s 指向同一处\n", name);
        g_pass++;
    } else if (got == NULL) {
        printf("  [FAIL] %-36s 返回 NULL\n", name);
        g_fail++;
    } else {
        printf("  [FAIL] %-36s 地址不同：%p vs %p\n",
               name, (const void *)got, (const void *)want);
        g_fail++;
    }
}

int main(void)
{
    printf("=== p04 结构体指针 ===\n\n");

    sensor_t arr[3] = {{0, 0, {0}}, {0, 0, {0}}, {0, 0, {0}}};

    printf("sizeof(sensor_t) = %zu 字节   ← 4 + 4 + 8，对齐到 4\n",
           sizeof(sensor_t));
    printf("按值传参会拷这么多字节；传指针只拷 %zu 字节\n\n", sizeof(sensor_t *));

    printf("--- 测试 ---\n");

    arr[0].name[0] = 'X';          /* 先把 name 弄脏，看 init 有没有清干净 */
    sensor_init(&arr[0], 1, 100);
    sensor_init(&arr[1], 2, 200);
    sensor_init(&arr[2], 3, 300);

    check_int("arr[0].id 初始化", arr[0].id, 1);
    check_int("arr[0].value 初始化", arr[0].value, 100);
    check_int("arr[1].id 初始化", arr[1].id, 2);
    check_int("name 是空串", arr[0].name[0] == '\0' ? 1 : 0, 1);
    check_int("sizeof(sensor_t) 是 16", (int)sizeof(sensor_t), 16);

    sensor_init(NULL, 9, 9);
    printf("  [PASS] %-36s 不崩\n", "sensor_init(NULL)");
    g_pass++;

    check_int("sensor_total = 600", sensor_total(arr, 3), 600);
    check_int("sensor_total n=0", sensor_total(arr, 0), 0);
    check_int("sensor_total NULL", sensor_total(NULL, 3), 0);

    sensor_t *p = sensor_find(arr, 3, 2);
    check_ptr("sensor_find(id=2) 命中 arr[1]", p, &arr[1]);
    check_int("找到的记录 value = 200", p ? p->value : -1, 200);

    check_ptr("sensor_find(id=9) 返回 NULL", sensor_find(arr, 3, 9), NULL);
    check_ptr("sensor_find NULL 返回 NULL", sensor_find(NULL, 3, 1), NULL);

    /* 通过 find 返回的指针去改，原数组应该跟着变 */
    if (p != NULL) {
        p->value = 999;
    }
    check_int("通过返回的指针改，改动落在原数组", arr[1].value, 999);

    sensor_scale_all(arr, 3, 2);
    check_int("scale_all 后 arr[0].value", arr[0].value, 200);
    check_int("scale_all 后 arr[1].value", arr[1].value, 1998);
    check_int("scale_all 后 arr[2].value", arr[2].value, 600);

    /* 对照：按值拷贝改副本，原数组不受影响 */
    sensor_t copy = arr[0];
    copy.value = 0;
    check_int("改副本后副本是 0", copy.value, 0);
    check_int("原数组还是 200（这就是按值传参的真相）", arr[0].value, 200);

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
