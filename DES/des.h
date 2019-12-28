#pragma once
#include<iostream>
#include<bitset>

#define b64 bitset<64>
#define b56 bitset<56>
#define b28 bitset<28>
#define b48 bitset<48>
#define b32 bitset<32>
#define b8  bitset<8>
using namespace std;

class DES
{
	b64 k;
	b64 m;

public:
	DES(b64 k);
	b64 des_encrypt(b64 m);
	b64 des_decrypt(b64 c);
	b32 f(b32 R, int E[48], b48 Key, int S_Box[8][4][16]);
};
