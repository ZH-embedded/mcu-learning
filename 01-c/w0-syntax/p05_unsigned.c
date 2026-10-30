/* p05_unsigned.c —— 无符号的三个反直觉现象（参考实现）
 *
 * 总规则：表达式里只要有一个操作数是“无符号”，整个运算就在无符号域里做，
 *         结果按 2^N 取模回绕。这是标准规定的行为（不是 UB），所以编译器
 *         不会警告你 —— 只能靠脑子里有这条规则。
 */
#include <stdio.h>

int main(void)
{
    printf("== 现象一：无符号减法下溢 ==\n");
    {
        unsigned int a = 0;
        unsigned int b = 5;

        printf("a = 0, b = 5\n");
        printf("a - b          = %u   （预期：一个巨大的数）\n", a - b);
        printf("a - b > 0      = %d    （预期：真 —— 下溢之后当然是正数）\n", a - b > 0);
        printf("a - 1 > 100000 = %d    （预期：真）\n", a - 1 > 100000);
        printf("  → 所以“够不够”要用 if (a >= b)，不能写 if (a - b >= 0)\n");
    }

    printf("\n== 现象二：有符号和无符号比较，负数会变巨大正数 ==\n");
    {
        int i = -1;
        unsigned int u = 1;

        printf("-1 <  1  = %d   （都是 int，正常比较）\n", -1 < 1);
        printf("-1 < 1u  = %d   （-1 被转成 4294967295，所以为假）\n", i < u);
        printf("(unsigned)(-1) = %u\n", (unsigned)-1);
        printf("  → 混合比较前先把无符号那侧和有符号那侧统一类型\n");
    }

    printf("\n== 现象三：sizeof 返回无符号，减法会下溢 ==\n");
    {
        printf("sizeof(int) = %zu\n", sizeof(int));
        printf("sizeof(int) - 5       = %zu   （2^64-1，本机 size_t 是 8 字节）\n",
               sizeof(int) - 5);
        printf("sizeof(int) - 5 > 0   = %d\n", sizeof(int) - 5 > 0);
        printf("(unsigned)(sizeof(int) - 5) = %u\n", (unsigned)(sizeof(int) - 5));
        printf("  → 下溢后的“巨大值”有多大，取决于类型宽度\n");
    }

    printf("\n== 附：无符号倒序循环 —— 嵌入式死循环头号成因 ==\n");
    {
        unsigned int i;
        int guard = 0;

        for (i = 3; i >= 0; i--) {          /* i >= 0 对无符号恒为真 */
            printf("  i = %u\n", i);
            if (++guard > 5) {
                printf("  ... 停不下来：0 再减 1 回绕成 4294967295\n");
                break;                       /* 演示用，真实代码里不该有 */
            }
        }
        printf("  正确写法：for (i = n; i-- > 0; )  或  for (i = n; i > 0; i--)\n");
    }

    return 0;
}
