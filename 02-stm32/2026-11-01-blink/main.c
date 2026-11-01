#include <reg52.h>

sbit LED = P0^0;

void delay_ms(unsigned int ms)
{
    unsigned int i, j;
    for (i = ms; i > 0; i--)
        for (j = 110; j > 0; j--);   /* 11.0592MHz 下约 1ms，不精确但够看 */
}

void main(void)
{
    while (1)
    {
        LED = 0;          /* 低电平点亮 */
        delay_ms(500);
        LED = 1;
        delay_ms(500);
    }
}
