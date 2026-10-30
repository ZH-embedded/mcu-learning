/* p11_leap.c —— 闰年判断，两种写法（参考实现）
 * 规则：能被 4 整除且不能被 100 整除，或者能被 400 整除
 */
#include <stdio.h>

/* 写法一：一行合并，逻辑运算 && || 的短路特性在这里很重要 */
static int leap_oneline(int y)
{
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

/* 写法二：分层 if，可读性更好，也更容易加日志和断点 */
static int leap_nested(int y)
{
    if (y % 400 == 0) return 1;        /* 400 年一闰，优先级最高，先拦掉 */
    if (y % 100 == 0) return 0;        /* 整百年不闰 */
    if (y % 4 == 0)   return 1;        /* 4 年一闰 */
    return 0;
}

int main(void)
{
    const int years[] = { 1900, 1999, 2000, 2004, 2023, 2024, 2100, 2400 };
    int i;

    printf("%-8s %-8s %-8s %s\n", "年份", "一行式", "分层式", "说明");
    printf("--------------------------------------------------\n");
    for (i = 0; i < (int)(sizeof(years) / sizeof(years[0])); i++) {
        int y = years[i];
        int a = leap_oneline(y);
        int b = leap_nested(y);
        const char *note = "";
        if (y % 400 == 0)      note = "整百年且能被 400 整除";
        else if (y % 100 == 0) note = "整百年但不能被 400 整除";
        else if (y % 4 == 0)   note = "普通闰年";

        printf("%-8d %-8s %-8s %s\n", y, a ? "是" : "否", b ? "是" : "否", note);
        if (a != b) printf("  ⚠ 两种写法结果不一致！\n");
    }

    printf("\n要点：\n");
    printf("1) 1900 不是闰年（整百年），2000 是（能被 400 整除），这两个是必测的边界\n");
    printf("2) 分层写法把条件按“稀有程度”排序，早期返回，后面不用再判断\n");
    printf("3) 一行式依赖 && 的短路：y%%4!=0 时后面根本不求值\n");

    return 0;
}
