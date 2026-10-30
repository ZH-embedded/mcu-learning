/* p02_base.c —— 进制转换（参考实现）
 * 要求：十进制转 2/8/16 进制，不许用 %o %x；再写 16 进制字符串转十进制
 * 核心：取余得最低位，整除右移一位，最后倒序输出
 */
#include <stdio.h>

static const char DIGIT[] = "0123456789ABCDEF";

/* 把 v 转成 base 进制的字符串，buf 至少要 33 字节。
 * 返回 buf，方便直接 printf("%s") */
char *to_base(unsigned int v, int base, char *buf)
{
    char tmp[33];
    int n = 0, i = 0;

    if (v == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }

    while (v > 0) {
        tmp[n++] = DIGIT[v % base];    /* 取出当前最低位 */
        v /= base;                     /* 丢掉刚取走的那一位 */
    }
    while (n > 0) buf[i++] = tmp[--n]; /* 倒序拷回，才是正确的阅读顺序 */
    buf[i] = '\0';
    return buf;
}

/* 16 进制字符串 → 无符号整数，支持 0x 前缀与大小写 */
unsigned int hex_to_u(const char *s)
{
    unsigned int v = 0;

    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) s += 2;

    for (; *s != '\0'; s++) {
        unsigned int d;
        if      (*s >= '0' && *s <= '9') d = (unsigned int)(*s - '0');
        else if (*s >= 'a' && *s <= 'f') d = (unsigned int)(*s - 'a' + 10);
        else if (*s >= 'A' && *s <= 'F') d = (unsigned int)(*s - 'A' + 10);
        else break;                    /* 遇到非法字符就停 */
        v = v * 16 + d;                /* 每读一位，前面的值整体左移 4 bit */
    }
    return v;
}

int main(void)
{
    const unsigned int vals[] = { 0, 1, 10, 255, 4095, 65535 };
    char b2[33], b8[16], b16[16];
    int i, n = (int)(sizeof(vals) / sizeof(vals[0]));

    printf("%-8s %-18s %-8s %-6s\n", "DEC", "BIN", "OCT", "HEX");
    printf("--------------------------------------------------\n");
    for (i = 0; i < n; i++) {
        printf("%-8u %-18s %-8s %-6s\n",
               vals[i],
               to_base(vals[i], 2,  b2),
               to_base(vals[i], 8,  b8),
               to_base(vals[i], 16, b16));
    }

    printf("\n16 进制字符串转回十进制：\n");
    printf("  \"FF\"    = %u\n", hex_to_u("FF"));
    printf("  \"0xA5\"  = %u\n", hex_to_u("0xA5"));
    printf("  \"0xFFF\" = %u\n", hex_to_u("0xFFF"));
    printf("  \"0x1f\"  = %u\n", hex_to_u("0x1f"));

    return 0;
}
