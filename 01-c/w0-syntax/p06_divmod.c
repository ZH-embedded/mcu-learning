/* p06_divmod.c —— 整数除法与取模的符号规则（参考实现）
 * 结论（C99 起）：除法向 0 截断，余数的符号跟随被除数
 */
#include <stdio.h>

int main(void)
{
    const int pairs[][2] = {
        { 7,  2 },
        { -7, 2 },
        { 7, -2 },
        { -7, -2 },
        { 1,  2 },
        { -1, 2 },
    };
    int i;

    printf("%-6s %-6s %-8s %-8s %s\n", "a", "b", "a/b", "a%b", "a == (a/b)*b + a%b ?");
    printf("--------------------------------------------------------\n");

    for (i = 0; i < (int)(sizeof(pairs) / sizeof(pairs[0])); i++) {
        int a = pairs[i][0];
        int b = pairs[i][1];
        int q = a / b;
        int r = a % b;

        printf("%-6d %-6d %-8d %-8d %s\n",
               a, b, q, r, (a == q * b + r) ? "成立" : "不成立");
    }

    printf("\n结论：\n");
    printf("1) 除法向 0 截断：-7/2 = -3（不是 -4，也不是 -3.5 取整到 -4）\n");
    printf("2) 余数符号跟随被除数：-7%%2 = -1，7%%-2 = 1\n");
    printf("3) 恒等式 a == (a/b)*b + a%%b 永远成立，这是取模的定义\n");

    printf("\n浮点除法对照：-7.0/2 = %.2f（这里才出现小数）\n", -7.0 / 2);

    return 0;
}
