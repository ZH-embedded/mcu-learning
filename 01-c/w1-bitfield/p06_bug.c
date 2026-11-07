#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* ============================================================
 * bug 1：位域忘了写 unsigned
 *
 *     typedef struct { int flag:1; } b;      // 1 位，有符号
 *     b.flag = 1;                            // 存进去是 1
 *     printf("%d", b.flag);                   // 打印 -1
 *
 * 1 位的 int 只有 0 和 -1 两个值：最高位就是符号位，写 1 等于写 -1。
 * 更坑的是 `if (b.flag)` 判断仍然为真，所以只有拿去参与算术时才炸。
 * ============================================================ */
typedef struct { int flag:1; }  bad_sign_t;     /* 错误示范 */
typedef struct { unsigned int flag:1; } good_t; /* 正确类型 */

/* 下面两处 `= 1` 会被 gcc 报 -Woverflow（"1 变成 -1"）。这正是 bug 本身，
 * 编译器其实在提醒你，这里显式关掉警告只是为了保持"零警告"的编译标准。 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverflow"
int bug1_wrong(void)
{
    bad_sign_t b;
    memset(&b, 0, sizeof b);
    b.flag = 1;
    return b.flag;                 /* -1 */
}

/* TODO 1：用 good_t 存 1，返回值必须是 1 */
/* ============================================================
 * TODO 1：用 good_t 存 1，返回值必须是 1
 *
 *   - 声明 good_t g; memset 清零; g.flag = 1; return g.flag;
 *   - 对比上面 bug1_wrong 返回的 -1，理解"1 位的 int 只有 0 和 -1"
 * ============================================================ */
int bug1_fixed(void)
{
    good_t a;
    memset(&a, 0, sizeof(a));
    a.flag = 1;
    return a.flag;
}

/* ============================================================
 * bug 2：char 位域在 gcc 上默认是 signed char
 * ============================================================ */
typedef struct { char flag:1; }           bad_char_t;
typedef struct { unsigned char flag:1; }  good_char_t;

int bug2_wrong(void)
{
    bad_char_t b;
    memset(&b, 0, sizeof b);
    b.flag = 1;
    return b.flag;                 /* 同样是 -1 */
}
#pragma GCC diagnostic pop

/* TODO 2：用 good_char_t 存 1，返回 1 */
/* ============================================================
 * TODO 2：用 good_char_t 存 1，返回 1
 *
 *   - 写法和 TODO 1 一样，只是类型换成 unsigned char 基
 *   - char 的符号性是**实现定义**的：gcc/x86 默认 signed，
 *     ARM 上默认 unsigned，所以同一份代码在两端行为不同。
 *     这条能直接当面试答案。
 * ============================================================ */
int bug2_fixed(void)
{
    good_char_t b;
    memset(&b, 0, sizeof b);
    b.flag = 1;
    return b.flag;
}

/* ============================================================
 * bug 3：把结构体直接 memcpy 进通信缓冲
 *
 *     memcpy(buf, &msg, sizeof(msg));      // 8 字节，含 3 字节填充垃圾
 *
 * 填充字节里可能是上一条报文的残留，对端按 5 字节解析就会错位；
 * 而且不同编译器/不同对齐设置下这 8 字节的布局都不一样。
 * 正确做法：一个字段一个字段地写，明确字节序。
 * ============================================================ */
typedef struct {
    uint8_t  tag;
    uint32_t value;
} msg_t;                            /* sizeof = 8，其中 3 字节是填充 */

void bug3_wrong(const msg_t *m, uint8_t *buf)
{
    memcpy(buf, m, sizeof(*m));     /* 错误示范：8 字节 */
}

/* TODO 3：正确序列化 —— 只写 5 字节，value 用小端，返回写了多少字节。
 *         buf 为 NULL、cap < 5 都返回 0。 */
/* ============================================================
 * TODO 3：正确序列化，只写 5 字节
 *
 *   - m 或 buf 为 NULL → 返回 0
 *   - cap < 5 → 返回 0
 *   - buf[0] = tag；buf[1..4] = value 小端；返回 5
 *
 * 对比 bug3_wrong：它 memcpy 了 sizeof(msg_t) = 8 字节，
 * 中间 3 字节是填充。测试里故意把结构体先 memset 成 0xEE，
 * 你会看到那 3 个 0xEE 被原样发到线上 —— 对端解析直接错位。
 * ============================================================ */
size_t bug3_fixed(const msg_t *m, uint8_t *buf, size_t cap)
{
    if (m == NULL || buf == NULL || cap < 5)
    {
        return 0;
    }

    buf[0] = m->tag;
    for (int i = 1; i < 5; i++)
    {
        buf[i] = (m->value >> ((i-1)*8))&0xFF;
    }
    
    
    return 5;
}

/* ============================================================
 * bug 4：从 packed 结构体里取成员地址，强转成 uint32_t* 解引用
 *
 *     return *(const uint32_t *)&p->value;   // 地址可能不是 4 的倍数
 *
 * x86 允许非对齐访问，所以在你电脑上「跑得好好的」；
 * Cortex-M0/M4 上这是 HardFault（UsageFault），M3/M4 部分指令支持非对齐
 * 但多字节访问仍然可能 fault。这类 bug 只在板子上出现。
 * 正确做法：memcpy 到局部变量，让编译器生成字节访问。
 * ============================================================ */
#pragma pack(push, 1)
typedef struct {
    uint8_t  tag;
    uint32_t value;
} packed_msg_t;                     /* sizeof = 5 */
#pragma pack(pop)

uint32_t bug4_wrong(const packed_msg_t *p)
{
    return *(const uint32_t *)&p->value;   /* 错误示范：非对齐解引用 */
}

/* TODO 4：安全地读出 value（用 memcpy，不要强转指针） */
/* ============================================================
 * TODO 4：安全地从 packed 结构体里读 value
 *
 *   - p 为 NULL → 返回 0
 *   - 用 memcpy(&v, &p->value, sizeof v) 读进局部变量，
 *     **不要**写 *(const uint32_t *)&p->value
 *
 * memcpy 只有 4 字节，编译器会展开成 4 次字节访问，
 * 不要求地址对齐，所以 Cortex-M0 上也不会 fault。
 * ============================================================ */
uint32_t bug4_fixed(const packed_msg_t *p)
{
    if (p == NULL)
    {
        return 0;
    }

    uint32_t v = 0;
    memcpy(&v, &p->value, sizeof v);
    
    return v;
}

/* ---------- 测试 ---------- */
static int pass = 0, fail = 0;
static void ck(const char *name, long long got, long long want)
{
    if (got == want) { pass++; printf("  [PASS] %-52s = %lld\n", name, got); }
    else { fail++; printf("  [FAIL] %-52s 期望 %lld，实际 %lld\n", name, want, got); }
}

int main(void)
{
    printf("===== p06 位域与对齐的四个 bug =====\n");

    printf("-- bug1：int 位域没写 unsigned --\n");
    ck("错误版 int flag:1 存 1 读出来是 -1", bug1_wrong(), -1);
    ck("修复版 unsigned int flag:1 读出 1", bug1_fixed(), 1);

    /* 位宽决定值域：1 位无符号只有 0/1，写 2 会被截成 0 */
    good_t g2;
    memset(&g2, 0, sizeof g2);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverflow"
#pragma GCC diagnostic ignored "-Wconversion"
    g2.flag = 2;
#pragma GCC diagnostic pop
    ck("  unsigned:1 塞 2 被截成 0（值域只有 0/1）", (long long)g2.flag, 0);

    /* 有符号那一位真正的伤害在算术上：-1 当计数用会倒扣 */
    ck("  bug1 的 -1 参与算术会倒扣", (long long)(bug1_wrong() + 1), 0);
    /* 但 if 判断照样为真，所以这个 bug 极难靠肉眼发现 */
    ck("  if 判断照样为真，掩盖了 bug", bug1_wrong() != 0, 1);

    printf("-- bug2：char 位域默认也是 signed --\n");
    ck("错误版 char flag:1 读出来是 -1", bug2_wrong(), -1);
    ck("修复版 unsigned char flag:1 读出 1", bug2_fixed(), 1);
    /* 基类型决定存储单元：同样 1 位，一个占 1 字节，一个占 4 字节 */
    ck("  unsigned char:1 只占 1 字节", (long long)sizeof(good_char_t), 1);
    ck("  unsigned int:1 占 4 字节（随基类型）", (long long)sizeof(good_t), 4);

    printf("-- bug3：结构体直接 memcpy 上线 --\n");
    msg_t m;
    memset(&m, 0xEE, sizeof m);     /* 整个结构体先涂成 0xEE，模拟"上一包残留" */
    m.tag = 0x5A;
    m.value = 0x11223344u;
    ck("sizeof(msg_t) 是 8（含 3 字节填充）", (long long)sizeof(msg_t), 8);

    uint8_t wrong_buf[8];
    memset(wrong_buf, 0xEE, sizeof wrong_buf);
    bug3_wrong(&m, wrong_buf);
    ck("错误示范写满了 8 字节", (long long)sizeof wrong_buf, 8);
    ck("  填充位上是脏数据（第 1 字节不是 tag 也不是 value）", wrong_buf[1], 0xEE);

    uint8_t right_buf[8];
    memset(right_buf, 0xEE, sizeof right_buf);
    ck("修复版只写 5 字节", (long long)bug3_fixed(&m, right_buf, sizeof right_buf), 5);
    ck("  第 0 字节 tag", right_buf[0], 0x5A);
    ck("  第 1 字节 value 低字节", right_buf[1], 0x44);
    ck("  第 4 字节 value 高字节", right_buf[4], 0x11);
    ck("  第 5 字节没被碰过（还是 0xEE）", right_buf[5], 0xEE);
    ck("cap 不够返回 0", (long long)bug3_fixed(&m, right_buf, 4), 0);

    printf("-- bug4：packed 结构体取成员地址强转 --\n");
    uint8_t *arena = (uint8_t *)malloc(16);
    packed_msg_t *p = (packed_msg_t *)(void *)(arena + 1);   /* 故意造一个非对齐地址 */
    memset(arena, 0, 16);
    p->tag = 0x01;
    p->value = 0xCAFEBABEu;
    ck("sizeof(packed_msg_t) 是 5", (long long)sizeof(packed_msg_t), 5);
    ck("&p->value 不是 4 的倍数（正是危险所在）",
       ((uintptr_t)&p->value % 4) != 0, 1);
    ck("错误版在 x86 上碰巧读对了（测试抓不住）", (long long)bug4_wrong(p), 0xCAFEBABE);
    ck("修复版 memcpy 读出的值", (long long)bug4_fixed(p), 0xCAFEBABE);
    ck("  修复版结果与错误版一致，但不再依赖对齐",
       bug4_fixed(p) == bug4_wrong(p), 1);
    ck("buf 为 NULL 返回 0", (long long)bug4_fixed(NULL), 0);
    free(arena);

    printf("结果: %d 通过, %d 失败\n", pass, fail);
    return fail != 0;
}
