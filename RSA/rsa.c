#include <stdlib.h>
#include <stdio.h>
#include <gmp.h>
#include <time.h>

#define MODULUS_SIZE 1024       // 模数 n 的位数
#define MESSAGE_SIZE 100

typedef struct {
    mpz_t n;
    mpz_t e;
}public_key;

typedef struct {
    mpz_t n;
    mpz_t e;
    mpz_t d;
    mpz_t p;
    mpz_t q;
}private_key;

// 生成密钥对
void generate_key(private_key *kpr, public_key *kpu) {
    int i;
    time_t seed;
    gmp_randstate_t rstate;
    seed = time(NULL);
    gmp_randinit_default(rstate);
    gmp_randseed_ui(rstate, seed);

    // 生成素数
    mpz_t p, q; mpz_init(p); mpz_init(q);
    // 首先生成两个随机数
    mpz_urandomb(p, rstate, MODULUS_SIZE/2);
    mpz_urandomb(q, rstate, MODULUS_SIZE/2);
    // 如果为偶数就 +1 变为奇数
    if(mpz_even_p(p)) {
        mpz_add_ui(p, p, 1);
    }
    if(mpz_even_p(q)) {
        mpz_add_ui(q, q, 1);
    }
    // 对生成的大奇数进行 Miller-Rabin 素性检验，不是奇数则找相邻的奇数
    while(!mpz_probab_prime_p(p, 25) > 0) {
        mpz_add_ui(p, p, 2);
    }
    while(!mpz_probab_prime_p(q, 25) > 0) {
        mpz_add_ui(q, q, 2);
    }
    mpz_set(kpr->p, p);
    mpz_set(kpr->q, q);
    
    // 计算 n = pq
    mpz_mul(kpr->n, p, q);
    mpz_mul(kpu->n, p, q);
    
    // 计算 phi = (p-1)(q-1)
    mpz_t phi; mpz_init(phi);
    mpz_sub_ui(p, p, 1); mpz_sub_ui(q, q, 1);
    mpz_mul(phi, p, q);

    // 设置 e 为 65537，一般就是 3、17、65537
    mpz_set_ui(kpu->e, 65537);
    mpz_set_ui(kpr->e, 65537);

    // 计算 e 对 phi 的乘法逆元 ed = 1 mod phi
    mpz_invert(kpr->d, kpr->e, phi);

    mpz_clear(q);
    mpz_clear(p);
    mpz_clear(phi);
}

void rsa_encrypt(mpz_t cipher, public_key kpu, long long message) {
    mpz_t m, c;
    mpz_init_set_ui(m, message);
    mpz_init(c);
    // 模幂计算
    mpz_powm(c, m, kpu.e, kpu.n);
    mpz_set(cipher, c);
    mpz_clear(m);
    mpz_clear(c);
}

void rsa_decrypt(mpz_t message, private_key kpr, mpz_t cipher) {
    mpz_t m; mpz_init(m);
    mpz_powm(m, cipher, kpr.d, kpr.n);
    mpz_set(message, m);
    mpz_clear(m);
}

int main() {
    public_key kpu; 
    mpz_init(kpu.n); mpz_init(kpu.e);
    private_key kpr; 
    mpz_init(kpr.n); mpz_init(kpr.e); mpz_init(kpr.p); mpz_init(kpr.q); mpz_init(kpr.d);
    // 密钥对生成
    printf("Generating Keys:\n");
    generate_key(&kpr, &kpu);
    gmp_printf("p:%ZX\n", kpr.p);
    gmp_printf("q:%ZX\n", kpr.q);
    gmp_printf("n:%ZX\n", kpr.n);
    gmp_printf("e:%Zd\n", kpu.e);
    gmp_printf("d:%ZX\n", kpr.d);

    long long m;
    printf("Please input message to encrypt: ");
    scanf("%lld", &m);
    mpz_t cipher; mpz_init(cipher);
    rsa_encrypt(cipher, kpu, m);
    gmp_printf("cipher: %ZX\n", cipher);
    mpz_t message; mpz_init(message);
    rsa_decrypt(message, kpr, cipher);
    gmp_printf("message: %Zd\n", message);
    return 0;
}
