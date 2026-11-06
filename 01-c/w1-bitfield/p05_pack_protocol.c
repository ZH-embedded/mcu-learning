#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* 协议包：1 + 4 + 2 = 7 字节，但对齐后会变成 12 */
#pragma pack(push, 1)
typedef struct {
    uint8_t  tag;
    uint32_t value;
    uint16_t crc;
} packed_t;
#pragma pack(pop)

typedef struct {
    uint8_t  tag;
    uint32_t value;
    uint16_t crc;
} normal_t;

size_t packed_size(void) { return sizeof(packed_t); }
size_t normal_size(void) { return sizeof(normal_t); }

/* 从字节流读 32 位小端 —— 逐字节拼，不依赖p的对齐 */
/* ============================================================
 * TODO 1：从字节流里读 32 位小端整数
 *
 *   - p 为 NULL → 返回 0
 *   - 逐字节拼：p[0] 是最低字节，p[3] 是最高字节
 *   - 每一步都要转成 uint32_t 再左移，否则 p[3] << 24 会溢出 int
 *     （在 32 位 int 上，0xFF << 24 是 UB）
 *
 * 为什么不用 *(uint32_t*)p？因为 p 可能不是 4 的倍数，
 * 在 Cortex-M 上直接 HardFault。这个函数写在 p06 里会再考一次。
 * ============================================================ */
uint32_t read_u32_le(const uint8_t *p)
{
    (void)p;
    /* TODO：见上方注释 */
    return 0;
}

/* 写 32 位小端 */
/* ============================================================
 * TODO 2：把 32 位整数按小端写进字节流
 *
 *   - p 为 NULL → 直接 return
 *   - p[0] = v & 0xFF，p[1] = (v >> 8) & 0xFF，以此类推
 * ============================================================ */
void write_u32_le(uint8_t *p, uint32_t v)
{
    (void)p; (void)v;
    /* TODO：见上方注释 */
}

/* 把结构体序列化成 7 字节线格式，返回写了几字节；0 表示失败 */
/* ============================================================
 * TODO 3：把结构体序列化成 7 字节线格式
 *
 *   布局（小端）：
 *         buf[0]     = tag
 *         buf[1..4]  = value（低字节在前）
 *         buf[5..6]  = crc（低字节在前）
 *
 *   - in 或 buf 为 NULL → 返回 0
 *   - cap < 7 → 返回 0（缓冲区不够就别写，这是安全前提）
 *   - 成功返回 7
 *
 * 关键认知：**结构体不是线格式**。结构体是给人算的，线格式是给
 * 对端解析的，中间必须有一个明确的、逐字节写的转换函数。
 * ============================================================ */
size_t encode_packet(const normal_t *in, uint8_t *buf, size_t cap)
{
    (void)in; (void)buf; (void)cap;
    /* TODO：见上方注释 */
    return 0;
}

/* 从 7 字节线格式解析回结构体，0 成功，-1 失败 */
/* ============================================================
 * TODO 4：把 7 字节线格式解析回结构体
 *
 *   - buf 或 out 为 NULL → 返回 -1
 *   - len < 7 → 返回 -1（长度不够，不能硬读）
 *   - 成功返回 0
 *   - value 用 read_u32_le 读，crc 手工拼两字节
 *
 * 注意参数顺序：**先判指针，再判长度，最后才写 out**。
 * 这和 11-04 那道 filter_above 是同一个套路。
 * ============================================================ */
int decode_packet(const uint8_t *buf, size_t len, normal_t *out)
{
    (void)buf; (void)len; (void)out;
    /* TODO：见上方注释 */
    return 0;
}

/* ---------- 测试 ---------- */
static int pass = 0, fail = 0;
static void ck(const char *name, long long got, long long want)
{
    if (got == want) { pass++; printf("  [PASS] %-50s = %lld\n", name, got); }
    else { fail++; printf("  [FAIL] %-50s 期望 %lld，实际 %lld\n", name, want, got); }
}

int main(void)
{
    printf("===== p05 packed 结构体与协议序列化 =====\n");

    ck("sizeof(packed_t)  = 1+4+2", (long long)packed_size(), 7);
    ck("sizeof(normal_t)  = 对齐后", (long long)normal_size(), 12);
    ck("  省下来的 5 个字节全是填充", (long long)(normal_size() - packed_size()), 5);

    printf("-- 小端读写 --\n");
    const uint8_t raw[4] = {0x78, 0x56, 0x34, 0x12};
    ck("read_u32_le(78 56 34 12)", (long long)read_u32_le(raw), 0x12345678);

    uint8_t out4[4] = {0};
    write_u32_le(out4, 0x12345678u);
    ck("write_u32_le 第 0 字节", out4[0], 0x78);
    ck("write_u32_le 第 3 字节", out4[3], 0x12);

    printf("-- 序列化 --\n");
    normal_t in = {0};
    in.tag = 0xA1;
    in.value = 0x12345678u;
    in.crc = 0xBEEFu;

    uint8_t buf[16];
    memset(buf, 0, sizeof buf);
    ck("encode_packet 返回 7", (long long)encode_packet(&in, buf, sizeof buf), 7);
    ck("  第 0 字节是 tag", buf[0], 0xA1);
    ck("  第 1~4 字节是 value 小端", (long long)read_u32_le(&buf[1]), 0x12345678);
    ck("  第 5 字节是 crc 低字节", buf[5], 0xEF);
    ck("  第 6 字节是 crc 高字节", buf[6], 0xBE);
    ck("  缓冲区不够时返回 0（cap=6）",
       (long long)encode_packet(&in, buf, 6), 0);

    printf("-- 反序列化 --\n");
    normal_t back;
    memset(&back, 0, sizeof back);
    ck("decode_packet 返回 0（成功）", decode_packet(buf, 7, &back), 0);
    ck("  解析出的 tag", back.tag, 0xA1);
    ck("  解析出的 value", (long long)back.value, 0x12345678);
    ck("  解析出的 crc", back.crc, 0xBEEF);

    ck("长度不足 7 返回 -1", decode_packet(buf, 6, &back), -1);
    ck("buf 为 NULL 返回 -1", decode_packet(NULL, 7, &back), -1);
    ck("out 为 NULL 返回 -1", decode_packet(buf, 7, NULL), -1);

    printf("-- 非对齐地址也要能读 --\n");
    uint8_t off[8];
    memset(off, 0, sizeof off);
    write_u32_le(&off[1], 0xDEADBEEFu);          /* 从偏移 1 开始写 */
    ck("从奇数地址读出的值仍然正确",
       (long long)read_u32_le(&off[1]), 0xDEADBEEF);
    ck("  &off[1] 确实不是 4 对齐地址",
       ((uintptr_t)&off[1] % 4) != 0, 1);

    printf("结果: %d 通过, %d 失败\n", pass, fail);
    return fail != 0;
}
