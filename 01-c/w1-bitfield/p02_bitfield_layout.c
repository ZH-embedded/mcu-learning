#include <stdio.h>
#include <string.h>

/* 8 个 1 位字段，基类型 char */
typedef struct {
    unsigned char b0:1, b1:1, b2:1, b3:1, b4:1, b5:1, b6:1, b7:1;
} bits8_t;

/* 4 + 5 位，基类型 char：一个存储单元只有 8 位，放不下 → 开第二个字节 */
typedef struct {
    unsigned char lo:4;
    unsigned char hi:5;
} cross8_t;

/* 4 + 5 位，基类型 short：16 位单元够用 → 挤在同一个单元里，hi 横跨字节边界 */
typedef struct {
    unsigned short lo:4;
    unsigned short hi:5;
} cross16_t;

typedef union { cross8_t  c; unsigned char  raw[2]; } u8_t;
typedef union { cross16_t c; unsigned char  raw[2]; } u16_t;

size_t bits8_size(void)   { return sizeof(bits8_t); }
size_t cross8_size(void)  { return sizeof(cross8_t); }
size_t cross16_size(void) { return sizeof(cross16_t); }

/* 把 8 个 0/1 组装成一个字节：v[0] 是 bit0 */
/* ============================================================
 * TODO 1：把 8 个 0/1 组装成一个字节，v[0] 对应 bit0
 *
 *   - v 为 NULL → 返回 0
 *   - v[i] 非 0 就把第 i 位置 1
 * ============================================================ */
unsigned char bits_pack(const int *v)
{
    if (v == NULL)
    {
        return 0;
    }
    
    unsigned char a = 0;
    for (int i = 0; i < 8; i++)
    {
        if (v[i] != 0)
        {
           a =  a | (1<<i);
        }
    }
    
    return a;
}

/* 反过来拆 */
/* ============================================================
 * TODO 2：把一个字节拆成 8 个 0/1，out[0] 是最低位
 *
 *   - out 为 NULL → 直接 return
 *   - out[i] = (b >> i) & 1
 * ============================================================ */
void bits_unpack(unsigned char b, int *out)
{
    if (out == NULL)
    {
        return;
    }

    for (int i = 0; i < 8; i++)
    {
        out[i] = (b>>i)&1;
    }
    
    
}

/* char 基类型：lo 占低 4 位，hi 另开一个字节 */
/* ============================================================
 * TODO 3：基类型是 char，4 位 + 5 位放不下一个字节，会怎样？
 *
 *   - 把 lo 写进 u.c.lo、hi 写进 u.c.hi，然后按小端把 raw[0] | raw[1]<<8
 *     拼成 16 位返回
 *   - lo 要 & 0x0F，hi 要 & 0x1F（位域会截断，但显式写出来更清楚）
 *
 * 先猜结果再写：lo=0xA、hi=0x1F 时返回什么？
 * 这道题的答案会告诉你"char 基类型的位域，一个存储单元只有 8 位"。
 * ============================================================ */
unsigned short cross8_low16(unsigned char lo, unsigned char hi)
{
    u8_t u;
    memset(&u, 0, sizeof u);
    u.c.lo = (unsigned char)(lo & 0x0F);
    u.c.hi = (unsigned char)(hi & 0x1F);
    return (unsigned short)(u.raw[0] | (u.raw[1] << 8));


}

/* short 基类型：lo 占 bit0-3，hi 占 bit4-8，横跨两字节 */
/* ============================================================
 * TODO 4：同样的 4 位 + 5 位，基类型换成 short
 *
 *   - 写法和 TODO 3 一模一样，只是用 cross16_t 那个联合体
 *   - 观察返回值和 TODO 3 差在哪里：hi 这次是紧接在 lo 后面，
 *     从 bit4 开始，横跨了字节边界
 *
 * 这就是"位域会跨存储单元边界"的具体样子。
 * 协议里的字段如果跨字节，你手写的解析代码就必须按位拼，
 * 不能按字节切。
 * ============================================================ */
unsigned short cross16_low16(unsigned char lo, unsigned char hi)
{
    u16_t u;
    memset(&u, 0, sizeof u);
    u.c.lo = (unsigned short)(lo & 0x0F);
    u.c.hi = (unsigned short)(hi & 0x1F);
    return (unsigned short)(u.raw[0] | (u.raw[1] << 8));
}

/* ---------- 测试 ---------- */
static int pass = 0, fail = 0;
static void ck(const char *name, long long got, long long want)
{
    if (got == want) { pass++; printf("  [PASS] %-52s = 0x%llX\n", name, got); }
    else { fail++; printf("  [FAIL] %-52s 期望 0x%llX，实际 0x%llX\n", name, want, got); }
}

int main(void)
{
    printf("===== p02 位域布局：谁占哪一位 =====\n");

    ck("sizeof(bits8_t) 八个 1 位字段", (long long)bits8_size(), 1);
    ck("sizeof(cross8_t) 4+5 位，char 单元", (long long)cross8_size(), 2);
    ck("sizeof(cross16_t) 4+5 位，short 单元", (long long)cross16_size(), 2);

    int v[8] = {1, 0, 1, 0, 0, 0, 0, 1};
    ck("bits_pack({1,0,1,0,0,0,0,1}) = bit7|bit2|bit0", (long long)bits_pack(v), 0x85);

    int r[8] = {0};
    bits_unpack(0xA5, r);
    ck("bits_unpack(0xA5) → out[0]（最低位）", r[0], 1);
    ck("  out[1]", r[1], 0);
    ck("  out[2]", r[2], 1);
    ck("  out[7]（最高位）", r[7], 1);

    /* 第一个位域占的是最低位还是最高位？ */
    bits8_t b;
    memset(&b, 0, sizeof b);
    b.b0 = 1;
    {
        unsigned char raw;
        memcpy(&raw, &b, 1);
        ck("b0=1 时整个字节的值（LSB 起排 → 0x01）", (long long)raw, 0x01);
    }

    memset(&b, 0, sizeof b);
    b.b7 = 1;
    {
        unsigned char raw;
        memcpy(&raw, &b, 1);
        ck("b7=1 时整个字节的值（0x80）", (long long)raw, 0x80);
    }

    /* char 基类型：hi 另起一个字节 */
    ck("cross8: lo=0xA, hi=0x1F → 两字节", (long long)cross8_low16(0x0A, 0x1F), 0x1F0A);
    ck("cross8: hi 被挤到第 2 字节（高字节 = 0x1F）",
       (long long)(cross8_low16(0x00, 0x1F) >> 8), 0x1F);

    /* short 基类型：hi 从 bit4 开始，横跨字节边界 */
    ck("cross16: lo=0xA, hi=0x1F → 两字节（hi 跨字节）",
       (long long)cross16_low16(0x0A, 0x1F), 0x01FA);
    ck("cross16: lo=0xA 只占低 4 位", (long long)(cross16_low16(0x0A, 0x00) & 0xFF), 0x0A);

    ck("bits_pack(NULL) 返回 0", bits_pack(NULL), 0);
    ck("bits_unpack(b, NULL) 不崩", (bits_unpack(0xFF, NULL), 1), 1);

    printf("结果: %d 通过, %d 失败\n", pass, fail);
    return fail != 0;
}
