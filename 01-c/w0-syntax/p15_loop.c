/* p15_loop.c —— for / while / do-while 三种循环（参考实现）
 * 重点：do-while 至少执行一次；while 里的 continue 容易变成死循环
 */
#include <stdio.h>

int main(void)
{
    printf("== 一、同一件事写三遍：求 1..100 的和 ==\n");
    {
        int i, sum;

        for (sum = 0, i = 1; i <= 100; i++) sum += i;
        printf("for      : %d\n", sum);

        sum = 0; i = 1;
        while (i <= 100) { sum += i; i++; }
        printf("while    : %d\n", sum);

        sum = 0; i = 1;
        do { sum += i; i++; } while (i <= 100);
        printf("do-while : %d\n", sum);
    }

    printf("\n== 二、遇到第一个能被 7 整除的数就停（for + break）==\n");
    {
        int i;
        for (i = 1; i <= 100; i++) {
            if (i % 7 == 0) { printf("第一个：%d\n", i); break; }
        }
    }

    printf("\n== 三、do-while 和 while 的关键差别 ==\n");
    {
        int n = 0;                       /* 条件一开始就是假 */

        while (n > 0) {
            printf("while 循环体执行了（不该看到这行）\n");
        }
        printf("while    ：条件一开始为假 → 循环体一次都没执行\n");

        do {
            printf("do-while ：循环体执行了 1 次\n");
        } while (n > 0);
        printf("           因为 do-while 先执行后判断\n");
    }

    printf("\n== 四、continue 陷阱：while 里忘记自增就是死循环 ==\n");
    {
        int i = 0, guard = 0;

        printf("错误写法（用 guard 保护，只演示 6 次）：\n");
        while (i < 5) {
            if (i % 2 == 0) {
                /* 这里本来该有 i++，漏了 → i 永远停在 0 */
                printf("  卡在 i=%d\n", i);
                if (++guard > 5) { printf("  ... 真实代码里会无限循环\n"); break; }
                continue;
            }
            i++;
        }

        printf("正确写法：把自增放到 continue 之前，或者改用 for\n");
        {
            int j;
            for (j = 0; j < 5; j++) {
                if (j % 2 == 0) continue;   /* for 的自增在循环头，continue 不会跳过它 */
                printf("  奇数：%d\n", j);
            }
        }
    }

    printf("\n选择规则：\n");
    printf("1) 次数确定 → for；次数不确定但可能一次都不做 → while；至少做一次 → do-while\n");
    printf("2) do-while 的典型场景：先采样再判断、菜单至少显示一次、重试逻辑\n");
    printf("3) while + continue 时，自增一定要在 continue 之前，否则死循环\n");

    return 0;
}
