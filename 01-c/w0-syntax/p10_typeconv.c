/* p10_typeconv.c —— 隐式转换与截断（参考实现）
 * 两个关键词：整型提升（promotion）、截断（truncation）
 */
#include <stdio.h>

int main(void)
{
    printf("== 一、整型提升：小于 int 的类型参与运算时先变成 int ==\n");
    {
        unsigned char c = 200;
        printf("unsigned char 200 + 100 = %d   （char 先提升成 int，所以不会在 8 位里回绕）\n",
               c + 100);
        printf("sizeof(c + 100) = %zu   （结果是 int，不是 char）\n", sizeof(c + 100));
        printf("强制截回 8 位：(unsigned char)(200 + 100) = %u   （300 - 256 = 44）\n",
               (unsigned char)(c + 100));
    }

    printf("\n== 二、截断：大类型赋值给小类型，只保留低位 ==\n");
    {
        int i = 300;
        char ch = (char)i;             /* 实现定义：本机取低 8 位，300 -> 44 */
        printf("int 300 -> char = %d   （300 = 0x12C，只留 0x2C = 44）\n", ch);

        int neg = -1;
        printf("int -1 -> unsigned char = %u   （0xFFFFFFFF 只留 0xFF = 255）\n",
               (unsigned char)neg);
    }

    printf("\n== 三、有符号与无符号互转：位模式不变，解释变了 ==\n");
    {
        unsigned int u = 4294967295u;
        printf("unsigned 4294967295 -> int = %d\n", (int)u);
        printf("int -1 -> unsigned         = %u\n", (unsigned)-1);
        printf("两者位模式都是 0xFFFFFFFF，只是最高位被解释成符号位或数值位\n");
    }

    printf("\n== 四、转浮点：int -> float 也可能丢精度 ==\n");
    {
        int big = 16777217;            /* 2^24 + 1 */
        float fb = (float)big;
        printf("int 16777217 -> float = %.1f   （float 尾数只有 24 位，末位被舍掉）\n",
               (double)fb);
        printf("double 能精确表示：%.1f\n", (double)big);
    }

    printf("\n== 五、混合运算的转换顺序 ==\n");
    {
        char  c = 10;
        int   i = -20;
        unsigned int u = 30;
        double d = 1.5;

        printf("c + i = %d      （char 提升为 int）\n", c + i);
        printf("i + u = %u      （int 转 unsigned，负数变巨大）\n", i + u);
        printf("i + d = %.2f    （int 转 double）\n", i + d);
        printf("sizeof(c+i)=%zu sizeof(i+u)=%zu sizeof(i+d)=%zu\n",
               sizeof(c + i), sizeof(i + u), sizeof(i + d));
    }

    printf("\n规则：\n");
    printf("1) 小于 int 的类型先提升为 int，再参与运算\n");
    printf("2) int 与 unsigned int 混算，统一转成 unsigned（这就是 -1<1u 为假的原因）\n");
    printf("3) 有浮点参与，统一转成浮点（float/double 中等级高的那个）\n");
    printf("4) 赋值/传参时发生截断，多出的高位直接丢弃，不会有任何提示\n");

    return 0;
}
