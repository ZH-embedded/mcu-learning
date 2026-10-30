/* p12_switch.c —— switch 与穿透（参考实现）
 * 关键字：case 分组（有意的穿透）、漏 break（无意的穿透）
 */
#include <stdio.h>

/* 写法一：把分数折成十位再 switch，这是 switch 处理区间的标准技巧 */
static char grade_switch(int score)
{
    if (score < 0 || score > 100) return '?';

    switch (score / 10) {
    case 10:
    case 9:  return 'A';        /* 90-100：故意让 case 10 穿透到 case 9 */
    case 8:  return 'B';
    case 7:  return 'C';
    case 6:  return 'D';
    default: return 'E';        /* 0-59 */
    }
}

/* 写法二：漏 break 的 bug 版本。用赋值 + 末尾统一 return 才看得清后果 */
static char grade_no_break(int score)
{
    char g = '?';
    switch (score / 10) {
    case 10:
    case 9:  g = 'A';           /* 忘了 break，继续往下执行 */
    case 8:  g = 'B';
    case 7:  g = 'C';
    case 6:  g = 'D';
    default: g = 'E';
    }
    return g;
}

/* 写法三：有意的穿透做累积统计 —— 这时穿透是特性，不是 bug */
static int count_by_level(const int *scores, int n)
{
    int excellent = 0, pass = 0, fail = 0;
    int i;

    for (i = 0; i < n; i++) {
        int s = scores[i];
        if (s < 0 || s > 100) continue;

        switch (s / 10) {
        case 10:
        case 9:  excellent++;   /* 穿透：90 分以上同时计入优秀和及格 */
                 /* fall through */
        case 8:
        case 7:
        case 6:  pass++;        /* 60-89 只计入及格 */
                 break;
        default: fail++;
                 break;
        }
    }
    printf("\n统计：优秀 %d，及格(含优秀) %d，不及格 %d\n", excellent, pass, fail);
    return 0;
}

int main(void)
{
    const int scores[] = { 100, 95, 90, 89, 80, 75, 60, 59, 0, -5, 120 };
    int i, n = (int)(sizeof(scores) / sizeof(scores[0]));

    printf("%-8s %-10s %-10s %s\n", "分数", "正确写法", "漏 break", "是否一致");
    printf("----------------------------------------------------\n");
    for (i = 0; i < n; i++) {
        char a = grade_switch(scores[i]);
        char b = grade_no_break(scores[i]);
        printf("%-8d %-10c %-10c %s\n", scores[i], a, b, (a == b) ? "" : "← 不一致");
    }

    count_by_level(scores, n);

    printf("\n要点：\n");
    printf("1) switch 的表达式必须是整型，case 必须是常量，不能写区间\n");
    printf("2) 处理区间用 switch (x / 10)；区间不规则就别用 switch\n");
    printf("3) 有意穿透要写注释 + 显式标注，否则三个月后你也会当成 bug\n");
    printf("4) 漏 break 的后果：case 一路执行到底，值被最后那个 case 覆盖\n");

    return 0;
}
