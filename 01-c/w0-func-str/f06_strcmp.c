/* f06_strcmp.c —— 手写 strcmp
 *
 * 题目：实现
 *     int my_strcmp(const char *a, const char *b);
 *
 *   相等返回 0
 *   a < b 返回负数
 *   a > b 返回正数
 *
 * 约束：
 *   禁用 <string.h> 里的 strcmp
 *
 * 最重要的一条规矩：
 *   C 标准只规定「符号」，不规定具体数值。
 *   所以判断永远写 if (my_strcmp(x, y) == 0) 或 < 0，
 *   绝不要写 == -1 —— PC 上（glibc）可能恰好对，换到板子上（newlib）就错。
 *   这是面试常问的"标准库实现差异"。
 *
 * 「谁大谁小」是按 ASCII 码逐字节比，不是按长度：
 *   "abc" vs "abd"  → 'c'(99) < 'd'(100) → 前者小
 *   "abc" vs "abcd" → 比到第 4 个字符，'\0'(0) < 'd'(100) → 前者小
 *   "A"   vs "a"    → 65 < 97 → 前者小（大写字母全部小于小写字母）
 */
#include <stdio.h>

/* ============================================================
 * TODO: 实现 my_strcmp
 * ============================================================ */
int my_strcmp(const char *a, const char *b)
{
    (void)a; (void)b;
    return 999;     /* 999 = 还没实现（合法返回值，但符号不对，测试会抓到） */
}

/* ============================================================
 * 以下不用改
 * ============================================================ */
static int g_pass = 0, g_fail = 0;

/* expect: -1 表示期望负数，0 表示期望 0，1 表示期望正数 */
static void check_cmp(const char *name, const char *a, const char *b, int expect)
{
    int r = my_strcmp(a, b);
    int sign = (r > 0) - (r < 0);      /* 只取符号：-1 / 0 / 1 */

    if (sign == expect) {
        printf("  [PASS] %-26s → %d\n", name, r);
        g_pass++;
    } else {
        const char *want = expect == 0 ? "0" : (expect < 0 ? "负数" : "正数");
        printf("  [FAIL] %-26s 期望%s，实际 %d\n", name, want, r);
        g_fail++;
    }
}

int main(void)
{
    printf("=== my_strcmp 测试（只看符号，不看具体数值）===\n");

    check_cmp("\"abc\" vs \"abc\"",  "abc",  "abc",   0);
    check_cmp("\"abc\" vs \"abd\"",  "abc",  "abd",  -1);   /* 'c' < 'd' */
    check_cmp("\"abd\" vs \"abc\"",  "abd",  "abc",   1);
    check_cmp("\"abc\" vs \"abcd\"", "abc",  "abcd", -1);   /* '\0' < 'd' */
    check_cmp("\"abcd\" vs \"abc\"", "abcd", "abc",   1);
    check_cmp("\"\" vs \"\"",        "",     "",      0);
    check_cmp("\"\" vs \"a\"",       "",     "a",    -1);
    check_cmp("\"A\" vs \"a\"",      "A",    "a",    -1);   /* 65 < 97 */
    check_cmp("\"Z\" vs \"a\"",      "Z",    "a",    -1);   /* 90 < 97，大写全小于小写 */

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);

    printf("\n=== 三个值得记的结论 ===\n");
    printf("  1. 大小写敏感：\"Z\" < \"a\"，因为比的是 ASCII 码不是字母表顺序\n");
    printf("  2. 短串不一定小：\"abc\" vs \"abcd\" 前面全一样，比到 '\\0' 才分出大小\n");
    printf("  3. 返回值别当数字用：标准只说符号，实现可以返回-1/1，也可以返回字符差值\n");

    return g_fail == 0 ? 0 : 1;
}
