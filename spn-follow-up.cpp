#include <stdio.h>
#include <string.h>
#define KEYBYTES (16)
#define INPUTBYTES (1 << 24)
#define SPNBYTES (8)
#define ONEBYTE (8)
#define ROUND 3
using namespace std;
typedef unsigned int uint;
typedef unsigned long long ull;
typedef unsigned char byte;

uint sbox16[16] = {14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7};
uint sbox[256];
ull p[64] = {0,  16, 32, 48, 1,  17, 33, 49, 2,  18, 34, 50, 3,  19, 35, 51,
             4,  20, 36, 52, 5,  21, 37, 53, 6,  22, 38, 54, 7,  23, 39, 55,
             8,  24, 40, 56, 9,  25, 41, 57, 10, 26, 42, 58, 11, 27, 43, 59,
             12, 28, 44, 60, 13, 29, 45, 61, 14, 30, 46, 62, 15, 31, 47, 63};

ull keyMap[8];
// use a 8bits sbox
// refers to https://en.wikipedia.org/wiki/Rijndael_S-box
#define ROTL8(x, shift) ((byte)((x) << (shift)) | ((x) >> (8 - (shift))))
void initialize_aes_sbox() {
    byte p = 1, q = 1;

    /* loop invariant: p * q == 1 in the Galois field */
    do {
        /* multiply p by 3 */
        p = p ^ (p << 1) ^ (p & 0x80 ? 0x1B : 0);

        /* divide q by 3 (equals multiplication by 0xf6) */
        q ^= q << 1;
        q ^= q << 2;
        q ^= q << 4;
        q ^= q & 0x80 ? 0x09 : 0;

        /* compute the affine transformation */
        byte xformed =
            q ^ ROTL8(q, 1) ^ ROTL8(q, 2) ^ ROTL8(q, 3) ^ ROTL8(q, 4);

        sbox[p] = xformed ^ 0x63;
    } while (p != 1);

    /* 0 is a special case since it has no inverse */
    sbox[0] = 0x63;
}

void initKeyMap(byte keyBuf[]) {
    memset(keyMap, 0, sizeof(keyMap));
    for (int i = 1; i < 8; i++) {
        for (int j = i; j < i + 8; j++) {
            keyMap[i] <<= 8;
            keyMap[i] |= keyBuf[j];
        }
    }
}

ull substitute(ull x) {
    ull result = 0;
    for (int i = 0; i < 8; i++) {
        result <<= ONEBYTE;
        result |= sbox[(x >> (56 - 8 * i)) & 0xff];
    }
    return result;
}

ull permutate(ull plaintext) {
    ull mask = 0x8000000000000000;
    ull result = 0;
    for (int i = 0; i < 64; i++) {
        result |= (((plaintext & mask) << i) >> p[i]);
        mask >>= 1;
    }
    return result;
}

// key has been initialized
ull encrypt(ull x) {
    ull wr = x;
    ull kr, ur, vr;
    ull times = ROUND - 1;
    for (int r = 1; r <= times; r++) {
        kr = keyMap[r];
        ur = wr ^ kr;
        vr = substitute(ur);
        wr = permutate(vr);
    }
    ur = wr ^ keyMap[ROUND];
    vr = substitute(ur);
    ull result = vr ^ keyMap[ROUND + 1];
    return result;
}

int main() {
    initialize_aes_sbox();

    // freopen("./encryption/4/1.in", "rb", stdin);
    byte keyBuf[16];
    fread(&keyBuf, KEYBYTES, 1, stdin);
    int spnCount = INPUTBYTES / SPNBYTES;
    ull x;
    initKeyMap(keyBuf);
    ull y = 0xf3a5cde587582810;
    for (int i = 0; i < spnCount; i++) {
        fread(&x, SPNBYTES, 1, stdin);
        y = encrypt(x ^ y);
        fwrite(&y, SPNBYTES, 1, stdout);
    }
    return 0;
}
