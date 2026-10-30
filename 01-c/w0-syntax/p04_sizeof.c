//打印 `char/short/int/long/long long/float/double/指针` 的 `sizeof`
#include <stdio.h>
#include <limits.h>

int main(void)
{
    printf("char      %zu 字节\n", sizeof(char));
    printf("short     %zu 字节\n", sizeof(short));
    printf("int       %zu 字节\n", sizeof(int));
    printf("long      %zu 字节\n", sizeof(long));
    printf("long long %zu 字节\n", sizeof(long long));
    printf("float     %zu 字节\n", sizeof(float));
    printf("double    %zu 字节\n", sizeof(double));
    printf("指针      %zu 字节\n", sizeof(void *));

    printf("INT_MAX   %d\n", INT_MAX);
    printf("INT_MIN   %d\n", INT_MIN);
    printf("UINT_MAX  %u\n", UINT_MAX);
    printf("INT_MAX+1 无符号 %u\n", (unsigned)INT_MAX + 1u);
    return 0;
}
