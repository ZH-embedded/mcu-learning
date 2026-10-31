/* f03_calc.c —— 四则运算计算器
 *
 * 题目：实现
 *     int calc(int a, char op, int b, int *out);
 *
 *   支持 + - * / % 五种运算
 *   成功返回 0，结果写进 *out
 *   除数为 0      → 返回 1
 *   未知运算符    → 返回 2
 *
 * 为什么用返回值报错，而不是 printf 一下完事：
 *   这是嵌入式代码的标准写法。函数只负责算，报错交给调用者决定
 *   （上层可能是打印、可能是重发、可能是点亮一个错误灯）。
 *   12 月写 I2C/Modbus 驱动时全是这个模式：返回状态码 + 指针带出数据。
 *
 * 除法的两个细节：
 *   -7 / 2  = -3   不是 -4   （向零截断，不是向下取整）
 *   -7 % 2  = -1   符号跟被除数走，不是跟除数
 */
#include <stdio.h>

/* ============================================================
 * TODO: 实现 calc
 * ============================================================ */
int calc(int a, char op, int b, int *out)
{
    switch (op)
    {
    case '+':
        *out = a + b;
        return 0;
    case '-':
        *out = a - b;
        return 0;
    case '*':
        *out = a * b;
        return 0;
    case '/':
        if (b == 0)
        {
            return 1;
        }
        else
        {
            *out = a / b;
            return 0;    
        }
    case '%':
        if (b == 0)
        {
            return 1;
        }
        else
        {
            *out = a % b;
            return 0;    
        }        
    default:
        return 2;
    }
}

/* ============================================================
 * 以下不用改
 * ============================================================ */
static int g_pass = 0, g_fail = 0;

static void check_op(const char *name, int a, char op, int b,
                     int expect_ret, int expect_out)
{
    int out = 0x7FFFFFFF;          /* 故意填一个不可能的值，看函数有没有写进去 */
    int ret = calc(a, op, b, &out);

    if (ret != expect_ret) {
        printf("  [FAIL] %-18s 返回码期望 %d，实际 %d\n", name, expect_ret, ret);
        g_fail++;
        return;
    }
    if (ret != 0) {                /* 出错时不该改 *out，也就没什么可比的 */
        printf("  [PASS] %-18s 正确报错（返回 %d）\n", name, ret);
        g_pass++;
        return;
    }
    if (out == expect_out) {
        printf("  [PASS] %-18s = %d\n", name, out);
        g_pass++;
    } else {
        printf("  [FAIL] %-18s 期望 %d，实际 %d\n", name, expect_out, out);
        g_fail++;
    }
}

int main(void)
{
    printf("=== 计算器测试 ===\n");

    check_op("12 + 5",  12, '+', 5, 0, 17);
    check_op("12 - 5",  12, '-', 5, 0, 7);
    check_op("12 * 5",  12, '*', 5, 0, 60);
    check_op("12 / 5",  12, '/', 5, 0, 2);     /* 整数除法，2 不是 2.4 */
    check_op("12 % 5",  12, '%', 5, 0, 2);

    check_op("-7 / 2",  -7, '/', 2, 0, -3);    /* 向零截断，不是 -4 */
    check_op("-7 % 2",  -7, '%', 2, 0, -1);    /* 符号跟被除数 */
    check_op("7 / -2",   7, '/', -2, 0, -3);
    check_op("7 % -2",   7, '%', -2, 0, 1);

    check_op("12 / 0",  12, '/', 0, 1, 0);     /* 除零 → 返回 1 */
    check_op("12 % 0",  12, '%', 0, 1, 0);
    check_op("12 ^ 5",  12, '^', 5, 2, 0);     /* 未知运算符 → 返回 2 */

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);

    /* 想手动玩：把下面的 0 改成 1，重新编译，就能敲 "12 + 5" 回车看结果 */
#if 0
    printf("\n进入交互模式（Ctrl+C 退出）\n");
    int a, b;
    char op;
    while (scanf("%d %c %d", &a, &op, &b) == 3) {
        int out;
        int r = calc(a, op, b, &out);
        if (r == 0)      printf("= %d\n", out);
        else if (r == 1) printf("错误：除数为 0\n");
        else             printf("错误：不支持的运算符 '%c'\n", op);
    }
#endif

    return g_fail == 0 ? 0 : 1;
}
