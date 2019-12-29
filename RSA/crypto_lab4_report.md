# Lab4 公钥密码算法 RSA

## 实验目的

通过实际编程了解公钥密码算法 RSA 的加密和解密过程，加深对公钥密码算法的了解和使用。

## 实验内容

RSA 算法原理如下：

1. 公钥：选择两个不同的大素数 $p$ 和 $q$ ，$n=pq$ 。随机选取正整数 $e$ ，使其满足 $(e,\varphi(n)) =1$ 。$(n,e)$ 作为公钥。
2. 私钥：求正数 $d$ ，使其满足 $e\times d \equiv 1\ mod\ \varphi(n)$，$(n, d)$ 作为私钥。
3. 加密算法：对于明文 $m$，对应的密文 $c$ ，$c \equiv m^e mod\ n$。
4. 解密算法：对于密文 $c$ ，对应的明文 $m$ ， $m\equiv c^d mod\ n$。

$$例：p = 3, q = 11, m = 2\ 则\ n=pq=33, \varphi(n)=(p-1)(q-1)=20,\\ \quad 随机选取e=3满足(e,\varphi(n))=1，则计算 d = 7 满足 e \times d \equiv 1\ mod\ \varphi(n)\\ \quad 则计算密文 c \equiv m^e \equiv 8\ mod\ n \quad 解密m \equiv c^d \equiv 2\ mod\ n $$

### GMP 库

GMP(the GNU Multiple Precision arithmetic library) 提供了任意精度的算数运算，本实验用 GMP 库实现  512 位大素数的生成和运算。[GMP 库的官方文档](https://gmplib.org/manual/)

使用 `mpz_t` 多精度整型运算。

### 密钥对生成

$e$ 一般选用 $3, 17, 65537$。

$p, q$ 两个素数的生成过程如下：

1. `mpz_urandomb(p, rstate, MODULUS_SIZE/2)`  生成一个随机数

2. `mpz_even_p(p)` 判断是否为偶数，如果为偶数就 +1 得到一个奇数。

3. `mpz_probab_prime_p(p, 25)` 进行素性检验，这是 gmp 内置的函数。一般的素性检验方法时使用 Miller-Rabin 算法，伪代码如下：

   ```C
   bool Miller_Rabin(int n, int times) {
       int t = n - 1, s = 0;
   	if(n<2)	return false; 	// 0,1
       if(n=2)	return true;	// 2 是素数
       if(!n&1) return true;	// 偶数
       // 将 n-1 分解为 n-1 = t * 2^s, t 为奇数
       while(!(t&1)) {
           s++;
           t>>=1;
       }
       int i, j, k, a, b;
       for(i = 0; i < times; i++) {
           a = rand()%(n-3) + 2; // 1 < a < n-1
          	b = pow(a, t)%n;
           for(j = 1; j <= s; j++) {
               k = pow(b, 2)%n;
               // 二次探测定理
               if(k == 1 && b != 1 && b != x-1)
                   return false;
               b = k;
           }
           if(b != 1) return false;
       }
       // 如果多次测试正确则认为 x 可能是素数。
       return true;
   }
   ```

4. 当生成的奇数是素数是，则使用此数，否则 +2 探测相邻的奇数，直到满足条件。

### 加密

生成了密钥对之后，加密操作就是简单的模幂运算：$c\equiv\ m^e\ mod\ n $

```C
void rsa_encrypt(mpz_t cipher, public_key kpu, int message) {
    mpz_t m, c;
    mpz_init_set_ui(m, message);
    mpz_init(c);
    // 模幂计算
    mpz_powm(c, m, kpu.e, kpu.n);
    mpz_set(cipher, c);
    mpz_clear(m);
    mpz_clear(c);
}
```

### 解密

解密过程需要先求 $e$ 模 $n$ 的逆元 $d$ ，使用扩展欧几里得算法，然后再进行模幂运算 $m \equiv\ c^d\ mod\ n$

```C
// 计算 e 对 phi 的乘法逆元 ed = 1 mod phi
mpz_invert(kpr->d, kpr->e, phi);

void rsa_decrypt(mpz_t message, private_key kpr, mpz_t cipher) {
    mpz_t m; mpz_init(m);
    mpz_powm(m, cipher, kpr.d, kpr.n);
    mpz_set(message, m);
    mpz_clear(m);
}
```

