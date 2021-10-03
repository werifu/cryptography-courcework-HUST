#include <gmp.h>
#include <stdio.h>

using namespace std;

void gcd(mpz_t result, mpz_t a, mpz_t b) {
    mpz_t tmp;
    mpz_t backup_a, backup_b;
    mpz_init_set(backup_a, a);
    mpz_init_set(backup_b, b);
    mpz_init(tmp);
    // if a < b then swap(a, b) to garauntee a is the bigger one
    if (mpz_cmp(a, b) < 0) {
        mpz_swap(a, b);
    }
    // a = ?b + tmp;
    mpz_mod(tmp, a, b);
    while (mpz_cmp_ui(tmp, 0) != 0) {
        mpz_set(a, b);
        mpz_set(b, tmp);
        mpz_mod(tmp, a, b);
    }
    mpz_set(result, b);
    mpz_set(a, backup_a);
    mpz_set(b, backup_b);
}

bool gcdIsOne(mpz_t a, mpz_t b) {
    mpz_t result;
    mpz_init(result);
    gcd(result, a, b);
    if (mpz_cmp_ui(result, 1) != 0) {
        return false;
    }
    return true;
}

bool gcdIsBig(mpz_t a, mpz_t b) {
    mpz_t tmp, backup_a, backup_b;
    mpz_init(tmp);
    mpz_init_set(backup_a, a);
    mpz_init_set(backup_b, b);
    mpz_mod(tmp, a, b);
    while (mpz_cmp_ui(tmp, 0) != 0) {
        mpz_set(a, b);
        mpz_set(b, tmp);
        mpz_mod(tmp, a, b);
    }

    mpz_set(a, backup_a);
    if (mpz_cmp_ui(b, 100000) > 0) {
        mpz_set(b, backup_b);
        return true;
    } else {
        mpz_set(b, backup_b);
        return false;
    }
}
// set result to e^-1 mod phi
void invert(mpz_t result, mpz_t e, mpz_t phi) {
    mpz_t st[5000];
    for (int i = 0; i < 5000; i++) {
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
    // mpz_mod(result, tmp1, phi);
    mpz_init_set(result, tmp1);
}

// e^-1 mod phi(pq) == d
void calculateD(mpz_t e, mpz_t p, mpz_t q) {
    // verify the params

    mpz_t phi, tmp, tmp1, tmp2;
    mpz_init(phi);
    mpz_init(tmp);
    mpz_init(tmp1);
    mpz_init(tmp2);
    mpz_sub(tmp, p, q);
    mpz_div_ui(tmp1, p, 10);
    mpz_abs(tmp, tmp);
    if (mpz_cmp_ui(e, 10) < 0) {  // e must >= 10
        printf("ERROR\n");
        return;
    }
    if (mpz_probab_prime_p(p, 35) == 0 ||
        mpz_probab_prime_p(q, 35) == 0) {  // p, q not prime
        printf("ERROR\n");
        return;
    }
    if (mpz_cmp(tmp, tmp1) < 0) {  // p,q to close
        printf("ERROR\n");
        return;
    }
    mpz_sub_ui(p, p, 1);
    mpz_sub_ui(q, q, 1);
    if (gcdIsBig(p, q)) {
        printf("ERROR\n");
        return;
    }
    // phi(pq) = (p-1)*(q-1)
    mpz_mul(phi, p, q);
    mpz_set_ui(tmp, 0);
    mpz_sub(phi, tmp, phi);
    if (!gcdIsOne(e, phi)) {  // gcd(e, phi) != 1
        printf("ERROR\n");
        return;
    }
    // the params are valid, then calculate the d
    invert(tmp1, e, phi);
    gmp_printf("%Zd\n", tmp1);
}

void work() {
    // mpz_t e;
    char eBuf[1024], qBuf[1024], pBuf[1024];
    scanf("%1023s %1023s %1023s", eBuf, pBuf, qBuf);
    mpz_t e, p, q;
    mpz_init_set_str(e, eBuf, 10);
    mpz_init_set_str(p, pBuf, 10);
    mpz_init_set_str(q, qBuf, 10);

    if (mpz_cmp(p, q) < 0) {
        mpz_swap(p, q);
    }
    calculateD(e, p, q);
}

int main() {
    int n;
    scanf("%d", &n);
    getchar();
    for (int i_ = 0; i_ < n; i_++) {
        work();
    }
    return 0;
}