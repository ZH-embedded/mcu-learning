/* p08_printf.c —— 格式化输出大全（参考实现）
 * %5.2f 的含义：总宽 5（不够左边补空格），小数点后 2 位
 */
#include <stdio.h>

int main(void)
{
    int      n = 42;
    int      neg = -42;
    double   f = 3.14159;
    unsigned u = 0xBEEFu;
    char     s[] = "MCU";
    int      x = 7;

    printf("== 整数 ==\n");
    printf("默认        |%d|\n", n);
    printf("宽 8 右对齐 |%8d|\n", n);
    printf("宽 8 左对齐 |%-8d|\n", n);
    printf("补零        |%08d|\n", n);
    printf("强制带符号  |%+d| |%+d|\n", n, neg);
    printf("宽度与精度  |%8.4d|\n", n);        /* 精度对整数表示“最少几位”，前面补 0 */
    printf("用 * 传宽度 |%*d|\n", 8, n);

    printf("\n== 浮点 ==\n");
    printf("默认        |%f|\n", f);
    printf("2 位小数    |%.2f|\n", f);
    printf("宽 8 精度 2 |%8.2f|\n", f);
    printf("左对齐      |%-8.2f|\n", f);
    printf("补零        |%08.2f|\n", f);
    printf("科学计数    |%e| |%.2E|\n", f, f);

    printf("\n== 进制 ==\n");
    printf("16 进制     |%x| |%X| |%%#x 带前缀 |%#x|\n", u, u, u);
    printf("8 进制      |%o| |%%#o 带前缀 |%#o|\n", u, u);

    printf("\n== 字符与字符串 ==\n");
    printf("字符        |%c|\n", 'A');
    printf("字符串      |%s| |%6s| |%-6s|\n", s, s, s);
    printf("精度截断    |%.2s|\n", s);

    printf("\n== 其他 ==\n");
    printf("百分号本身  |%%|\n");
    printf("指针        |%p|（必须强转 (void*)）\n", (void *)&x);
    printf("size_t      |%zu|\n", sizeof(x));

    printf("\n常见错误：用 %%d 打 float → 打出 0 或垃圾值；用 %%f 打 int → 结果不可预测\n");

    return 0;
}
