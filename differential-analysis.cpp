#include <stdio.h>
#include <string.h>

#include <algorithm>
#define ROUND 4
typedef unsigned int uint;

using namespace std;
uint sbox[16] = {14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7};
uint rsbox[16] = {14, 3, 4, 8, 1, 12, 10, 15, 7, 13, 9, 6, 11, 2, 0, 5};
uint pbox[16] = {0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15};
uint allSbox[65536];
uint allPbox[65536];
uint spbox[65536];
char readBuf[65536 * 5];
uint ciphers[65536];
void initSPbox() {
    for (int i = 0; i < 65536; i++) {
        spbox[i] = allPbox[allSbox[i]];
    }
}

inline void readAll() {
    int cur = 0;
    char c1, c2, c3, c4;
    for (int i = 0; i < 65536; i++) {
        uint x = 0;
        c1 = readBuf[cur];
        c2 = readBuf[cur + 1];
        c3 = readBuf[cur + 2];
        c4 = readBuf[cur + 3];
        x = ((c1 >= 'a') ? (c1 - 'a' + 10) : (c1 - '0'));
        x = (x << 4) | ((c2 >= 'a') ? (c2 - 'a' + 10) : (c2 - '0'));
        x = (x << 4) | ((c3 >= 'a') ? (c3 - 'a' + 10) : (c3 - '0'));
        x = (x << 4) | ((c4 >= 'a') ? (c4 - 'a' + 10) : (c4 - '0'));
        ciphers[i] = x;
        cur += 5;
    }
}
// the first chain
// 0000 1011 0000 0000
uint xor_x = 0x0b00;
// 0000 0110 0000 0110
uint xor_u = 0x0606;

// the second chain
// 0000 0000 0101 0000
uint xor_x2 = 0x0050;
// 0101 0000 0101 0000
uint xor_u2 = 0x5050;

// used to verify the keys
uint keyMap[6];
void initKeyMap(uint key) {
    for (int i = 5; i > 0; i--) {
        keyMap[i] = key & 0xffff;
        key >>= 4;
    }
}
void initSbox() {
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
uint encrypt(uint key, uint plain) {
    uint wr = plain;
    uint kr, ur, vr;
    uint times = ROUND - 1;
    for (int r = 1; r <= times; r++) {
        kr = keyMap[r];
        ur = wr ^ kr;
        wr = spbox[ur];
    }
    ur = wr ^ keyMap[ROUND];
    vr = allSbox[ur];
    uint result = vr ^ keyMap[ROUND + 1];
    return result;
}

void work() {
    uint v4_2, v4_4, u4_2, u4_4, v4_2_star, v4_4_star, u4_2_star, u4_4_star,
        u4_2_xor, u4_4_xor;
    uint v4_1, v4_3, u4_1, u4_3, v4_1_star, v4_3_star, u4_1_star, u4_3_star,
        u4_1_xor, u4_3_xor;
    fread(readBuf, 5, 65536, stdin);
    readAll();
    int count1[16][16];
    int count2[16][16];
    memset(count1, 0, sizeof(count1));
    memset(count2, 0, sizeof(count2));
    for (uint x = 0; x < 65536; x++) {
        uint y = ciphers[x];
        uint x_star = x ^ xor_x;
        uint y_star = ciphers[x_star];
        // invalid pair
        if ((y >> 12) == (y_star >> 12) &&
            ((y >> 4) & 0xf) == ((y_star >> 4) & 0xf)) {
            for (int l1 = 0; l1 < 16; l1++) {
                for (int l2 = 0; l2 < 16; l2++) {
                    v4_2 = l1 ^ ((y >> 8) & 0xf);
                    v4_4 = l2 ^ (y & 0xf);
                    u4_2 = rsbox[v4_2];
                    u4_4 = rsbox[v4_4];
                    v4_2_star = l1 ^ ((y_star >> 8) & 0xf);
                    v4_4_star = l2 ^ (y_star & 0xf);
                    u4_2_star = rsbox[v4_2_star];
                    u4_4_star = rsbox[v4_4_star];
                    u4_2_xor = u4_2 ^ u4_2_star;
                    u4_4_xor = u4_4 ^ u4_4_star;
                    if (u4_2_xor == ((xor_u >> 8) & 0xf) &&
                        u4_4_xor == (xor_u & 0xf)) {
                        count1[l1][l2]++;
                    }
                }
            }
        }
    }
    for (uint x = 0; x < 65536; x++) {
        // the 2nd chain
        uint y = ciphers[x];
        uint x_star = x ^ xor_x2;
        uint y_star = ciphers[x_star];
        if (((y >> 8) & 0xf) == ((y_star >> 8) & 0xf) &&
            (y & 0xf) == (y_star & 0xf)) {
            for (int l1 = 0; l1 < 16; l1++) {
                for (int l2 = 0; l2 < 16; l2++) {
                    v4_1 = l1 ^ ((y >> 12) & 0xf);
                    v4_3 = l2 ^ ((y >> 4) & 0xf);
                    u4_1 = rsbox[v4_1];
                    u4_3 = rsbox[v4_3];
                    v4_1_star = l1 ^ ((y_star >> 12) & 0xf);
                    v4_3_star = l2 ^ ((y_star >> 4) & 0xf);
                    u4_1_star = rsbox[v4_1_star];
                    u4_3_star = rsbox[v4_3_star];
                    u4_1_xor = u4_1 ^ u4_1_star;
                    u4_3_xor = u4_3 ^ u4_3_star;
                    if (u4_1_xor == ((xor_u2 >> 12) & 0xf) &&
                        u4_3_xor == ((xor_u2 >> 4) & 0xf)) {
                        count2[l1][l2]++;
                    }
                }
            }
        }
    }
    uint sorted_key24[256];
    uint sorted_key13[256];
    // find the (l1 l2) which leads to the largest deviation
    for (int l1 = 0; l1 < 16; l1++) {
        for (int l2 = 0; l2 < 16; l2++) {
            sorted_key24[(l1 << 4) | l2] = (l1 << 4) | l2;
        }
    }
    sort(sorted_key24, sorted_key24 + 256, [&](uint a, uint b) {
        return count1[a >> 4][a & 0xf] > count1[b >> 4][b & 0xf];
    });
    // find the (l1 l2) which leads to the largest deviation
    for (int l1 = 0; l1 < 16; l1++) {
        for (int l2 = 0; l2 < 16; l2++) {
            sorted_key13[(l1 << 4) | l2] = (l1 << 4) | l2;
        }
    }
    sort(sorted_key13, sorted_key13 + 256, [&](uint a, uint b) {
        return count2[a >> 4][a & 0xf] > count2[b >> 4][b & 0xf];
    });
    uint block2, block4;
    bool foundFlag = false;
    for (int i = 0; i < 256; i++) {
        block2 = (sorted_key24[i] >> 4) & 0xf;
        block4 = sorted_key24[i] & 0xf;
        for (int j = 0; j < 30; j++) {
            uint block1 = (sorted_key13[j] >> 4) & 0xf;
            uint block3 = sorted_key13[j] & 0xf;
            for (int highKey = 0; highKey < 65536; highKey++) {
                uint wholeKey = (highKey << 16) | (block1 << 12) |
                                (block2 << 8) | (block3 << 4) | block4;
                initKeyMap(wholeKey);
                if (encrypt(wholeKey, 0x1145) != ciphers[0x1145]) continue;
                if (encrypt(wholeKey, 0x1919) != ciphers[0x1919]) continue;
                printf("%08x\n", wholeKey);
                foundFlag = true;
                break;
            }
            if (foundFlag) break;
        }
        if (foundFlag) break;
    }
}

int main() {
    // freopen("./encryption/3/1.in", "r", stdin);
    initSbox();
    initPbox();
    initSPbox();
    int n;
    scanf("%d", &n);
    getchar();
    for (int _i = 0; _i < n; _i++) {
        work();
    }
    return 0;
}