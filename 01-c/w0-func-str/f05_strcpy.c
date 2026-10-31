/* f05_strcpy.c —— 手写 strcpy
 *
 * 题目：实现
 *     char *my_strcpy(char *dst, const char *src);
 *
 *   把 src（含结尾 '\0'）复制到 dst，返回 dst
 *
 * 约束：
 *   禁用 <string.h> 里的 strcpy
 *
 * 三个要弄明白的点：
 *   1) 为什么返回 dst 而不是 void？—— 为了链式调用，标准库就是这么设计的
 *   2) '\0' 必须一起复制，少拷这一个字节，后面每次打印都会读到垃圾
 *   3) 这个函数不知道 dst 有多大 → 缓冲区溢出的元凶之一。
 *      所以 11 月写串口接收要用带长度参数的版本（strncpy / memcpy），
 *      这也是为什么很多公司禁用 strcpy。
 */
#include <stdio.h>

/* ============================================================
 * TODO: 实现 my_strcpy
 * ============================================================ */
char *my_strcpy(char *dst, const char *src)
{
    int i = 0;
    while (src[i] != '\0')
    {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
    return dst;
}

/* ============================================================
 * 以下不用改
 * ============================================================ */
static int g_pass = 0, g_fail = 0;

/* 逐字节比较，遇到 '\0' 结束（不用 strcmp） */
static int same_str(const char *a, const char *b)
{
    while (*a != '\0' && *a == *b) { a++; b++; }
    return *a == *b;
}

static void check_copy(const char *name, const char *src, int expect_len)
{
    char buf[64];
    for (int i = 0; i < 64; i++) buf[i] = '#';   /* 填满哨兵，看有没有多写 */

    char *ret = my_strcpy(buf, src);

    int ok_content = same_str(buf, src);
    int ok_null    = (buf[expect_len] == '\0');          /* '\0' 位置对不对 */
    int ok_no_extra = (buf[expect_len + 1] == '#');      /* 有没有多写一个字节 */
    int ok_return  = (ret == buf);                       /* 返回值是不是 dst */

    if (ok_content && ok_null && ok_no_extra && ok_return) {
        printf("  [PASS] %-20s 复制 \"%s\"（%d 字节 + \\0）\n", name, src, expect_len);
        g_pass++;
    } else {
        printf("  [FAIL] %-20s", name);
        if (!ok_content)  printf(" 内容不对");
        if (!ok_null)     printf(" \\0 位置错了");
        if (!ok_no_extra) printf(" 多写了一个字节(溢出)");
        if (!ok_return)   printf(" 返回值不是 dst");
        printf("\n");
        g_fail++;
    }
}

int main(void)
{
    printf("=== my_strcpy 测试 ===\n");

    check_copy("普通字符串", "abc", 3);
    check_copy("空串",       "",    0);
    check_copy("刚好填满",   "12345", 5);

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);

    /* ---- 观察：返回值为什么要设计成 char* ---- */
    printf("\n=== 为什么返回 dst（链式调用）===\n");
    char a[32], b[32];
    if (my_strcpy(a, "hello") != 0) {
        printf("  my_strcpy(a, \"hello\") = \"%s\"\n", my_strcpy(a, "hello"));
        printf("  可以连着用：my_strcpy(b, my_strcpy(a, \"hi\")) → b = \"%s\"\n",
               my_strcpy(b, my_strcpy(a, "hi")));
    } else {
        printf("  （函数还没实现，先看不了）\n");
    }

    printf("\n一个真实教训：strcpy 不知道 dst 有多大，\n");
    printf("  别人传进来的 src 比你缓冲区长，就静默踩坏后面的内存，\n");
    printf("  程序不报错、继续跑，直到某个不相关的变量莫名其妙变值。\n");
    printf("  → 所以实际工程里要么用 strncpy(dst, src, sizeof(dst)-1)，\n");
    printf("    要么自己写一个带 size 参数的版本。\n");

    return g_fail == 0 ? 0 : 1;
}
