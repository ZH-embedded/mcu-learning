/* p13_prime.c —— 素数判断与循环控制（参考实现）
 * 关键：只需要试到 i*i <= n，不用 sqrt，既快又不依赖 libm
 */
#include <stdio.h>

/* 带 break 的版本：找到因子立刻退出 */
static int is_prime(int n)
{
    int i;
    if (n < 2) return 0;               /* 0 和 1 不是素数 */
    if (n % 2 == 0) return n == 2;      /* 偶数里只有 2 是素数，先砍掉一半 */
    for (i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return 0;       /* 提前退出，不用跑完整个循环 */
    }
    return 1;
}

/* 不带 break 的版本：即使已经确定不是素数也跑完全程，用计数器看差别 */
static int is_prime_no_break(int n, int *iterations)
{
    int i, prime = 1;
    *iterations = 0;
    if (n < 2) return 0;
    for (i = 2; i * i <= n; i++) {
        (*iterations)++;
        if (n % i == 0) prime = 0;      /* 只记标记，不退出 */
    }
    return prime;
}

int main(void)
{
    int n, count = 0;

    printf("== 2-100 之间的素数 ==\n");
    for (n = 2; n <= 100; n++) {
        if (is_prime(n)) {
            printf("%3d ", n);
            if (++count % 10 == 0) putchar('\n');
        }
    }
    printf("\n共 %d 个\n", count);

    printf("\n== break 与不 break 的循环次数对比 ==\n");
    {
        const int probes[] = { 97, 91, 1000003, 1000005 };
        int i;
        printf("%-10s %-10s %-14s %s\n", "n", "是否素数", "带break次数", "不带break次数");
        printf("--------------------------------------------------\n");
        for (i = 0; i < (int)(sizeof(probes) / sizeof(probes[0])); i++) {
            int iter = 0;
            int a = is_prime_no_break(probes[i], &iter);

            /* 带 break 的版本，用同样的试除范围统计次数 */
            int j, iter2 = 0;
            for (j = 2; (long)j * j <= probes[i]; j++) {
                iter2++;
                if (probes[i] % j == 0) break;
            }
            printf("%-10d %-10s %-14d %d\n",
                   probes[i], a ? "是" : "否", iter2, iter);
        }
        printf("结论：合数用 break 能省掉大量循环；素数两者一样，因为本来就要跑完\n");
    }

    printf("\n边界值自测：\n");
    printf("  -3 → %s，0 → %s，1 → %s，2 → %s，4 → %s，9 → %s，25 → %s\n",
           is_prime(-3) ? "素数" : "非素数", is_prime(0) ? "素数" : "非素数",
           is_prime(1) ? "素数" : "非素数", is_prime(2) ? "素数" : "非素数",
           is_prime(4) ? "素数" : "非素数", is_prime(9) ? "素数" : "非素数",
           is_prime(25) ? "素数" : "非素数");

    return 0;
}
