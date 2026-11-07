#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef struct
{
    unsigned int power_on : 1, fault : 1, running : 1, calibrating : 1,
        comm_ok : 1, limit_hi : 1, limit_lo : 1, reserved : 1;
} flag_u32_t;

typedef struct
{
    unsigned char power_on : 1, fault : 1, running : 1, calibrating : 1,
        comm_ok : 1, limit_hi : 1, limit_lo : 1, reserved : 1;
} flag_u8_t;

enum
{
    BIT_POWER_ON = 0,
    BIT_FAULT,
    BIT_RUNNING,
    BIT_CALIBRATING,
    BIT_COMM_OK,
    BIT_LIMIT_HI,
    BIT_LIMIT_LO,
    BIT_RESERVED,
    BIT_COUNT
};

size_t flag_u32_size(void) { return sizeof(flag_u32_t); }
size_t flag_u8_size(void) { return sizeof(flag_u8_t); }

/* ============================================================
 * TODO 1：按位号置位 / 清位
 *
 *   - f 为 NULL → 直接 return
 *   - bit 不在 [0, BIT_COUNT) → 直接 return（越界不能崩，也不能乱写）
 *   - on 非 0 就置 1，为 0 就清 0
 *
 * 位域**没有"运行时下标"**：写不出 f->bit[bit]，只能 switch。
 * 这一步就是要你体会 switch 有多啰嗦 —— 等写完 TODO 6 的字节指针版，
 * 你会自己得出"位域不适合动态访问"这个结论。
 * ============================================================ */
void flag_set(flag_u8_t *f, int bit, int on)
{
    if (f == NULL)
    {
        return;
    }

    switch (bit)
    {
    case BIT_POWER_ON:
        f->power_on = (on != 0);
        break;
    case BIT_FAULT:
        f->fault = (on != 0);
        break;
    case BIT_RUNNING:
        f->running = (on != 0);
        break;
    case BIT_CALIBRATING:
        f->calibrating = (on != 0);
        break;
    case BIT_COMM_OK:
        f->comm_ok = (on != 0);
        break;
    case BIT_LIMIT_HI:
        f->limit_hi = (on != 0);
        break;
    case BIT_LIMIT_LO:
        f->limit_lo = (on != 0);
        break;
    case BIT_RESERVED:
        f->reserved = (on != 0);
        break;
    default:
        return;
    }
}

/* ============================================================
 * TODO 2：按位号读值，返回 0 或 1
 *
 *   - f 为 NULL 或 bit 越界 → 返回 0
 *   - 同样只能 switch
 * ============================================================ */
int flag_get(const flag_u8_t *f, int bit)
{
    if (f == NULL)
    {
        return 0;
    }

    switch (bit)
    {
    case BIT_POWER_ON:
        return f->power_on;
    case BIT_FAULT:
        return f->fault;
    case BIT_RUNNING:
        return f->running;
    case BIT_CALIBRATING:
        return f->calibrating;
    case BIT_COMM_OK:
        return f->comm_ok;
    case BIT_LIMIT_HI:
        return f->limit_hi;
    case BIT_LIMIT_LO:
        return f->limit_lo;
    case BIT_RESERVED:
        return f->reserved;
    default:
        return 0;
    }
}

/* ============================================================
 * TODO 3：数一数有几个位是 1
 *
 *   - f 为 NULL → 返回 0
 *   - 推荐直接循环调用 flag_get(f, i) 累加，别复制 8 个 if
 * ============================================================ */
int flag_popcount(const flag_u8_t *f)
{
    if (f == NULL)
    {
        return 0;
    }

    int sum = 0;
    for (int i = 0; i < BIT_COUNT; i++)
    {
        sum = sum + flag_get(f, i);
    }

    return sum;
}

/* ============================================================
 * TODO 4：把 8 个状态位压成一个字节（序列化）
 *
 *   - f 为 NULL → 返回 0
 *   - 位号 i 对应字节里的第 i 位：置位就 `v |= (1u << i)`
 *   - 注意 (1u << i) 是 unsigned int，赋回 unsigned char 要显式转换
 *
 * 想一想：这个函数返回的字节，和 memcpy(&f, 1) 得到的字节一样吗？
 * 如果位域基类型换成 unsigned int（4 字节），答案还一样吗？
 * ============================================================ */
unsigned char flag_to_byte(const flag_u8_t *f)
{
    if (f == NULL)
    {
        return 0;
    }

    unsigned char v = 0;
    for (int i = 0; i < BIT_COUNT; i++)
    {
        if (flag_get(f, i))
        {
            v |= (1u << i);
        }
    }
    return v;
}

/* ============================================================
 * TODO 5：从一个字节还原 8 个状态位（反序列化）
 *
 *   - f 为 NULL → 直接 return
 *   - 第 i 位：`(b >> i) & 1u`
 *   - 循环 8 次调用 flag_set 即可，读起来比 switch 体面
 * ============================================================ */
void flag_from_byte(flag_u8_t *f, unsigned char b)
{
    if (f == NULL)
    {
        return;
    }

    for (int i = 0; i < BIT_COUNT; i++)
    {
        flag_set(f, i, (b >> i) & 1);
    }
}

/* ============================================================
 * TODO 6：用**字节指针**置位，绕开位域语法
 *
 *   - f 为 NULL 或 bit 不在 [0,8) → 直接 return
 *   - 把 f 强转成 unsigned char *，直接操作那一个字节：
 *         unsigned char *p = (unsigned char *)f;
 *         置位：*p |= (1u << bit);
 *         清位：*p &= ~(1u << bit);
 *
 * 前提是"8 个 1 位字段正好占满 1 个字节、且从最低位开始排"——
 * 这是 gcc 在小端平台上的实际行为，不是 C 标准保证的。
 * 测试里有一条专门比对 flag_set 和 flag_set_raw 的结果，
 * 两者一致就说明"位域在这个平台上确实是 LSB 起排"。
 *
 * 这也是面试能讲的点：可移植代码不该写这个函数，
 * 但调试时它比 switch 快十倍。
 * ============================================================ */
void flag_set_raw(flag_u8_t *f, int bit, int on)
{
    if (f == NULL)
    {
        return;
    }

    if ((bit >= BIT_COUNT) || (bit < 0))
    {
        return;
    }

    unsigned char *p = (unsigned char *)f;
    if (on != 0)
    {
        *p = (unsigned char)(*p | (1u << bit));
    }
    else
    {
        *p = (unsigned char)(*p & ~(1u << bit));
    }
}

/* ============================================================
 * TODO 7：mask 里任意一位被置起来，就返回 1
 *
 *   - f 为 NULL → 返回 0
 *   - 本质是 `(flag_to_byte(f) & mask) != 0`
 * ============================================================ */
int flag_any(const flag_u8_t *f, unsigned char mask)
{
    if (f == NULL)
    {
        return 0;
    }
    if((flag_to_byte(f) & mask ) != 0)
    {
        return 1;
    }
    else
    return 0;
    
}

/* ---------- 测试 ---------- */
static int pass = 0, fail = 0;
static void ck(const char *name, long long got, long long want)
{
    if (got == want)
    {
        pass++;
        printf("  [PASS] %-46s = %lld\n", name, got);
    }
    else
    {
        fail++;
        printf("  [FAIL] %-46s 期望 %lld，实际 %lld\n", name, want, got);
    }
}

int main(void)
{
    printf("===== p01 位域：8 个状态位 =====\n");

    ck("sizeof(flag_u32_t)：用 int 当位域基类型", (long long)flag_u32_size(), 4);
    ck("sizeof(flag_u8_t)：用 char 当位域基类型", (long long)flag_u8_size(), 1);

    flag_u8_t f;
    memset(&f, 0, sizeof(f));
    ck("清零后 popcount", flag_popcount(&f), 0);

    flag_set(&f, BIT_FAULT, 1);
    ck("置 fault 后 get(fault)", flag_get(&f, BIT_FAULT), 1);
    ck("  其它位没被带上", flag_popcount(&f), 1);

    flag_set(&f, BIT_RUNNING, 1);
    ck("再置 running 后 to_byte", (long long)flag_to_byte(&f), 0x06);
    ck("  0x06 是 bit1|bit2", (flag_to_byte(&f) == 0x06), 1);

    flag_set(&f, BIT_FAULT, 0);
    ck("清 fault 后 to_byte", (long long)flag_to_byte(&f), 0x04);

    for (int i = 0; i < BIT_COUNT; i++)
        flag_set(&f, i, 1);
    ck("8 位全置 → 一个字节装满", (long long)flag_to_byte(&f), 0xFF);
    ck("  全置后 popcount", flag_popcount(&f), 8);

    flag_from_byte(&f, 0x00);
    ck("from_byte(0x00) 后全清", flag_popcount(&f), 0);
    flag_from_byte(&f, 0x05);
    ck("from_byte(0x05) → power_on", flag_get(&f, BIT_POWER_ON), 1);
    ck("from_byte(0x05) → running", flag_get(&f, BIT_RUNNING), 1);
    ck("from_byte(0x05) → fault 为 0", flag_get(&f, BIT_FAULT), 0);

    ck("flag_any(0x04) 命中 running", flag_any(&f, 0x04), 1);
    ck("flag_any(0x02) 未命中", flag_any(&f, 0x02), 0);
    ck("flag_any(mask=0x00) 恒为 0", flag_any(&f, 0x00), 0);
    ck("flag_any(mask=0x0A) 位不重叠", flag_any(&f, 0x0A), 0);
    ck("flag_any(0xFF) 全命中", flag_any(&f, 0xFF), 1);
    ck("flag_any(NULL) 返回 0", flag_any(NULL, 0xFF), 0);

    /* 关键：switch 版 和 字节指针版 必须完全一致 */
    flag_u8_t a, b;
    memset(&a, 0, sizeof(a));
    memset(&b, 0, sizeof(b));
    for (int i = 0; i < 8; i++)
    {
        flag_set(&a, i, 1);
        flag_set_raw(&b, i, 1);
    }
    ck("set 与 set_raw 结果一致（证明位域就是 LSB 起排）",
       flag_to_byte(&a) == flag_to_byte(&b), 1);
    ck("  set_raw 全置后字节值", (long long)flag_to_byte(&b), 0xFF);

    flag_set_raw(&b, 3, 0);
    ck("set_raw 清单个 bit3", (long long)flag_to_byte(&b), 0xF7);

    /* 单项写：只置一位 / 只清一位，两个方向都要有鉴别力 */
    flag_u8_t c;
    memset(&c, 0, sizeof(c));
    flag_set_raw(&c, 5, 1);
    ck("set_raw(bit5,1) 只置 bit5", (long long)flag_to_byte(&c), 0x20);
    flag_set_raw(&c, 5, 0);
    ck("set_raw(bit5,0) 只清 bit5", (long long)flag_to_byte(&c), 0x00);
    flag_set_raw(&c, 8, 1);            /* 越界：bit == BIT_COUNT */
    ck("set_raw bit=8 越界不写", (long long)flag_to_byte(&c), 0x00);
    flag_set_raw(&c, -1, 1);
    ck("set_raw bit=-1 越界不写", (long long)flag_to_byte(&c), 0x00);

    /* 边界 */
    ck("set(NULL) 不崩", (flag_set(NULL, 0, 1), 1), 1);
    ck("get(NULL) 返回 0", flag_get(NULL, 0), 0);
    ck("bit 越界 -1 不崩", (flag_set(&a, -1, 1), 1), 1);
    ck("bit 越界 99 不崩", (flag_set(&a, 99, 1), 1), 1);
    ck("get 越界 99 返回 0", flag_get(&a, 99), 0);

    printf("结果: %d 通过, %d 失败\n", pass, fail);
    return fail != 0;
}
