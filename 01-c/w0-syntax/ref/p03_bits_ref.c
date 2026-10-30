/* p03_bits.c —— 打印 8 位二进制 + 置位/清位/取反/读位（参考实现）
 * 关键点：操作必须真的改变 c，而不是“打印时把某一位画成想要的样子”
 */
#include <stdio.h>

/* 从高位到低位打印，中间插空格把高 4 位和低 4 位分开 */
void print_bits(unsigned char x)
{
    int i;
    for (i = 7; i >= 0; i--) {
        if (i == 3) putchar(' ');
        putchar(((x >> i) & 1) ? '1' : '0');
    }
    putchar('\n');
}

int main(void)
{
    const unsigned char vals[] = { 0x00, 0x01, 0x80, 0xA5, 0xFF };
    int i;

    /* 一、打印 8 位。0x01 和 0x80 专门用来验方向：
     * 用 0xA5 测不出方向错误，因为 10100101 是回文 */
    printf("== 一、打印 8 个二进制位 ==\n");
    for (i = 0; i < (int)(sizeof(vals) / sizeof(vals[0])); i++) {
        printf("0x%02X  ", vals[i]);
        print_bits(vals[i]);
    }

    /* 二、四个惯用法。连续做，每一步都建立在上一行的结果上 */
    printf("\n== 二、置位 / 清位 / 取反 第 n 位 ==\n");
    {
        unsigned char c = 0xA5;

        printf("初始      0x%02X  ", c); print_bits(c);

        c |= (1u << 3);                 /* 置位：或 1 得 1 */
        printf("置位 3    0x%02X  ", c); print_bits(c);

        c &= ~(1u << 7);                /* 清位：取反后与，只把目标位清 0 */
        printf("清位 7    0x%02X  ", c); print_bits(c);

        c ^= (1u << 0);                 /* 取反：异或 1 翻转 */
        printf("取反 0    0x%02X  ", c); print_bits(c);
    }

    /* 三、读第 n 位 */
    printf("\n== 三、读第 n 位（0xA5）==\n");
    {
        unsigned char c = 0xA5;
        int n;
        printf("0x%02X: ", c);
        for (n = 7; n >= 0; n--) printf("bit%d=%d ", n, (c >> n) & 1);
        putchar('\n');
    }

    /* 四、“操作了但值没变”也要看得见：A5 的最高位本来就是 1 */
    printf("\n== 四、置位一个本来就是 1 的位，值不变 ==\n");
    {
        unsigned char c = 0xA5;
        c |= (1u << 7);
        printf("A5 置位 7 后：0x%02X  ", c); print_bits(c);
        printf("（值没变是正常的，不要误判成程序没生效）\n");
    }

    return 0;
}
