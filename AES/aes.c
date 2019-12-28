#include <stdio.h>
#include <memory.h>
#include "aes.h"

int aes_rounds[] = {10, 12, 14};

int aes_nk[] = {4, 6, 8};

int aes_nb[] = {4, 4, 4};

static const uint32_t aes_rcon[] = {
    0x01000000, 0x02000000, 0x04000000, 0x08000000, 0x10000000, 0x20000000, 0x40000000, 0x80000000,
    0x1b000000, 0x36000000, 0x6c000000, 0xd8000000, 0xab000000, 0xed000000, 0x9a000000};

/* aes sbox and invert-sbox */
static const uint8_t aes_sbox[256] = {
    /* 0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F  */
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16};

static const uint8_t inv_sbox[256] = {
    /* 0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F  */
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d};

uint8_t aes_sub_sbox(uint8_t val) {
    return aes_sbox[val];
}

uint32_t aes_sub_dword(uint32_t val) {
    uint32_t tmp = 0;
    tmp |= ((uint32_t)aes_sub_sbox((uint8_t)((val >> 0) & 0xFF))) << 0;
    tmp |= ((uint32_t)aes_sub_sbox((uint8_t)((val >> 8) & 0xFF))) << 8;
    tmp |= ((uint32_t)aes_sub_sbox((uint8_t)((val >> 16) & 0xFF))) << 16;
    tmp |= ((uint32_t)aes_sub_sbox((uint8_t)((val >> 24) & 0xFF))) << 24;
    return tmp;
}

uint32_t aes_rot_dword(uint32_t val) {
    uint32_t tmp = val;
    return (val >> 8) | ((tmp & 0xFF) << 24);
}

uint32_t aes_swap_dword(uint32_t val){
    return (((val & 0x000000FF) << 24) |
            ((val & 0x0000FF00) << 8) |
            ((val & 0x00FF0000) >> 8) |
            ((val & 0xFF000000) >> 24));
}

/*
 * nr: number of rounds
 * nb: number of columns comprising the state, nb = 4 dwords (16 bytes)
 * nk: number of 32-bit words comprising cipher key, nk = 4, 6, 8 (KeyLength/(4*8))
 */

void aes_key_expansion(AES_CYPHER_T mode, uint8_t *key, uint8_t *round) {
    uint32_t *w = (uint32_t *)round;
    uint32_t t;
    int i = 0;

    printf("Key Expansion:\n");
    do {
        w[i] = *((uint32_t *)&key[i * 4 + 0]);
        // printf("    %2.2d:  rs: %8.8x\n", i, aes_swap_dword(w[i]));
    } while (++i < aes_nk[mode]);

    do {
        // printf("    %2.2d: ", i);
        if ((i % aes_nk[mode]) == 0) {
            t = aes_rot_dword(w[i - 1]);
            // printf(" rot: %8.8x", aes_swap_dword(t));
            t = aes_sub_dword(t);
            // printf(" sub: %8.8x", aes_swap_dword(t));
            // printf(" rcon: %8.8x", aes_rcon[i / aes_nk[mode] - 1]);
            t = t ^ aes_swap_dword(aes_rcon[i / aes_nk[mode] - 1]);
            // printf(" xor: %8.8x", t);
        } else if (aes_nk[mode] > 6 && (i % aes_nk[mode]) == 4) {
            t = aes_sub_dword(w[i - 1]);
            // printf(" sub: %8.8x", aes_swap_dword(t));
        } else {
            t = w[i - 1];
            // printf(" equ: %8.8x", aes_swap_dword(t));
        }
        w[i] = w[i - aes_nk[mode]] ^ t;
        // printf(" rs: %8.8x\n", aes_swap_dword(w[i]));
    } while (++i < aes_nb[mode] * (aes_rounds[mode] + 1));

    /* key can be discarded (or zeroed) from memory */
}

void aes_add_round_key(AES_CYPHER_T mode, uint8_t *state, uint8_t *round, int nr) {
    uint32_t *w = (uint32_t *)round;
    uint32_t *s = (uint32_t *)state;
    int i; 
    for (i = 0; i < aes_nb[mode]; i++) {
        s[i] ^= w[nr * aes_nb[mode] + i];
    }
}

void aes_sub_bytes(AES_CYPHER_T mode, uint8_t *state) {
    int i, j;
    for (i = 0; i < aes_nb[mode]; i++) {
        for (j = 0; j < 4; j++) {
            state[i * 4 + j] = aes_sub_sbox(state[i * 4 + j]);
        }
    }
}

void aes_shift_rows(AES_CYPHER_T mode, uint8_t *state) {
    uint8_t *s = (uint8_t *)state;
    int i, j, r;

    for (i = 1; i < aes_nb[mode]; i++) {
        for (j = 0; j < i; j++) {
            uint8_t tmp = s[i];
            for (r = 0; r < aes_nb[mode]; r++) {
                s[i + r * 4] = s[i + (r + 1) * 4];
            }
            s[i + (aes_nb[mode] - 1) * 4] = tmp;
        }
    }
}

uint8_t aes_xtime(uint8_t x) {
    return ((x << 1) ^ (((x >> 7) & 1) * 0x1b));
}

uint8_t aes_xtimes(uint8_t x, int ts) {
    while (ts-- > 0) {
        x = aes_xtime(x);
    }
    return x;
}

uint8_t aes_mul(uint8_t x, uint8_t y){
    /*
     * encrypt: y has only 2 bits: can be 1, 2 or 3
     * decrypt: y could be any value of 9, b, d, or e
     */

    return ((((y >> 0) & 1) * aes_xtimes(x, 0)) ^
            (((y >> 1) & 1) * aes_xtimes(x, 1)) ^
            (((y >> 2) & 1) * aes_xtimes(x, 2)) ^
            (((y >> 3) & 1) * aes_xtimes(x, 3)) ^
            (((y >> 4) & 1) * aes_xtimes(x, 4)) ^
            (((y >> 5) & 1) * aes_xtimes(x, 5)) ^
            (((y >> 6) & 1) * aes_xtimes(x, 6)) ^
            (((y >> 7) & 1) * aes_xtimes(x, 7)));
}

void aes_mix_columns(AES_CYPHER_T mode, uint8_t *state) {
    uint8_t y[16] = {2, 3, 1, 1, 1, 2, 3, 1, 1, 1, 2, 3, 3, 1, 1, 2};
    uint8_t s[4];
    int i, j, r;
    for (i = 0; i < aes_nb[mode]; i++) {
        for (r = 0; r < 4; r++) {
            s[r] = 0;
            for (j = 0; j < 4; j++) {
                s[r] = s[r] ^ aes_mul(state[i * 4 + j], y[r * 4 + j]);
            }
        }
        for (r = 0; r < 4; r++) {
            state[i * 4 + r] = s[r];
        }
    }
}

void aes_dump(char *msg, uint8_t *data, int len) {
    int i;
    printf("%8.8s: ", msg);
    for (i = 0; i < len; i++) {
        printf(" %2.2x", data[i]);
    }
    printf("\n");
}

int aes_encrypt(AES_CYPHER_T mode, uint8_t *data, int len, uint8_t *key) {
    uint8_t w[4 * 4 * 15] = {0}; /* round key */
    uint8_t s[4 * 4] = {0};      /* state */

    int nr, i, j;

    /* key expansion */
    aes_key_expansion(mode, key, w);

    /* start data cypher loop over input buffer */
    for (i = 0; i < len; i += 4 * aes_nb[mode]) {
        printf("Encrypting block at %u ...\n", i);
        /* init state from user buffer (plaintext) */
        for (j = 0; j < 4 * aes_nb[mode]; j++)
            s[j] = data[i + j];
        /* start AES cypher loop over all AES rounds */
        for (nr = 0; nr <= aes_rounds[mode]; nr++) {
            // printf(" Round %d:\n", nr);
            // aes_dump("input", s, 4 * aes_nb[mode]);
            if (nr > 0) {
                /* do SubBytes */
                aes_sub_bytes(mode, s);
                // aes_dump("  sub", s, 4 * aes_nb[mode]);
                /* do ShiftRows */
                aes_shift_rows(mode, s);
                // aes_dump("  shift", s, 4 * aes_nb[mode]);
                if (nr < aes_rounds[mode]) {
                    /* do MixColumns */
                    aes_mix_columns(mode, s);
                    // aes_dump("  mix", s, 4 * aes_nb[mode]);
                }
            }
            /* do AddRoundKey */
            aes_add_round_key(mode, s, w, nr);
            // aes_dump("  round", &w[nr * 4 * aes_nb[mode]], 4 * aes_nb[mode]);
            // aes_dump("  state", s, 4 * aes_nb[mode]);
        }
        /* save state (cypher) to user buffer */
        for (j = 0; j < 4 * aes_nb[mode]; j++)
            data[i + j] = s[j];
        printf("Output:\n");
        aes_dump("cypher", &data[i], 4 * aes_nb[mode]);
    }
    return 0;
}

int aes_encrypt_ecb(AES_CYPHER_T mode, uint8_t *data, int len, uint8_t *key){
    return aes_encrypt(mode, data, len, key);
}

void inv_shift_rows(AES_CYPHER_T mode, uint8_t *state) {
    uint8_t *s = (uint8_t *)state;
    int i, j, r;
    for (i = 1; i < aes_nb[mode]; i++) {
        for (j = 0; j < aes_nb[mode] - i; j++) {
            uint8_t tmp = s[i];
            for (r = 0; r < aes_nb[mode]; r++) {
                s[i + r * 4] = s[i + (r + 1) * 4];
            }
            s[i + (aes_nb[mode] - 1) * 4] = tmp;
        }
    }
}

uint8_t inv_sub_sbox(uint8_t val) {
    return inv_sbox[val];
}

void inv_sub_bytes(AES_CYPHER_T mode, uint8_t *state) {
    int i, j;

    for (i = 0; i < aes_nb[mode]; i++) {
        for (j = 0; j < 4; j++) {
            state[i * 4 + j] = inv_sub_sbox(state[i * 4 + j]);
        }
    }
}

void inv_mix_columns(AES_CYPHER_T mode, uint8_t *state) {
    uint8_t y[16] = {0x0e, 0x0b, 0x0d, 0x09, 0x09, 0x0e, 0x0b, 0x0d,
                     0x0d, 0x09, 0x0e, 0x0b, 0x0b, 0x0d, 0x09, 0x0e};
    uint8_t s[4];
    int i, j, r;
    for (i = 0; i < aes_nb[mode]; i++) {
        for (r = 0; r < 4; r++) {
            s[r] = 0;
            for (j = 0; j < 4; j++) {
                s[r] = s[r] ^ aes_mul(state[i * 4 + j], y[r * 4 + j]);
            }
        }
        for (r = 0; r < 4; r++) {
            state[i * 4 + r] = s[r];
        }
    }
}

int aes_decrypt(AES_CYPHER_T mode, uint8_t *data, int len, uint8_t *key) {
    uint8_t w[4 * 4 * 15] = {0}; /* round key */
    uint8_t s[4 * 4] = {0};      /* state */
    int nr, i, j;
    /* key expansion */
    aes_key_expansion(mode, key, w);
    /* start data cypher loop over input buffer */
    for (i = 0; i < len; i += 4 * aes_nb[mode]) {
        printf("Decrypting block at %u ...\n", i);
        /* init state from user buffer (cyphertext) */
        for (j = 0; j < 4 * aes_nb[mode]; j++)
            s[j] = data[i + j];
        /* start AES cypher loop over all AES rounds */
        for (nr = aes_rounds[mode]; nr >= 0; nr--) {
            // printf(" Round %d:\n", nr);
            // aes_dump("input", s, 4 * aes_nb[mode]);
            /* do AddRoundKey */
            aes_add_round_key(mode, s, w, nr);
            // aes_dump("  round", &w[nr * 4 * aes_nb[mode]], 4 * aes_nb[mode]);
            if (nr > 0) {
                if (nr < aes_rounds[mode]) {
                    // aes_dump("  mix", s, 4 * aes_nb[mode]);
                    /* do MixColumns */
                    inv_mix_columns(mode, s);
                }
                /* do ShiftRows */
                // aes_dump("  shift", s, 4 * aes_nb[mode]);
                inv_shift_rows(mode, s);
                /* do SubBytes */
                // aes_dump("  sub", s, 4 * aes_nb[mode]);
                inv_sub_bytes(mode, s);
            }
            // aes_dump("  state", s, 4 * aes_nb[mode]);
        }
        /* save state (cypher) to user buffer */
        for (j = 0; j < 4 * aes_nb[mode]; j++)
            data[i + j] = s[j];
        printf("Output:\n");
        aes_dump("plain", &data[i], 4 * aes_nb[mode]);
    }
    return 0;
}

int aes_decrypt_ecb(AES_CYPHER_T mode, uint8_t *data, int len, uint8_t *key){
    return aes_decrypt(mode, data, len, key);
}

uint8_t* aes_cypher_128_test(uint8_t buf[], uint8_t key[], int buf_len, int key_len) {
    uint8_t *cipher = (uint8_t*)malloc(buf_len); 
    printf("\nAES_CYPHER_128 encrypt test case:\n");
    printf("Input:\n");
    aes_dump("data", buf, buf_len);
    aes_dump("key ", key, key_len);
    aes_encrypt(AES_CYPHER_128, buf, buf_len, key);
    int i;
    for(i = 0; i < buf_len; i++) {
        cipher[i] = buf[i];
    }
    printf("\nAES_CYPHER_128 decrypt test case:\n");
    printf("Input:\n");
    aes_dump("data", buf, buf_len);
    aes_dump("key ", key, key_len);
    aes_decrypt(AES_CYPHER_128, buf, buf_len, key);
    return cipher;
}

int cnt_diff(uint8_t a[], uint8_t b[], int len) {
    int i, j, cnt = 0;
    for(i = 0; i < len; i++){
        a[i] ^= b[i];
        for(j = 0; j < 8; j++) {
            if((a[i]>>j) % 2 == 1){
                cnt++;
            }
        }
    }
    return cnt;
}

int main() {

    uint8_t buf[] = {0x00, 0x01, 0x00, 0x01, 0x01, 0xa1, 0x98, 0xaf,
                     0xda, 0x78, 0x17, 0x34, 0x86, 0x15, 0x35, 0x66};
    uint8_t key[] = {0x00, 0x01, 0x20, 0x01, 0x71, 0x01, 0x98, 0xae,
                     0xda, 0x79, 0x17, 0x14, 0x60, 0x15, 0x35, 0x94};
    aes_cypher_128_test(buf, key, sizeof(buf), sizeof(key));

    uint8_t buf1[] = { 0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 
                      0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
    uint8_t key1[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 
                      0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};                      
    aes_cypher_128_test(buf1, key1, sizeof(buf1), sizeof(key1));

    return 0;
}