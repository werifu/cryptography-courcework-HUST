#include <stdio.h>
#include <string.h>

#include <algorithm>
#define ROUND 4
#define makeAbs(a)        \
    {                     \
        if (a >= 4000)    \
            a -= 4000;    \
        else              \
            a = 4000 - a; \
    }

using namespace std;
typedef unsigned int uint;

inline uint read();

uint sbox[16];
uint pbox[16];
uint rsbox[16];
uint rpbox[16];
uint allSbox[65536];
uint allPbox[65536];
uint spbox[65536];
// init
void initSbox();
void initPbox();
void initAllSbox();
void initAllPbox();
void initSPbox();
uint plains[8000];
uint ciphers[8000];

// used to verify the keys
uint keyMap[6];
void initKeyMap(uint key) {
    for (int i = 5; i > 0; i--) {
        keyMap[i] = key & 0xffff;
        key >>= 4;
    }
}
uint encrypt(uint key, uint plain);
inline uint getRoundKey(uint key, uint round) { return keyMap[round]; }
inline uint putAllSbox(uint num, uint* sbox_) { return sbox_[num]; }
int main() {
    initSbox();
    initPbox();
    initSPbox();

    int n;
    scanf("%d", &n);
    getchar();
    for (int i_ = 0; i_ < n; i_++) {
        // 2 chains need 2 counters
        uint count[16][16];
        uint count2[16][16];
        memset(count, 0, sizeof(count));
        memset(count2, 0, sizeof(count2));
        for (int i = 0; i < 8000; i++) {
            uint x = read();
            getchar();
            uint y = read();
            getchar();
            plains[i] = x;
            ciphers[i] = y;
            // (l1 l2) is the last round key
            for (int l1 = 0; l1 < 16; l1++) {
                for (int l2 = 0; l2 < 16; l2++) {
                    uint y2 = (y >> 8) & 0xf;
                    uint v4_2 = l1 ^ y2;
                    uint y4 = y & 0xf;
                    uint v4_4 = l2 ^ y4;
                    uint u4_2 = rsbox[v4_2];
                    uint u4_4 = rsbox[v4_4];
                    // one chain from the textbook:
                    // X_5 ^ X_7 ^ X_8 ^ U4_6 ^ U4_8 ^ U4_14 ^ U4_16
                    // its ksai is ±1/32
                    bool z = ((x >> 11) & 1) ^ ((x >> 9) & 1) ^ ((x >> 8) & 1) ^
                             ((u4_2 >> 2) & 1) ^ ((u4_2)&1) ^
                             ((u4_4 >> 2) & 1) ^ ((u4_4)&1);
                    if (z == 0) {
                        count[l1][l2]++;
                    }
                }
            }
        }
        int maxCount = 0;

        // one block means 4bits
        uint block2 = 0;
        uint block4 = 0;
        uint sorted_key24[256];
        uint sorted_key13[256];
        // find the (l1 l2) which leads to the largest deviation
        for (int l1 = 0; l1 < 16; l1++) {
            for (int l2 = 0; l2 < 16; l2++) {
                makeAbs(count[l1][l2]);
                sorted_key24[(l1 << 4) | l2] = (l1 << 4) | l2;
            }
        }
        sort(sorted_key24, sorted_key24 + 256, [=](uint a, uint b) {
            return count[a >> 4][a & 0xf] > count[b >> 4][b & 0xf];
        });
        block2 = sorted_key24[0] >> 4;
        // The second chain
        // X_5 ^ X_6 ^ X_7 ^ X_8 ^ U4_2 ^ U4_4 ^ U4_6 ^ U4_8 ^ U4_ 10 ^
        // U4_12
        for (int i = 0; i < 8000; i++) {
            uint x = plains[i];
            uint y = ciphers[i];
            for (int l1 = 0; l1 < 16; l1++) {
                for (int l2 = 0; l2 < 16; l2++) {
                    uint y1 = (y >> 12) & 0xf;
                    uint y2 = (y >> 8) & 0xf;
                    uint y3 = (y >> 4) & 0xf;
                    uint v4_1 = l1 ^ y1;
                    uint v4_2 = block2 ^ y2;
                    uint v4_3 = l2 ^ y3;
                    uint u4_1 = rsbox[v4_1];
                    uint u4_2 = rsbox[v4_2];
                    uint u4_3 = rsbox[v4_3];
                    bool z = ((x >> 11) & 1) ^ ((x >> 10) & 1) ^
                             ((x >> 9) & 1) ^ ((x >> 8) & 1) ^
                             ((u4_1 >> 2) & 1) ^ (u4_1 & 1) ^
                             ((u4_2 >> 2) & 1) ^ (u4_2 & 1) ^
                             ((u4_3 >> 2) & 1) ^ (u4_3 & 1);
                    if (z == 0) {
                        count2[l1][l2]++;
                    }
                }
            }
        }
        uint block1 = 0;
        uint block3 = 0;
        maxCount = 0;
        for (int l1 = 0; l1 < 16; l1++) {
            for (int l2 = 0; l2 < 16; l2++) {
                makeAbs(count2[l1][l2]);
                sorted_key13[(l1 << 4) | l2] = (l1 << 4) | l2;
            }
        }
        sort(sorted_key13, sorted_key13 + 256, [=](uint a, uint b) {
            return count2[a >> 4][a & 0xf] > count2[b >> 4][b & 0xf];
        });
        // travers all the keys
        // key = (???? ???? ???? ???? block1 blcok2 block3 block4)
        // because the K5 is the low 16 bits
        for (int b24 = 0; b24 < 5; b24++) {
            block2 = sorted_key24[b24] >> 4;
            block4 = sorted_key24[b24] & 0xf;
            for (int b13 = 0; b13 < 5; b13++) {
                block1 = sorted_key13[b13] >> 4;
                block3 = sorted_key13[b13] & 0xf;

                for (int i = 0; i < 65536; i++) {
                    uint wholeKey = (i << 16) | (block1 << 12) | (block2 << 8) |
                                    (block3 << 4) | block4;
                    initKeyMap(wholeKey);
                    if (encrypt(wholeKey, plains[0]) != ciphers[0]) continue;
                    if (encrypt(wholeKey, plains[1]) != ciphers[1]) continue;
                    if (encrypt(wholeKey, plains[2]) != ciphers[2]) continue;
                    printf("%08x\n", wholeKey);
                    break;
                }
            }
        }
    }
    return 0;
}

inline uint read() {
    uint x = 0;
    char ch = 0;
    for (int i = 0; i < 4; i++) {
        ch = getchar();
        if (ch >= '0' && ch <= '9') {
            x = (x << 4) + (ch - '0');
        } else {
            x = (x << 4) + ch - 'a' + 10;
        }
    }
    return x;
}

void initSbox() {
    sbox[0] = 0xe;
    sbox[1] = 0x4;
    sbox[2] = 0xd;
    sbox[3] = 0x1;
    sbox[4] = 0x2;
    sbox[5] = 0xf;
    sbox[6] = 0xb;
    sbox[7] = 0x8;
    sbox[8] = 0x3;
    sbox[9] = 0xa;
    sbox[0xa] = 0x6;
    sbox[0xb] = 0xc;
    sbox[0xc] = 0x5;
    sbox[0xd] = 0x9;
    sbox[0xe] = 0x0;
    sbox[0xf] = 0x7;
    for (int i = 0; i < 16; i++) {
        rsbox[sbox[i]] = i;
    }
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            for (int k = 0; k < 16; k++) {
                for (int l = 0; l < 16; l++) {
                    uint key = (i << 12) | (j << 8) | (k << 4) | l;
                    uint val = (sbox[i] << 12) | (sbox[j] << 8) |
                               (sbox[k] << 4) | sbox[l];
                    allSbox[key] = val;
                }
            }
        }
    }
}

void initPbox() {
    pbox[0] = 0;
    pbox[1] = 4;
    pbox[2] = 8;
    pbox[3] = 12;
    pbox[4] = 1;
    pbox[5] = 5;
    pbox[6] = 9;
    pbox[7] = 13;
    pbox[8] = 2;
    pbox[9] = 6;
    pbox[0xa] = 10;
    pbox[0xb] = 14;
    pbox[0xc] = 3;
    pbox[0xd] = 7;
    pbox[0xe] = 11;
    pbox[0xf] = 15;
    for (int num = 0; num < 65536; num++) {
        bool pb[16];
        for (int i = 0; i < 16; i++) {
            pb[15 - i] = (num >> i) & 1;
        }
        uint result = 0;
        for (int i = 0; i < 16; i++) {
            result <<= 1;
            result |= pb[pbox[i]];
        }
        allPbox[num] = result;
    }
}

void initSPbox() {
    for (int i = 0; i < 65536; i++) {
        spbox[i] = allPbox[allSbox[i]];
    }
}

uint encrypt(uint key, uint plain) {
    uint wr = plain;
    uint kr, ur, vr;
    uint times = ROUND - 1;
    for (int r = 1; r <= times; r++) {
        kr = getRoundKey(key, r);
        ur = wr ^ kr;
        wr = spbox[ur];
    }
    ur = wr ^ getRoundKey(key, ROUND);
    vr = putAllSbox(ur, allSbox);
    uint result = vr ^ getRoundKey(key, ROUND + 1);
    return result;
}