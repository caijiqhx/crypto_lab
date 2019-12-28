# Lab3 分组密码算法 AES

> 1613573 秦浩翔

## 实验目的

通过用 AES 算法对实际的数据进行加密和解密来深刻了解 AES 的运行原理。

## 实验内容

AES 加密算法的流程图如下：

<img src="./aes.png" />

左侧为轮函数的流程，主要包含 4 种主要运算操作：字节代换(SubBytes)、行移位(ShiftRows)、列混合(MixColumns)、轮密钥加(AddRoundKey)。右侧为密钥扩展操作。AES 标准算法将 128 位的明文以特定的生成一个 4×4 的矩阵（每个元素8位），即初始状态（state），经由轮函数的迭代转换之后又将作为下一轮迭代的输入继续参与运算直到迭代结束。

### 轮函数

已经知道轮函数主要包含 4 种运算，但不同的运算轮所做的具体的运算组合不同。主要是初始轮 Round0 和 最后一轮 RoundN，所有中间轮的运算都是相同的，会依次进行 4 种运算，即：

1. 字节代换 SubByte
2. 行移位 ShiftRow
3. 列混合 MixColumn
4. 轮密钥加 AddRoundKey

根据 Rinjdel 算法的定义，加密轮数会针对不同的分组及不同的密钥长度选择不同的值，AES 标准仅支持 128bits 的分组长度，密钥长度则有 128、192、256 bits三种选择，因此迭代轮数也依次为 10、12、14 次。

#### 字节代换 Substitute Bytes

字节代换就是对 state 矩阵中的每一个元素通过 S 盒中查找的方式得到对应输出。

```C
uint8_t aes_sub_sbox(uint8_t val){
    return aes_sbox[val];
}
void aes_sub_bytes(AES_CYPHER_T mode, uint8_t *state){
    int i, j;   
    for (i = 0; i < aes_nb[mode]; i++) {
        for (j = 0; j < 4; j++) {
            state[i * 4 + j] = aes_sub_sbox(state[i * 4 + j]);
        }
    }
}
```

#### 行移位 Shift Rows

行移位主要目的就是实现字节在每一行的扩散。

```C
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
```

#### 列混合 Mix Columns

列混合是将 state 矩阵与常矩阵 C 相乘以达成在列上的扩散，本质上是在有限域 $GF(2^8)$ 上的多项式乘法运算。

```C
uint8_t aes_xtime(uint8_t x) {
    return ((x << 1) ^ (((x >> 7) & 1) * 0x1b));
}

uint8_t aes_xtimes(uint8_t x, int ts){
    while (ts-- > 0) {
        x = aes_xtime(x);
    }
    return x;
}

uint8_t aes_mul(uint8_t x, uint8_t y) {
    return ((((y >> 0) & 1) * aes_xtimes(x, 0)) ^
            (((y >> 1) & 1) * aes_xtimes(x, 1)) ^
            (((y >> 2) & 1) * aes_xtimes(x, 2)) ^
            (((y >> 3) & 1) * aes_xtimes(x, 3)) ^
            (((y >> 4) & 1) * aes_xtimes(x, 4)) ^
            (((y >> 5) & 1) * aes_xtimes(x, 5)) ^
            (((y >> 6) & 1) * aes_xtimes(x, 6)) ^
            (((y >> 7) & 1) * aes_xtimes(x, 7)) );
}

void aes_mix_columns(AES_CYPHER_T mode, uint8_t *state) {
    uint8_t y[16] = { 2, 3, 1, 1,  1, 2, 3, 1,  1, 1, 2, 3,  3, 1, 1, 2};
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
```

#### 轮密钥加 Add Round Key

轮密钥加就是将轮密钥简单地与 state 矩阵逐位异或。

```C
void aes_add_round_key(AES_CYPHER_T mode, uint8_t *state, uint8_t *round, int nr) {
    uint32_t *w = (uint32_t *)round;
    uint32_t *s = (uint32_t *)state;
    int i; 
    for (i = 0; i < aes_nb[mode]; i++) {
        s[i] ^= w[nr * aes_nb[mode] + i];
    }
}
```

#### 密钥扩展算法 Key Expansion

密钥扩展算法根据用户提供的密钥生成多轮密钥。对应不同密钥长度，轮密钥分别由 11、13、15组 128bits 的密钥。

```C
void aes_key_expansion(AES_CYPHER_T mode, uint8_t *key, uint8_t *round) {
    uint32_t *w = (uint32_t *)round;
    uint32_t t;
    int i = 0;
    printf("Key Expansion:\n");
    do {
        w[i] = *((uint32_t *)&key[i * 4 + 0]);
        printf("    %2.2d:  rs: %8.8x\n", i, aes_swap_dword(w[i]));
    } while (++i < aes_nk[mode]);

    do {
        printf("    %2.2d: ", i);
        if ((i % aes_nk[mode]) == 0) {
            t = aes_rot_dword(w[i - 1]);
            printf(" rot: %8.8x", aes_swap_dword(t));
            t = aes_sub_dword(t);
            printf(" sub: %8.8x", aes_swap_dword(t));
            printf(" rcon: %8.8x", aes_rcon[i / aes_nk[mode] - 1]);
            t = t ^ aes_swap_dword(aes_rcon[i / aes_nk[mode] - 1]);
            printf(" xor: %8.8x", t);
        } else if (aes_nk[mode] > 6 && (i % aes_nk[mode]) == 4) {
            t = aes_sub_dword(w[i - 1]);
            printf(" sub: %8.8x", aes_swap_dword(t));
        } else {
            t = w[i - 1];
            printf(" equ: %8.8x", aes_swap_dword(t));
        }
        w[i] = w[i - aes_nk[mode]] ^ t;
        printf(" rs: %8.8x\n", aes_swap_dword(w[i]));
    } while (++i < aes_nb[mode] * (aes_rounds[mode] + 1));

    /* key can be discarded (or zeroed) from memory */
}
```

### 解密过程

解密就是加密的逆过程，每一轮的轮运算需要用到加密的逆运算。
