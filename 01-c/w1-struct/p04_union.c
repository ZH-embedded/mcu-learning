/* p04_union.c —— 共用体：所有成员共用同一块内存
 *
 * struct：成员依次排开，sizeof = 各成员之和（+ 填充）
 * union ：成员全部叠在同一个地址上，sizeof = **最大那个成员的大小**
 *
 * 也就是说，写进 union 的 a 成员，再从 b 成员读出来的就是同一批字节，
 * 只是按 b 的类型重新解释了一遍。这叫"类型双关"（type punning）。
 *
 * 它在嵌入式里有三个正当用途：
 *   ① 探字节序：把一个整数和它的字节表示叠在一起看
 *   ② 协议打包/解包：一帧数据既有"整体 32 位"的看法，也有"4 个字节"的看法
 *   ③ 省 RAM：一个变量在不同时刻存不同类型的东西（要自己保证不错读）
 *
 * 不该用的地方：想省事在两个不相关的类型之间乱转。那叫 bug，不叫技巧。
 */
#include <stdio.h>
#include <stddef.h>

/* 一个 32 位整数，和它的 4 个字节，共用 4 字节内存 */
typedef union {
    unsigned int  u32;
    unsigned char b[4];
} u32_u;

static int g_pass = 0;
static int g_fail = 0;

static void check_int(const char *name, int got, int want)
{
    if (got == want)
    {
        printf("  [PASS] %-44s = %d\n", name, got);
        g_pass++;
    }
    else
    {
        printf("  [FAIL] %-44s 期望 %d，实际 %d\n", name, want, got);
        g_fail++;
    }
}

static void check_size(const char *name, size_t got, size_t want)
{
    if (got == want)
    {
        printf("  [PASS] %-44s = %zu\n", name, got);
        g_pass++;
    }
    else
    {
        printf("  [FAIL] %-44s 期望 %zu，实际 %zu\n", name, want, got);
        g_fail++;
    }
}

static void check_hex(const char *name, unsigned int got, unsigned int want)
{
    if (got == want)
    {
        printf("  [PASS] %-44s = 0x%08X\n", name, got);
        g_pass++;
    }
    else
    {
        printf("  [FAIL] %-44s 期望 0x%08X，实际 0x%08X\n", name, want, got);
        g_fail++;
    }
}

/* ============================================================
 * TODO 1：用 union 判断这台机器是不是小端
 *
 * 做法：把 u32 写成 1（内存里是 00 00 00 01），
 *       然后看 b[0] 这个"最低地址的字节"拿到的是 1 还是 0。
 *
 *       小端（x86 / ARM Cortex-M）：最低地址存最低有效字节 → b[0] == 1
 *       大端：                     最低地址存最高有效字节 → b[0] == 0
 *
 * 返回值：小端返回 1，大端返回 0。
 *
 * 为什么这件事重要：串口收到的多字节数据、Flash 里存的 int、
 * 网络协议里的长度字段，全都约定了一种字节序。你按相反的序去解释，
 * 得到的就是一个完全不相关的数 —— 而且不报错。
 *
 * 注意 u 是局部变量，必须**先赋值再读**，否则读的是栈上的垃圾。
 * ============================================================ */
int is_little_endian(void)
{
     u32_u u;
    u.u32 = 1;           
    return u.b[0] == 1;  

}

/* ============================================================
 * TODO 2：把 4 个字节打包成一个 32 位整数
 *
 * 结果按这个顺序排（r 在最高 8 位）：
 *
 *       位 31..24   23..16   15..8    7..0
 *       +--------+--------+--------+--------+
 *       |   r    |   g    |   b    |   a    |
 *       +--------+--------+--------+--------+
 *
 * 用移位和按位或拼起来，不要用 union 去"叠"。
 *
 * 有一个坑必须处理：形参是 unsigned char，它会先被提升成 **int**，
 * 然后才参与移位。如果直接写 `r << 24`，当 r >= 128 时最高位会顶到
 * int 的符号位上 —— 有符号溢出，UB（和昨天比较器那个 a-b 是同一类问题）。
 * 所以移位前先转成 unsigned：`(unsigned)r << 24`。
 * ============================================================ */
unsigned int pack_rgba(unsigned char r, unsigned char g,
                       unsigned char b, unsigned char a)
{
    return ((unsigned int)r << 24) | ((unsigned int)g << 16) |
           ((unsigned int)b << 8)  |  (unsigned int)a;
}

/* ============================================================
 * TODO 3：把一个 32 位整数拆回 4 个字节
 *
 * out 是一个至少 4 字节的数组，按 out[0]=r、out[1]=g、out[2]=b、out[3]=a 写。
 * out 为 NULL 时返回 0，正常返回 1。
 *
 * 用右移 + 掩码 0xFF 取出来，每个字节都要 & 0xFF，
 * 否则移位后高位还带着别的字节的位。
 *
 * 这个函数和 TODO 2 是一对：pack 之后 unpack，应该原样回来（往返测试）。
 * ============================================================ */
int unpack_rgba(unsigned int v, unsigned char *out)
{
    if (out == NULL)
    {
        return 0;
    }
    out[0] = (unsigned char)((v >> 24) & 0xFFu);   /* r */
    out[1] = (unsigned char)((v >> 16) & 0xFFu);   /* g */
    out[2] = (unsigned char)((v >>  8) & 0xFFu);   /* b */
    out[3] = (unsigned char)( v        & 0xFFu);   /* a */
    
    return 1;
}

/* ============================================================
 * TODO 4：返回 u32_u 这个共用体的大小
 *
 * 别写死数字，用 sizeof。
 * 顺便回答自己一个问题：它等于 sizeof(unsigned int) + sizeof(unsigned char[4]) 吗？
 * ============================================================ */
size_t union_size_of(void)
{
    return sizeof(u32_u);
}

int main(void)
{
    printf("=== p04 共用体 ===\n\n");

    printf("sizeof(unsigned int)  = %zu\n", sizeof(unsigned int));
    printf("sizeof(unsigned char[4]) = %zu\n", sizeof(unsigned char[4]));
    printf("sizeof(u32_u)         = %zu   ← 不是 8，是 4：成员叠在一起\n\n",
           sizeof(u32_u));

    u32_u u;
    u.u32 = 0x01020304u;
    printf("写 u.u32 = 0x01020304 之后，从字节那头看：\n");
    printf("  b[0]=0x%02X  b[1]=0x%02X  b[2]=0x%02X  b[3]=0x%02X\n",
           u.b[0], u.b[1], u.b[2], u.b[3]);
    printf("  b[0] 是 0x04 → 小端（最低地址存最低有效字节）\n\n");

    printf("--- 测试 ---\n");

    check_int("is_little_endian()（这台机器）", is_little_endian(), 1);
    check_size("union_size_of() = 4 而不是 8", union_size_of(), 4);
    printf("\n");

    check_hex("pack_rgba(0x11,0x22,0x33,0x44)",
              pack_rgba(0x11, 0x22, 0x33, 0x44), 0x11223344u);
    check_hex("pack_rgba(0,0,0,0)", pack_rgba(0, 0, 0, 0), 0x00000000u);
    check_hex("pack_rgba(255,255,255,255) 全 0xFF",
              pack_rgba(255, 255, 255, 255), 0xFFFFFFFFu);
    check_hex("pack_rgba(255,0,0,0) 只填 r（符号位陷阱）",
              pack_rgba(255, 0, 0, 0), 0xFF000000u);

    unsigned char out[4] = {0, 0, 0, 0};
    check_int("unpack_rgba 正常返回 1",
              unpack_rgba(0x11223344u, out), 1);
    check_int("  out[0] = 0x11（r）", out[0], 0x11);
    check_int("  out[1] = 0x22（g）", out[1], 0x22);
    check_int("  out[2] = 0x33（b）", out[2], 0x33);
    check_int("  out[3] = 0x44（a）", out[3], 0x44);
    check_int("unpack_rgba(NULL) 返回 0", unpack_rgba(1u, NULL), 0);

    unsigned char back[4] = {0, 0, 0, 0};
    unsigned int v = 0xDEADBEEFu;
    unpack_rgba(v, back);
    check_hex("往返：unpack 再 pack 应该原样回来",
              pack_rgba(back[0], back[1], back[2], back[3]), v);

    printf("\n结果: %d 通过, %d 失败\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
