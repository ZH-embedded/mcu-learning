/* f04_strlen.c —— 手写 strlen
 *
 * 题目：实现
 *     size_t my_strlen(const char *s);
 *
 *   返回字符串长度（不含结尾的 '\0'）
 *
 * 约束：
 *   禁用 <string.h> 里的 strlen
 *   （可以 include 它，但别调用 strlen，也别用 strlen 来验证你自己）
 *
 * 关键认知：
 *   sizeof(s)  是"这个变量占几个字节"，编译期就定了
 *   my_strlen(s) 是"从 s 开始数，数到 \0 为止"，运行期才知道
 *   对 char 数组，sizeof = 长度 + 1（多了那个 '\0'）
 *   对 char *指针，sizeof = 8（指针大小），跟字符串多长毫无关系
 */
#include <stdio.h>
#include <stddef.h>     /* size_t */

/* ============================================================
 * TODO: 实现 my_strlen
 * ============================================================ */
size_t my_strlen(const char *s)
{
    int i = 0;
    size_t len = 0;
    while (s[i] != '\0')
    {
        i++;
        len++;
    }
    return len;
}

/* ============================================================
 * 以下不用改
 * ============================================================ */
static int g_pass = 0, g_fail = 0;

static void check_len(const char *name, const char *s, size_t expect)
{
    size_t actual = my_strlen(s);
    if (actual == expect) {
        printf("  [PASS] %-22s = %zu\n", name, actual);
        g_pass++;
    } else {
        printf("  [FAIL] %-22s 期望 %zu，实际 %zu\n", name, expect, actual);
        g_fail++;
    }
}

int main(void)
{
    printf("=== my_strlen 测试 ===\n");

    check_len("空串 \"\"",        "",      0);
    check_len("\"a\"",            "a",     1);
    check_len("\"hello\"",        "hello", 5);
    check_len("\"a b c\" (含空格)", "a b c", 5);

    /* 这一条是重点：UTF-8 下一个汉字占 3 字节
     * "中文" 是两个汉字 → 6 字节，不是 2 个字符 */
    check_len("\"中文\" (UTF-8)", "中文",  6);

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);

    /* ---- 下面是观察，不做 PASS/FAIL ---- */
    printf("\n=== sizeof vs strlen（这一块是今天的核心）===\n");
    {
        char arr[]  = "abc";     /* 4 个字节：a b c \0 */
        char *ptr   = "abc";     /* 指针本身 8 字节，指向的字符串在只读区 */
        char buf[32] = "abc";    /* 32 个字节，只有前 4 个有意义，后面是 0 */

        printf("  char arr[]   = \"abc\"  sizeof = %2zu   strlen = %zu\n",
               sizeof(arr),  my_strlen(arr));
        printf("  char *ptr    = \"abc\"  sizeof = %2zu   strlen = %zu\n",
               sizeof(ptr),  my_strlen(ptr));
        printf("  char buf[32] = \"abc\"  sizeof = %2zu   strlen = %zu\n",
               sizeof(buf),  my_strlen(buf));
        printf("\n  看懂这三行，就明白为什么函数里必须用 strlen 而不是 sizeof。\n");
        printf("  再想一层：为什么 my_strlen 的参数写成 const char *s，而不是 char s[]？\n");
        printf("  （答案：写 char s[] 也对，但它在函数里已经退化成指针了，sizeof(s) 还是 8）\n");
    }

    return g_fail == 0 ? 0 : 1;
}
