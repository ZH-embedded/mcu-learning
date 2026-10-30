/* p04_sizeof.c —— 类型宽度、取值范围、溢出（参考实现）
 * 三个坑：sizeof 要用 %zu；UINT_MAX 要用 %u；有符号溢出是 UB
 */
#include <stdio.h>
#include <limits.h>
#include <float.h>

int main(void)
{
    /* sizeof 返回 size_t（无符号），必须用 %zu，用 %d 会类型不匹配 */
    printf("== 一、各类型宽度（字节）==\n");
    printf("%-12s %zu\n", "char",       sizeof(char));
    printf("%-12s %zu\n", "short",      sizeof(short));
    printf("%-12s %zu\n", "int",        sizeof(int));
    printf("%-12s %zu\n", "long",       sizeof(long));
    printf("%-12s %zu\n", "long long",  sizeof(long long));
    printf("%-12s %zu\n", "float",      sizeof(float));
    printf("%-12s %zu\n", "double",     sizeof(double));
    printf("%-12s %zu\n", "long double",sizeof(long double));
    printf("%-12s %zu\n", "void *",     sizeof(void *));
    printf("注意：Windows 上 long 是 4 字节（LLP64），Linux 上是 8 字节（LP64）\n");

    printf("\n== 二、取值范围 ==\n");
    printf("CHAR_MIN  %d\n", CHAR_MIN);
    printf("CHAR_MAX  %d\n", CHAR_MAX);
    printf("SHRT_MIN  %d\n", SHRT_MIN);
    printf("SHRT_MAX  %d\n", SHRT_MAX);
    printf("INT_MIN   %d\n", INT_MIN);
    printf("INT_MAX   %d\n", INT_MAX);
    printf("UINT_MAX  %u\n", UINT_MAX);
    printf("LONG_MIN  %ld\n", LONG_MIN);
    printf("LONG_MAX  %ld\n", LONG_MAX);
    printf("FLT_EPSILON %.10e\n", (double)FLT_EPSILON);
    printf("DBL_EPSILON %.10e\n", DBL_EPSILON);

    printf("\n== 三、溢出实验（先读注释里的判断，再看结果）==\n");
    {
        /* 用 volatile 阻止编译器在编译期算这个表达式，
         * 否则 gcc 会直接报 -Woverflow；但“运行时溢出”同样是未定义行为 */
        volatile int vmax = INT_MAX;
        int wrapped = vmax + 1;

        printf("INT_MAX + 1 有符号溢出（UB，本机表现为回绕）= %d\n", wrapped);
        printf("  → 不要依赖这个结果，它只是本机这一次的表现\n");
    }
    printf("(unsigned)INT_MAX + 1u  定义良好      = %u\n", (unsigned)INT_MAX + 1u);

    printf("\n== 四、size_t 也是无符号，减法会下溢 ==\n");
    printf("sizeof(int) - 5            = %zu\n", sizeof(int) - 5);
    printf("sizeof(int) - 5 > 0        = %d（真）\n", sizeof(int) - 5 > 0);
    printf("(unsigned)(sizeof(int) - 5)= %u\n", (unsigned)(sizeof(int) - 5));

    return 0;
}
