#include <gmp.h>
#include <stdio.h>

using namespace std;
// set result to e^-1 mod phi
inline void invert(mpz_t result, mpz_t e_, mpz_t phi_) {
    mpz_t e, phi;
    mpz_init_set(phi, phi_);
    mpz_init_set(e, e_);
    mpz_t st[1000];
    for (int i = 0; i < 1000; i++) {
        mpz_init(st[i]);
    }
    mpz_t tmp, tmp1, tmp2;
    int count = -1;
    mpz_init(tmp);
    mpz_init(tmp1);
    mpz_init(tmp2);
    mpz_mod(tmp, e, phi);
    mpz_sub(tmp1, e, tmp);
    mpz_div(tmp1, tmp1, phi);
    mpz_set(st[0], tmp1);
    while (mpz_cmp_ui(tmp, 0) != 0) {
        count++;
        mpz_set(st[count], tmp1);
        mpz_set(e, phi);
        mpz_set(phi, tmp);
        mpz_mod(tmp, e, phi);
        mpz_sub(tmp1, e, tmp);
        mpz_div(tmp1, tmp1, phi);
    }
    mpz_set_ui(tmp1, 0);
    mpz_set_ui(tmp2, 1);
    mpz_sub(phi, tmp1, phi);
    for (int i = count; i >= 0; i--) {
        mpz_set(tmp, tmp2);
        mpz_mul(tmp2, tmp2, st[i]);
        mpz_sub(tmp2, tmp1, tmp2);
        mpz_set(tmp1, tmp);
    }
    mpz_set(result, tmp1);
}

// set result to m^e mod pq
// chinese remainder: {
//    a = m^e mod p
//    b = m^e mod q
// }
// m^e mod pq == (a*q*(q^-1(mod p)) + b*p*(p^-1(mod q))) mod n
inline void expmod(mpz_t result, mpz_t m, mpz_t e, mpz_t p, mpz_t q) {
    mpz_t n;
    mpz_init(n);
    mpz_mul(n, p, q);
    mpz_t tmp_m;
    mpz_init_set(tmp_m, m);

    mpz_t r;
    mpz_init_set_ui(r, 1);

    mpz_t a, b;
    mpz_init(a);
    mpz_init(b);

    // set a to m^e mod p
    mpz_powm(a, m, e, p);
    mpz_powm(b, m, e, q);

    mpz_t p_1, q_1;
    mpz_init(p_1);
    mpz_init(q_1);
    invert(p_1, p, q);
    invert(q_1, q, p);

    // asum = a*q*(q^-1 mod p)
    // bsum = b*p*(p^-1 mod q)
    mpz_t asum, bsum;
    mpz_init(asum);
    mpz_init(bsum);
    mpz_mul(asum, q, q_1);
    // mpz_mod(asum, asum, n);
    mpz_mul(asum, asum, a);

    mpz_mul(bsum, p, p_1);
    // mpz_mod(bsum, bsum, n);
    mpz_mul(bsum, bsum, b);

    mpz_add(result, asum, bsum);
    mpz_mod(result, result, n);
}

// set d to e^-1 mod phi(pq)
inline void calculateD(mpz_t d, mpz_t e_, mpz_t p_, mpz_t q_) {
    mpz_t backup_p, backup_q;
    mpz_t e, p, q;
    mpz_init_set(e, e_);
    mpz_init_set(p, p_);
    mpz_init_set(q, q_);

    mpz_t phi, tmp;
    mpz_init(tmp);
    mpz_init(phi);

    mpz_sub_ui(p, p, 1);
    mpz_sub_ui(q, q, 1);
    // phi(pq) = (p-1)*(q-1)
    mpz_mul(phi, p, q);

    mpz_set_ui(tmp, 0);
    mpz_sub(phi, tmp, phi);

    // the params are valid, then calculate the d
    invert(tmp, e, phi);
    mpz_mod(d, tmp, phi);
}

int main() {
    // freopen("./4.in", "r", stdin);
    int n;
    mpz_t result, c, e, p, q, d, tmp;
    mpz_init(tmp);
    mpz_init(c);
    char e_buf[2048], q_buf[1024], p_buf[1024];
    scanf("%d %1024s %1024s %2048s", &n, p_buf, q_buf, e_buf);
    mpz_init_set_str(e, e_buf, 10);
    mpz_init_set_str(p, p_buf, 10);
    mpz_init_set_str(q, q_buf, 10);
    mpz_init(result);
    mpz_init(d);
    for (int i_ = 0; i_ < n; i_++) {
        gmp_scanf("%Zd", c);

        // to calculate c^d(mod pq)
        // garauntee p > q
        if (mpz_cmp(p, q) < 0) {
            mpz_swap(p, q);
        }
        calculateD(d, e, p, q);
        expmod(result, c, d, p, q);

        gmp_printf("%Zd\n", result);
    }
    return 0;
}