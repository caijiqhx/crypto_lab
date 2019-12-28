#include"des.h"
using namespace std;

// 初始置换
int IP_1[64] = { 
	58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4,
	62, 54, 46, 38, 30, 22, 14, 6, 64, 56, 48, 40, 32, 24, 16, 8,
	57, 49, 41, 33, 25, 17, 9,  1, 59, 51, 43, 35, 27, 19, 11, 3,
	61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7
};
// 最终逆置换
int IP_2[64] = { 
	40, 8, 48, 16, 56, 24, 64, 32, 39, 7, 47, 15, 55, 23, 63, 31,
	38, 6, 46, 14, 54, 22, 62, 30, 37, 5, 45, 13, 53, 21, 61, 29,
	36, 4, 44, 12, 52, 20, 60, 28, 35, 3, 43, 11, 51, 19, 59, 27,
	34, 2, 42, 10, 50, 18, 58, 26, 33, 1, 41,  9, 49, 17, 57, 25,
};
// 压缩置换1
int PC_1[56] = { 
	57, 49,	41,	33,	25,	17,	9,
	 1,	58,	50,	42,	34,	26,	18,
	10,	2,	59,	51, 43, 35,	27,
	19,	11,	3,	60,	52,	44,	36,
	63,	55,	47,	39,	31,	23,	15,
	 7,	62,	54,	46,	38,	30,	22,
	14,	6,	61,	53,	45,	37,	29,
	21,	13,	5,	28,	20,	12,	4
};
// 压缩置换2
int PC_2[48] = { 
	14, 17,	11,	24,	1,	5,
	 3,	28,	15,	6,  21,	10,
	23,	19,	12,	4,	26,	8,
	16,	7,	27,	20,	13,	2,
	41,	52,	31,	37,	47,	55,
	30,	40,	51,	45,	33,	48,
	44,	49,	39,	56,	34,	53,
	46,	42,	50,	36,	29,	32
};
// E表
int E[48] = {
	32,  1,  2,  3,  4,  5,  4,  5,  6,  7,  8,  9,
	 8,  9, 10, 11, 12, 13, 12, 13, 14, 15, 16, 17,
	16, 17, 18, 19, 20, 21, 20, 21, 22, 23, 24, 25,
	24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32, 1 
};
// S 盒
int S_Box[8][4][16] = {
	// S1
	14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
	 0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
	 4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
	15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13,
	// S2
	15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
	 3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
	 0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
	13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9,
	// S3
	10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
	13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
	13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
	 1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12,
	// S4
	 7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
	13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
	10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
	 3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14,
	// S5
	 2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
	14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
	 4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
	11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3,
	// S6
	12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
	10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
	 9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
	 4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13,
	// S7
	 4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
	13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
	 1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
	 6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12,
	// S8
	13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
	 1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
	 7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
	 2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
};
// P表
int P[32] = {
	16, 7, 20, 21, 29, 12, 28, 17, 1,  15, 23, 26, 5,  18, 31, 10,
	 2, 8, 24, 14, 32, 27, 3,  9,  19, 13, 30, 6,  22, 11, 4,  25 
};
// 每一轮子密钥循环移位的位数
int movetimes[17] = { 0, 1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1 };
// 每一轮的子密钥
b48 finalKey[17];

// 置换函数，P位数据通过表置换成Q位数据
template<int P, int Q>
inline bitset<Q> permutation(bitset<P> k, int table[Q])
{
	bitset<Q> temp;
	for (int i = 0; i < Q; i++)
	{
		temp[Q - i - 1] = k[P - table[i]];
	}
	return temp;
}
// 循环移位，28位的Ci, Di
inline b28 movemove(b28 k, int shift) {
	return (k << shift) | (k >> (28 - shift));
}
// 构造函数作为子密钥生成函数
DES::DES(b64 k) : k(k)
{
	b56 k_0;
	b28 k_c,k_d;
	b56 key;
	// PC_1置换
	k_0=permutation<64, 56>(k, PC_1);
	
	k_c = (k_0 >> 28).to_ullong() ;
	k_d = (k_0 << 28 >> 28).to_ullong();
	
	for (int round = 1; round <= 16; round++)
	{
		k_c = movemove(k_c, movetimes[round]);
		k_d = movemove(k_d, movetimes[round]);

		for (int i = 28; i<56; ++i)
			key[i] = k_c[i - 28];
		for (int i = 0; i<28; ++i)
			key[i] = k_d[i];
		// 循环移位后经过 PC_2 置换得到每一轮的子密钥
		finalKey[round] = permutation<56, 48>(key, PC_2);
	}
}

// 加密
b64 DES::des_encrypt(b64 m)
{
	b64 m_0;
	b32 m_l, m_r;
	b64 result;
	b64 finalresult;
	// 初始 IP 置换
	m_0 = permutation<64, 64>(m, IP_1);
	m_l = (m_0 >> 32).to_ullong();
	m_r = (m_0 << 32 >> 32).to_ullong();
    // 16 轮 f 函数
	for (int round = 1; round <= 16; round++)
	{
		b32 temp;
		temp=m_l ^ f(m_r, E, finalKey[round], S_Box);
		m_l = m_r;
		m_r = temp;
	}
	for (int i = 0; i<32; ++i)
		result[i] = m_l[i];
	for (int i = 32; i<64; ++i)
		result[i] = m_r[i - 32];
	// 逆置换
	finalresult = permutation<64, 64>(result, IP_2);
	return finalresult;
}

// 解密
b64 DES::des_decrypt(b64 c)
{
	b64 c_0;
	b32 c_l, c_r;
	b64 result;
	b64 finalresult;
	c_0 = permutation<64, 64>(c, IP_1);
	c_l = (c_0 >> 32).to_ullong();
	c_r = (c_0 << 32 >> 32).to_ullong();
	for (int round = 1; round <= 16; round++)
	{
		b32 temp;
		temp = c_l ^ f(c_r, E, finalKey[17-round], S_Box);
		c_l = c_r;
		c_r = temp;
	}
	for (int i = 0; i<32; ++i)
		result[i] = c_l[i];
	for (int i = 32; i<64; ++i)
		result[i] = c_r[i - 32];
	finalresult = permutation<64, 64>(result, IP_2);
	return finalresult;
}

// f 函数
b32 DES::f(b32 R, int E[48], b48 key, int S_Box[8][4][16])
{
	b32 temp;
	// 32 位数据先经过 E 表置换成 48 位
	b48 e_per = permutation<32, 48>(R, E);
	// 置换得到数据与子密钥疑惑
	b48 num_xor = e_per ^ key;
	b32 S;
	int x = 0;
	// S 盒映射
	for (int i = 0; i< 48; i = i + 6)
	{
		int row = num_xor[47 - i] * 2 + num_xor[47 - i - 5];
		int col = num_xor[47 - i - 1] * 8 + num_xor[47 - i - 2] * 4 + num_xor[47 - i - 3] * 2 + num_xor[47 - i - 4];
		int num = S_Box[i / 6][row][col];
		bitset<4> binary(num);
		S[31 - x] = binary[3];
		S[31 - x - 1] = binary[2];
		S[31 - x - 2] = binary[1];
		S[31 - x - 3] = binary[0];
		x += 4;
	}
	// P表置换
	temp = permutation<32, 32>(S, P);
	return temp;
}



