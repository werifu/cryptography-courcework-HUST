#include <gmp.h>
#include <stdio.h>

using namespace std;

void expmod(mpz_t result, mpz_t m, mpz_t e, mpz_t p, mpz_t q) {
    mpz_t n;
    mpz_init(n);
    mpz_mul(n, p, q);
    mpz_t tmp_m;
    mpz_init_set(tmp_m, m);

    mpz_t r;
    mpz_init_set_ui(r, 1);

    int e_size = mpz_sizeinbase(e, 2);
    for (int i = 0; i < e_size; i++) {
        int bit = mpz_tstbit(e, i);
        if (bit == 0) {
            mpz_mod(r, r, n);
        } else {
            mpz_mul(r, r, tmp_m);
            mpz_mod(r, r, n);
        }
        mpz_mul(tmp_m, tmp_m, tmp_m);
        mpz_mod(tmp_m, tmp_m, n);
    }
    mpz_set(result, r);
}

int main() {
    // freopen("./4.in", "r", stdin);
    int n;
    scanf("%d", &n);
    mpz_t result, m, e, p, q;
    for (int i_ = 0; i_ < n; i_++) {
        // e_buf & m_buf should bigger than 1024 QAQ
        char e_buf[2048], m_buf[2048], q_buf[1024], p_buf[1024];
        scanf("%2048s %2048s %1024s %1024s", e_buf, m_buf, p_buf, q_buf);
        mpz_init_set_str(e, e_buf, 10);
        mpz_init_set_str(m, m_buf, 10);
        mpz_init_set_str(p, p_buf, 10);
        mpz_init_set_str(q, q_buf, 10);
        mpz_init(result);
        expmod(result, m, e, p, q);
        gmp_printf("%Zd\n", result);
    }
    return 0;
}