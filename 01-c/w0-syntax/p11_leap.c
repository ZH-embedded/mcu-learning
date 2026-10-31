/* p11_leap.c —— 闰年判断，两种写法（参考实现）
 * 规则：能被 4 整除且不能被 100 整除，或者能被 400 整除
 */
#include <stdio.h>

/* 写法一：一行合并 */
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
    
    int year = 0;
    printf("请输入年份\n");
    scanf("%d", &year);

//    int flag = leap_oneline(year);
    int flag = leap_nested(year);
    if(flag == 1)
    {
        printf("%d是闰年\n",year);
    }
    else
    {
        printf("%d不是闰年\n",year);
    }

    return 0;
}
