//十进制 255 转 2 / 8 / 16
#include <stdio.h>
#include <string.h>

void dec_to_bin(int a,char* STR)
{
    char str[64] = {'0'};
    int i = 0;
    if (a == 0) {                // 特殊情况
        STR[0] = '0';
        STR[1] = '\0';
        return;
    }
    while(a != 0)
    {

        str[i] = a%2 + '0';     
        a = a/2;
        i++;
    }
    int m = 0;
    while (i>0)
    {
        STR[m++] = str[--i];
    }
    STR[m] = '\0';
    return;
}

void dec_to_ocr(int a,char* STR)
{
    char str[64] = {'0'};
    int i = 0;
    if (a == 0) {                // 特殊情况
        STR[0] = '0';
        STR[1] = '\0';
        return;
    }
    while(a != 0)
    {
        str[i++] = (a % 8) + '0';   
        a = a / 8;
        
    }
    int m = 0;
    while (i>0)
    {
        STR[m++] = str[--i];
    }
    STR[m] = '\0';
}

void dec_to_hex(int a,char* STR)
{
    char str[64] = {'0'};
    int i = 0;
    if (a == 0) {                // 特殊情况
        STR[0] = '0';
        STR[1] = '\0';
        return;
    }
    while(a != 0)
    {
        if((a%16) < 10)
        {
            str[i++] = (a % 16) + '0';
        }
        else
        {
            str[i++] = (a % 16 -10) + 'A';
        }   
        a = a / 16;
        
    }
    int m = 0;
    while (i>0)
    {
        STR[m++] = str[--i];
    }
    STR[m] = '\0';
}

int hex_to_dec(char* STR)
{
    int a = 0;
    int i = 0;
    while(STR[i] != 0)
    {
        if('0' <= STR[i] && STR[i] <= '9')
        {
            a = a * 16 + (STR[i] - '0') ;
        }
        else
        {
            a = a * 16 + (STR[i] - 'A' + 10) ;
        }
        i++;
    }
    return a;
}

int main()
{
    int a = 0;
    char STR[64] = "0";
    scanf("%s",STR);
//    scanf("%d", &a);
//    dec_to_bin(a,STR);
//    dec_to_ocr(a,STR);
//    dec_to_hex(a,STR);
    a = hex_to_dec(STR);
//    printf("%s\n",STR);
    printf("%d\n",a);
    return 0;
}