/* p05_menu.c —— 当天的硬产出：函数指针数组做命令菜单（注册表模式）
 *
 * 这是今天必须能"当面讲清"的那一段代码。
 *
 * 没有函数指针的时候，命令分发只能这么写：
 *     if (!strcmp(cmd, "led on"))  { led_on();  }
 *     else if (!strcmp(cmd, "led off")) { led_off(); }
 *     else if (...) { ... }
 * 加一条命令要改三个地方：函数、if 链、帮助文本。改漏一处就是 bug，
 * 而且这几个地方隔得很远，review 时根本看不出来。
 *
 * 注册表模式换了个思路：**把"名字、函数、说明"绑成一个结构体，
 * 所有命令排成一个数组**。分发逻辑只写一次（就是遍历这个数组），
 * 以后加命令 = 往数组里加一行，一行只在一个地方出现。
 *
 * 这个结构在真实代码里到处都是：
 *   RTOS 的 shell 命令表、AT 指令表、驱动 ops 结构体（file_operations 那种）、
 *   UART 协议的命令码分发表、菜单状态机。
 *
 * 表里三列分别是：
 *     name    命令行里打什么
 *     handler 打完之后调哪个函数
 *     help    给 help 命令看的说明
 */
#include <stdio.h>
#include <stddef.h>

typedef void (*cmd_fn)(void);

typedef struct
{
    const char *name;
    cmd_fn handler;
    const char *help;
} cmd_entry_t;

/* ============================================================
 * 三个具体命令（给你了，注意它们的类型必须和 cmd_fn 完全一致：
 *  void 返回值、void 参数。差一个 int 就不匹配，gcc 会警告，
 *  强制转换能骗过编译器，然后运行时炸 —— 别转）
 * ============================================================ */
static int g_on = 0, g_off = 0, g_status = 0, g_total = 0;

static void cmd_led_on(void)
{
    g_on++;
    g_total++;
}
static void cmd_led_off(void)
{
    g_off++;
    g_total++;
}
static void cmd_status(void)
{
    g_status++;
    g_total++;
}

/* ============================================================
 * 注册表本身（给你了）
 *
 * 想加一条命令？在下面加一行就行 —— 查找、执行、帮助全自动跟上。
 * 这就是这张表存在的全部理由。
 * ============================================================ */
static const cmd_entry_t MENU[] = {
    {"led on", cmd_led_on, "点亮 LED"},
    {"led off", cmd_led_off, "熄灭 LED"},
    {"status", cmd_status, "打印状态"},
};

/* 表的长度交给 sizeof 算，以后加行不用改这里（这就是"数组不退化"的用法） */
#define MENU_N ((int)(sizeof(MENU) / sizeof(MENU[0])))

/* ============================================================
 * TODO 1：按名字查表，返回指向那一条的指针
 *
 * 要求：
 *   - table 或 name 为 NULL、n <= 0 → 返回 NULL
 *   - 逐个比较 table[i].name 和 name，相等就 return &table[i]
 *   - 全部不匹配 → NULL
 *
 * 字符串比较不许用 strcmp / string.h，自己写个循环：
 * 逐字符比，直到有一个 '\0' 为止。
 * 别忘了：两个字符串都要先判 NULL，否则一开始解引用就崩。
 *
 * 返回值是 const 指针，因为 MENU 本身是 const 的 —— 谁也别想改这张表。
 * ============================================================ */
const cmd_entry_t *find_command(const cmd_entry_t *table, int n,
                                const char *name)
{
    if (table == NULL || name == NULL || n <= 0)
    {
        return NULL;
    }

    for (int i = 0; i < n; i++)
    {
        int flag = 1; // 标志位
        if (table[i].name == NULL)
        {
            continue;
        }

        int j = 0;
        while ((*(table[i].name + j) != '\0') || (name[j] != '\0'))
        {
            if (*(table[i].name + j) != name[j])
            {
                flag = 0;
                break;
            }
            j++;
        }
        if (flag == 1)
            return &table[i];
    }
    return NULL;
}

/* ============================================================
 * TODO 2：执行一条命令（今天的核心）
 *
 * 要求：
 *   - 先 find_command
 *   - 没找到 → 返回 -1，**什么函数都不许调**
 *   - 找到了 → 调用 p->handler()，返回 0
 *
 * 这一题的验收点不是"能跑"，是"打错命令时绝不能误调别的函数"。
 * 你想想一个真实的电机控制板：用户打错一个字，把 "motor stop" 糊成
 * "motor stro"，如果分发逻辑写岔了，那就不是报错，是事故。
 *
 * handler 虽然理论上有值（表是静态的），但写成 if (p->handler) 更稳 ——
 * 以后有人往表里塞一个 NULL 表示"这条先占个位"，你就不会崩。
 * ============================================================ */
int run_command(const cmd_entry_t *table, int n, const char *name)
{
    const cmd_entry_t *p = find_command(table, n, name);
    if (p == NULL || p->handler == NULL)
    {
        return -1;
    }

    p->handler();
    return 0;
}

/* ============================================================
 * TODO 3：依次执行表里所有命令
 *
 * 要求：table 为 NULL 或 n <= 0 直接 return；否则按顺序调用每一条的 handler。
 *
 * 用途：上电自检、回归测试、把所有命令跑一遍看会不会崩。
 * 注意这里**不经过名字查找**，直接用下标 —— 表和遍历是同一份数据，
 * 这就是"注册表"的另一个好处：批量操作天然支持。
 * ============================================================ */
void run_all(const cmd_entry_t *table, int n)
{
    if (table == NULL || n <= 0)
    {
        return;
    }

    for (int i = 0; i < n; i++)
    {
        table[i].handler();
    }
}

/* ============================================================
 * TODO 4：查一条命令的帮助文本
 *
 * 要求：找到就返回那条的 help 指针；找不到返回 NULL。
 * 直接复用 TODO 1，不要再写一遍循环（复制粘贴的循环迟早会改漏一处）。
 * ============================================================ */
const char *help_of(const cmd_entry_t *table, int n, const char *name)
{

    const cmd_entry_t *p = find_command(table, n, name);
    return (p != NULL) ? p->help : NULL;
}

/* ============================================================
 * 以下不用改
 * ============================================================ */
static int g_pass = 0, g_fail = 0;

static void check_int(const char *name, int got, int want)
{
    if (got == want)
    {
        printf("  [PASS] %-38s = %d\n", name, got);
        g_pass++;
    }
    else
    {
        printf("  [FAIL] %-38s 期望 %d，实际 %d\n", name, want, got);
        g_fail++;
    }
}

static void check_ptr(const char *name, const void *got, const void *want)
{
    if (got == want)
    {
        printf("  [PASS] %-38s 指向同一条\n", name);
        g_pass++;
    }
    else if (got == NULL)
    {
        printf("  [FAIL] %-38s 返回 NULL\n", name);
        g_fail++;
    }
    else
    {
        printf("  [FAIL] %-38s 地址不同：%p vs %p\n", name, got, want);
        g_fail++;
    }
}

static void reset_counts(void)
{
    g_on = g_off = g_status = g_total = 0;
}

int main(void)
{
    printf("=== p05 函数指针数组：命令菜单 ===\n\n");

    printf("MENU_N = %d 条命令   ← sizeof 算出来的，加一行就自动 +1\n\n",
           MENU_N);

    printf("--- 测试 ---\n");

    /* ---- 查表 ---- */
    const cmd_entry_t *p = find_command(MENU, MENU_N, "status");
    check_ptr("find_command(\"status\")", p, &MENU[2]);
    if (p != NULL)
    {
        check_int("查到的那条 handler 是 cmd_status",
                  p->handler == cmd_status ? 1 : 0, 1);
    }
    else
    {
        printf("  [FAIL] %-38s handler 无法比较（上一条已失败）\n",
               "查到的那条 handler 是 cmd_status");
        g_fail++;
    }
    check_ptr("find_command(\"reboot\") 返回 NULL",
              find_command(MENU, MENU_N, "reboot"), NULL);
    check_ptr("find_command(NULL 名字) 返回 NULL",
              find_command(MENU, MENU_N, NULL), NULL);
    check_ptr("find_command(n=0) 返回 NULL",
              find_command(MENU, 0, "status"), NULL);

    /* ---- 执行：正常的 ---- */
    reset_counts();
    check_int("run_command(\"led on\") 返回 0", run_command(MENU, MENU_N, "led on"), 0);
    check_int("  → g_on 加了 1", g_on, 1);
    check_int("  → 别的命令没被误调", g_off + g_status, 0);

    check_int("再执行一次 led on", run_command(MENU, MENU_N, "led on"), 0);
    check_int("  → g_on = 2", g_on, 2);

    check_int("run_command(\"led off\")", run_command(MENU, MENU_N, "led off"), 0);
    check_int("  → g_off 加了 1", g_off, 1);
    check_int("  → g_on 没变", g_on, 2);

    /* ---- 执行：找不到的（这一组最重要） ---- */
    reset_counts();
    check_int("run_command(\"reboot\") 返回 -1",
              run_command(MENU, MENU_N, "reboot"), -1);
    check_int("  → 一个 handler 都没被调", g_total, 0);

    check_int("run_command 名字为 NULL 返回 -1",
              run_command(MENU, MENU_N, NULL), -1);
    check_int("run_command n=0 返回 -1", run_command(MENU, 0, "led on"), -1);
    check_int("run_command table=NULL 返回 -1",
              run_command(NULL, MENU_N, "led on"), -1);
    check_int("  → 全程没有 handler 被调", g_total, 0);

    /* ---- 全跑一遍 ---- */
    reset_counts();
    run_all(MENU, MENU_N);
    check_int("run_all 后 3 个命令都执行了", g_total, 3);
    check_int("  → g_on", g_on, 1);
    check_int("  → g_off", g_off, 1);
    check_int("  → g_status", g_status, 1);

    reset_counts();
    run_all(NULL, MENU_N);
    run_all(MENU, 0);
    check_int("run_all 空表不崩、不执行", g_total, 0);

    /* ---- 帮助 ---- */
    check_ptr("help_of(\"led on\") 返回表里那条的 help",
              (const void *)help_of(MENU, MENU_N, "led on"),
              (const void *)MENU[0].help);
    check_ptr("help_of(\"led off\") 返回表里那条的 help",
              (const void *)help_of(MENU, MENU_N, "led off"),
              (const void *)MENU[1].help);
    check_ptr("help_of(\"reboot\") 返回 NULL",
              (const void *)help_of(MENU, MENU_N, "reboot"), NULL);

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
