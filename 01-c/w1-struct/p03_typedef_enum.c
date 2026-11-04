/* p03_typedef_enum.c —— typedef 与 enum：给类型起名，给状态编号
 *
 * 这两个关键字都不产生任何机器码，纯粹是写给人和编译器看的。
 * 但它们在真实代码里的分量很重：
 *
 *   typedef  —— 把"长类型"收成一个名字。`unsigned char` → `u8`，
 *               结构体/函数指针更是必须用（不然每处都得写一长串）
 *   enum     —— 把"几个互斥的取值"收成一套有名字的常量，
 *               而不是散落在代码里的 0 / 1 / 2
 *
 * 为什么不用 #define 代替？
 *   ① #define 是文本替换，没有类型，传错地方编译器不吭声
 *   ② #define 不受作用域限制，调试器里也看不到名字
 *   ③ enum 的值自动递增，加一个状态不用手动编号
 */
#include <stdio.h>
#include <stddef.h>

/* 设备状态枚举。
 * 这就是嵌入式里最典型的状态机：空闲 → 运行 → 故障 → （复位后）空闲。
 *
 * STATE_COUNT 是个**哨兵**：它自己不是合法状态，只用来表示"有几个状态"，
 * 这样以后加状态，凡是用 STATE_COUNT 的地方（数组大小、边界判断）自动跟上。 */
typedef enum
{
    STATE_IDLE = 0,
    STATE_RUN,
    STATE_FAULT,
    STATE_COUNT
} dev_state_t;

/* 状态名表格：下标就是枚举值，直接 STATE_NAMES[s] 就能查到名字。
 * 这是 enum 最常用的两个套路之一（另一个是 switch）。 */
static const char *const STATE_NAMES[STATE_COUNT] = {
    "IDLE", "RUN", "FAULT"};

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
        printf("  [PASS] %-42s \"%s\"\n", name, got);
        g_pass++;
    }
    else
    {
        printf("  [FAIL] %-42s 期望 \"%s\"，实际 \"%s\"\n",
               name, want, got != NULL ? got : "(NULL)");
        g_fail++;
    }
}

/* ============================================================
 * TODO 1：状态转移
 *
 *   fault 非 0        → STATE_FAULT（故障优先，从任何状态都进故障）
 *   STATE_IDLE        → STATE_RUN
 *   STATE_RUN         → STATE_RUN（继续跑）
 *   STATE_FAULT       → STATE_FAULT（故障要人工复位，自己不会好）
 *   其它（非法值）    → STATE_IDLE（归一到一个安全状态）
 *
 * 用 switch 写，记得配一个 default。
 *
 * 为什么 default 重要：这个函数以后可能被喂进从 Flash 读出来的、
 * 被写坏过的状态值。没有 default 的 switch 遇到没列举的值，
 * 函数会执行到结尾而没有 return —— 那是有去无回的未定义行为。
 * ============================================================ */
dev_state_t next_state(dev_state_t s, int fault)
{
    /* 优先级最高的一条，先挡掉，不进 switch */
    if (fault != 0)
    {
        return STATE_FAULT;
    }

    switch (s)
    {
    case STATE_IDLE:
        return STATE_RUN;
    case STATE_RUN:
        return STATE_RUN;
    case STATE_FAULT:
        return STATE_FAULT;
    default:
        return STATE_IDLE; /* 非法值归一 */
    }
}

/* ============================================================
 * TODO 2：查状态名
 *
 * 直接 STATE_NAMES[s] 查表，不要写 if-else 链。
 * 但**必须先判 s 是否在 0 ~ STATE_COUNT-1 之间**，
 * 否则 STATE_NAMES[99] 就是越界读 —— 拿到的是一段野内存当字符串用。
 * 越界时返回字符串 "UNKNOWN"。
 * ============================================================ */
const char *state_name(dev_state_t s)
{
    if ((unsigned)s >= (unsigned)STATE_COUNT)
    {
        return "UNKNOWN";
    }

    return STATE_NAMES[s];
}

/* ============================================================
 * TODO 3：判断一个整数是不是合法状态
 *
 * 合法范围：0 <= s < STATE_COUNT。合法返回 1，否则返回 0。
 *
 * 注意形参是 **int** 而不是 dev_state_t —— 故意的。
 * 调用者手里那个值可能来自串口、Flash、或者被人强转过的 int，
 * 不能假设它一定是合法状态。用 int 接才诚实，也是在说：
 * **枚举在运行时就是个整数，没有任何保护**。
 * ============================================================ */
int state_is_valid(int s)
{
    if ((s >= 0) && (s < STATE_COUNT))
    {
        return 1;
    }
    return 0;
}

/* ============================================================
 * TODO 4：按名字反查状态（字符串转枚举）
 *
 * 遍历 STATE_NAMES，逐字符比较（不许用 strcmp / string.h）。
 * 找不到、或 name 为 NULL，返回 STATE_COUNT（哨兵值，非法状态）。
 *
 * 这个函数在真实项目里的用途：串口收到 "RUN" 这样的字符串，
 * 要转成代码里能 switch 的枚举值。
 *
 * 注意大小写敏感："run" 不等于 "RUN"，应该返回 STATE_COUNT。
 * ============================================================ */
dev_state_t state_from_name(const char *name)
{
    if (name == NULL)
    {
        return STATE_COUNT;
    }

    for (int i = 0; i < STATE_COUNT; i++)
    {
        int j = 0;
        while (STATE_NAMES[i][j] != '\0' && STATE_NAMES[i][j] == name[j])
        {
            j++;
        }
        if (STATE_NAMES[i][j] == '\0' && name[j] == '\0')
        {
            return (dev_state_t)i;
        }
    }
    return STATE_COUNT;
}

int main(void)
{
    printf("=== p03 typedef 与 enum ===\n\n");

    printf("枚举值：IDLE=%d  RUN=%d  FAULT=%d  COUNT=%d（哨兵）\n",
           STATE_IDLE, STATE_RUN, STATE_FAULT, STATE_COUNT);
    printf("sizeof(dev_state_t) = %zu（枚举本质就是个整数）\n\n",
           sizeof(dev_state_t));

    printf("--- 测试 ---\n");

    check_int("STATE_COUNT 自动递增到 3", STATE_COUNT, 3);

    check_int("next_state(IDLE, 无故障) → RUN",
              next_state(STATE_IDLE, 0), STATE_RUN);
    check_int("next_state(RUN, 无故障) → RUN",
              next_state(STATE_RUN, 0), STATE_RUN);
    check_int("next_state(IDLE, 有故障) → FAULT",
              next_state(STATE_IDLE, 1), STATE_FAULT);
    check_int("next_state(RUN, 有故障) → FAULT",
              next_state(STATE_RUN, 1), STATE_FAULT);
    check_int("next_state(FAULT, 无故障) → 保持 FAULT",
              next_state(STATE_FAULT, 0), STATE_FAULT);
    check_int("next_state(FAULT, 有故障) → 还是 FAULT",
              next_state(STATE_FAULT, 1), STATE_FAULT);
    check_int("next_state(非法值 99) → 归一到 IDLE",
              next_state((dev_state_t)99, 0), STATE_IDLE);

    check_str("state_name(IDLE)", state_name(STATE_IDLE), "IDLE");
    check_str("state_name(RUN)", state_name(STATE_RUN), "RUN");
    check_str("state_name(FAULT)", state_name(STATE_FAULT), "FAULT");
    check_str("state_name(99) 越界返回 UNKNOWN",
              state_name((dev_state_t)99), "UNKNOWN");

    check_int("state_is_valid(IDLE) = 1", state_is_valid(STATE_IDLE), 1);
    check_int("state_is_valid(FAULT) = 1", state_is_valid(STATE_FAULT), 1);
    check_int("state_is_valid(3) 哨兵不算合法", state_is_valid(STATE_COUNT), 0);
    check_int("state_is_valid(99) = 0", state_is_valid(99), 0);
    check_int("state_is_valid(-1) = 0", state_is_valid(-1), 0);

    check_int("state_from_name(\"RUN\") → STATE_RUN",
              state_from_name("RUN"), STATE_RUN);
    check_int("state_from_name(\"FAULT\") → STATE_FAULT",
              state_from_name("FAULT"), STATE_FAULT);
    check_int("state_from_name(\"run\") 大小写敏感 → STATE_COUNT",
              state_from_name("run"), STATE_COUNT);
    check_int("state_from_name(NULL) → STATE_COUNT",
              state_from_name(NULL), STATE_COUNT);

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
