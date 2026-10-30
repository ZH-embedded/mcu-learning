//打印一个 `unsigned char` 的 8 个二进制位（从高位到低位）
#include <stdio.h>

int main()
{
    int a = 0;
    unsigned char c;
    scanf(" %i",&a);
    c = (unsigned char)a;
    int n = 0;
    scanf("%d",&n);
    for (int i = 7; i >= 0; i--)
    {
        int bit = (c >> i) & 1;//从高到低
        if(i == 3)
        {
            printf(" ");
        }
     printf("%d", bit);
    }
    printf("\n");

    //置位
    for (int i = 7; i >= 0; i--)
    {
        int bit = (c >> i) & 1;//从高到低
        if(n == i)
        {
            bit = bit | 1;
        }
        if(i == 3)
        {
            printf(" ");
        }
    printf("%d", bit);
    }
    printf("\n");

    //清零
    for (int i = 7; i >= 0; i--)
    {
        int bit = (c >> i) & 1;//从高到低
        if(n == i)
        {
            bit = bit & 0;
        }
        if(i == 3)
        {
            printf(" ");
        }
    printf("%d", bit);
    }
    printf("\n");

    //取反
    for (int i = 7; i >= 0; i--)
    {
        int bit = (c >> i) & 1;//从高到低
        if(n == i)
        {
            bit = (~bit) & 1;
        }
        if(i == 3)
        {
            printf(" ");
        }
    printf("%d", bit);
    }
    printf("\n");    
    return 0;
}