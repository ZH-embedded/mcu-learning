/* p14_pattern.c —— 图形打印：嵌套循环的计数训练（参考实现）
 * 方法：不要试，先推出每行的「空格数」和「星号数」公式，再写循环
 */
#include <stdio.h>

static void print_ch(char ch, int n)
{
    int i;
    for (i = 0; i < n; i++) putchar(ch);
}

/* 直角三角形：第 r 行（r 从 1 开始）有 r 个星 */
static void triangle(int h)
{
    int r;
    for (r = 1; r <= h; r++) { print_ch('*', r); putchar('\n'); }
}

/* 倒三角：第 r 行有 h-r+1 个星 */
static void triangle_inv(int h)
{
    int r;
    for (r = 1; r <= h; r++) { print_ch('*', h - r + 1); putchar('\n'); }
}

/* 菱形：总高 2h-1，第 r 行（r 从 1 到 2h-1）
 * 空格数 = |h - r|，星数 = 2*(h - |h-r|) - 1 */
static void diamond(int h)
{
    int r;
    for (r = 1; r <= 2 * h - 1; r++) {
        int d = (r > h) ? (r - h) : (h - r);
        int stars = 2 * (h - d) - 1;
        print_ch(' ', d);
        print_ch('*', stars);
        putchar('\n');
    }
}

/* 空心菱形：每行只有第一个和最后一个位置打星 */
static void diamond_hollow(int h)
{
    int r;
    for (r = 1; r <= 2 * h - 1; r++) {
        int d = (r > h) ? (r - h) : (h - r);
        int stars = 2 * (h - d) - 1;
        int k;

        print_ch(' ', d);
        for (k = 0; k < stars; k++) {
            if (k == 0 || k == stars - 1) putchar('*');
            else                          putchar(' ');
        }
        putchar('\n');
    }
}

int main(void)
{
    printf("== 直角三角形（高 5）==\n");
    triangle(5);

    printf("\n== 倒三角（高 5）==\n");
    triangle_inv(5);

    printf("\n== 菱形（高 7）==\n");
    diamond(4);

    printf("\n== 空心菱形（高 7）==\n");
    diamond_hollow(4);

    printf("\n推导过程（以菱形为例，h=4）：\n");
    printf("  r     |h-r|   空格    星数=2*(h-|h-r|)-1\n");
    printf("  1     3       3       1\n");
    printf("  2     2       2       3\n");
    printf("  3     1       1       5\n");
    printf("  4     0       0       7     ← 最宽的一行\n");
    printf("  5     1       1       5\n");
    printf("把这张表推出来，循环就只是把表翻译成代码\n");

    return 0;
}
