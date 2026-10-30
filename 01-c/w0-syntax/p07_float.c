/* p07_float.c —— 浮点精度（参考实现）
 * 核心：二进制表示不了 0.1，所以 0.1 + 0.2 != 0.3；判断浮点相等必须用误差范围
 */
#include <stdio.h>

/* 自己实现“取绝对值”，避免引入 math.h 和 -lm */
static double dabs(double x) { return (x < 0) ? -x : x; }

static int approx_eq(double a, double b, double eps)
{
    return dabs(a - b) < eps;
}

int main(void)
{
    double a = 0.1;
    double b = 0.2;

    printf("== 一、看起来相等，实际不等 ==\n");
    printf("0.1 + 0.2      = %.20f\n", a + b);
    printf("0.3            = %.20f\n", 0.3);
    printf("0.1 + 0.2 - 0.3 = %.20e\n", (a + b) - 0.3);
    printf("(a + b) == 0.3 = %d   ← 0 表示假\n", (a + b) == 0.3);
    printf("近似比较       = %d   ← 1 表示真（误差 < 1e-9）\n",
           approx_eq(a + b, 0.3, 1e-9));

    printf("\n== 二、float 比 double 更早出错 ==\n");
    {
        float fa = 0.1f;
        float fb = 0.2f;
        printf("0.1f          = %.20f\n", (double)fa);
        printf("0.1f + 0.2f   = %.20f\n", (double)(fa + fb));
        printf("(fa + fb) == 0.3f ? %d\n", (fa + fb) == 0.3f);
        printf("sizeof(float) = %zu, sizeof(double) = %zu\n",
               sizeof(float), sizeof(double));
    }

    printf("\n== 三、有效数字：float 约 7 位，double 约 16 位 ==\n");
    printf("float  1.0f/3.0f = %.12f\n", (double)(1.0f / 3.0f));
    printf("double 1.0/3.0   = %.16f\n", 1.0 / 3.0);

    printf("\n== 四、累加一万个 0.1 的漂移 ==\n");
    {
        double sum_d = 0.0;
        float  sum_f = 0.0f;
        int i;
        for (i = 0; i < 10000; i++) {
            sum_d += 0.1;
            sum_f += 0.1f;
        }
        printf("double 累加 = %.10f  （误差 %.3e）\n", sum_d, dabs(sum_d - 1000.0));
        printf("float  累加 = %.10f  （误差 %.3e）\n", (double)sum_f,
               dabs((double)sum_f - 1000.0));
    }

    printf("\n结论：\n");
    printf("1) 浮点数不能用 == 比较，用 |a-b| < eps\n");
    printf("2) eps 要跟量级匹配：量级 1e6 时用 1e-6 做阈值等于没做\n");
    printf("3) STM32F103 没有 FPU，float 比 double 快得多，统一用 float + f 后缀\n");

    return 0;
}
