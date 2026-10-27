#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    
    uint32_t high32 = (uint32_t)(challenge >> 32);
    uint32_t low32 = (uint32_t)challenge;
    uint32_t arg1 = high32;
    uint32_t arg2 = (uint32_t)(challenge >> 16);

    uint32_t step0_res = (high32 + 0xfe31) ^ low32;
    uint32_t step1_res = (3 * arg1) ^ (arg2 + 0x2a60);
    uint32_t step2_res = (3 * step1_res + low32) ^ 0x28c0;

    values[0] = step2_res ^ step0_res;
    count = 1;
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    values[0] = (uint32_t)((challenge >> 13) & 0x7F);

    for (int i = 1; i < 6; ++i) {
        uint32_t shift = 7 + 3 * (i - 1);
        uint32_t tmp = (uint32_t)((challenge >> shift) & 0x1F);
        tmp += i;
        tmp ^= values[i-1];
        values[i] = tmp * 3;
    }

    count = 6;
    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
   uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t v0 = (uint32_t)(challenge & 0x7);
    values[0] = v0;

    uint32_t arg = (uint32_t)(challenge >> 32) ^ (uint32_t)(challenge >> 3);

    uint32_t v1 = 0;
    switch(v0) {
        case 0:
            v1 = 5 * arg + 0x821b;
            break;
        case 1:
            v1 = 5 * arg + 0x29a1d;
            break;
        case 2:
            v1 = (arg ^ 0xd8e5) + (arg >> 2);
            break;
        case 3:
            v1 = 5 * arg + 0xc5ad;
            break;
        case 4:
            v1 = (4 * arg) ^ 0x2ea2;
            break;
        case 5:
            v1 = (arg ^ 0x8fb8) + (arg >> 2);
            break;
        case 6:
            v1 = 5 * arg + 0x3c186;
            break;
        case 7:
            v1 = (4 * arg) ^ 0xb962;
            break;
    }
    values[1] = v1;

    count = 2;
    return write_answer(out, cap, values, count);
}

static uint32_t p4_walk(uint32_t n, uint32_t low, uint32_t high, uint32_t x) {
    uint32_t mid = low + (high - low) / 2;
    if (n == mid) {
        return (mid + 0x2b) ^ x;
    } else if (n > mid) {
        uint32_t res = p4_walk(n, mid + 1, high, x);
        return (3 * res ^ (mid + x)) ^ 0xf;
    } else {
        uint32_t res = p4_walk(n, low, mid - 1, x);
        return (3 * res ^ (mid + x)) ^ 0x12;
    }
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t n = (uint32_t)(challenge & 0x7F);
    uint32_t x = (uint32_t)(challenge >> 7) ^ (uint32_t)(challenge >> 32);

    values[0] = p4_walk(n, 0, 0x7F, x);
    count = 1;

    return write_answer(out, cap, values, count);
}

typedef struct {
    int next_idx;
    uint32_t val_a;
    uint32_t val_b;
} p5_node_t;

static const p5_node_t p5_nodes[] = {
    { 4, 0x081a, 0xdc28 },   
    { -1, 0x070f, 0xccfb }, 
    { 7, 0x040b, 0xcd42 },
    { 1, 0x0b17, 0x0d26 },
    { 2, 0x0671, 0xb1f3 },
    { 0, 0x0222, 0x78b1 }, 
    { 9, 0x0151, 0x7c49 }, 
    { 11, 0x0028, 0x4f4c },
    { 5, 0x050b, 0x002a }, 
    { 3, 0x0a3f, 0xc7aa },
    { 8, 0x096a, 0xec28 },
    { 6, 0x0336, 0xae87 }  
};

static const int p5_heads[8] = { 10, 8, 5, 0, 4, 2, 7, 11 };

static uint32_t p5_traverse(uint64_t challenge, uint32_t *out_v1) {
    uint32_t idx = (uint32_t)(challenge & 0x7);
    int cur_idx = p5_heads[idx];

    uint32_t base = (uint32_t)(challenge >> 32) ^ (uint32_t)(challenge >> 6);
    base ^= 0xeaa5;

    base = base * 3 + p5_nodes[cur_idx].val_b;
    base ^= p5_nodes[cur_idx].val_a;
    *out_v1 = p5_nodes[cur_idx].val_a;

    uint32_t loop_cnt = (uint32_t)((challenge >> 3) & 0x3);
    for (uint32_t i = 1; i <= loop_cnt; ++i) {
        cur_idx = p5_nodes[cur_idx].next_idx;
        if (cur_idx < 0) break;
        base = base * 3 + p5_nodes[cur_idx].val_b;
        base ^= p5_nodes[cur_idx].val_a;
        *out_v1 = p5_nodes[cur_idx].val_a;
    }
    return base;
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t v1;
    values[0] = p5_traverse(challenge, &v1);
    values[1] = v1;
    count = 2;

    return write_answer(out, cap, values, count);
}

static inline uint32_t rol3(uint32_t x) {
    return (x << 3) | (x >> 29);
}

static inline uint32_t ror3(uint32_t x) {
    return (x >> 3) | (x << 29);
}

static uint32_t p6_op(uint32_t idx, uint32_t x) {
    switch(idx) {
        case 0:
            return rol3(x) ^ 0x6bc9;
        case 1:
            return ((x >> 7) ^ x) + 0x2aaf;
        case 2:
            return (x + 0xfcfd) ^ (x >> 7);
        case 3:
            return (x ^ 0xd890) + (x << 7);
        case 4:
            return ((x << 5) + 0xebab) ^ (3 * x);
        case 5:
            return ror3(x);
        case 6:
            return (x ^ 0xd890) + (x << 7);
        case 7:
            return (x + 0xfcfd) ^ (x >> 7);
        default:
            return 0;
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t idx1 = (uint32_t)(challenge & 0x7);
    uint32_t idx2 = (uint32_t)((challenge >> 3) & 0x7);

    uint32_t arg = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);

    uint32_t res = p6_op(idx1, arg);
    res ^= 0x5746;
    res = p6_op(idx2, res);
    values[0] = res + 0x9b24;

    count = 1;
    return write_answer(out, cap, values, count);
}


static inline uint32_t rol7(uint32_t x) {
    return (x << 7) | (x >> 25);
}


static uint32_t secret_mix(uint32_t x) {
    return (rol7(x) + 0xd515) ^ 0xe16d;
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t v0 = (uint32_t)(context & 0x3);
    values[0] = v0;

    uint32_t chal_half = (uint32_t)(challenge >> 32) ^ (uint32_t)challenge;
    uint32_t ctx_half = (uint32_t)(context >> 32) ^ (uint32_t)context;
    uint32_t arg = chal_half ^ ctx_half;

    uint32_t offset = v0 * 0xd515;
    offset ^= (uint32_t)(context >> 16);
    uint32_t v1 = secret_mix(arg) ^ offset;
    values[1] = v1;

    uint32_t tmp = (chal_half >> 7) + ctx_half + v0 + 0xe16d;
    values[2] = v1 ^ tmp;

    count = 3;
    write_answer(out, cap, values, count);
    return 0;
}
