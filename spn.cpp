#include <stdio.h>

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#define ROUND 4
typedef unsigned int uint;
typedef unsigned char uchar;

using namespace std;

uint sbox[16];
uint pbox[16];
uint keyMap[6];

uint allSbox[65536];
uint allRevSbox[65536];
uint allPbox[65536];
uint allRevPbox[65536];

uint spbox[65536];
uint revSpbox[65536];

void initSbox();
void initPbox();
void initSPbox();
void initReverseSbox();
void initReversePbox();
void initKeyMap();

uint decrypt(uint key, uint cipher);
uint encrypt(uint key, uint plain);

// grouped by 4 sbox
inline uint putAllSbox(uint num, uint* sb);
inline uint putPbox(uint num, uint* pb);
inline void write(uint num);
inline uint read(uint chars);

inline uint read(uint chars) {
    uint x = 0;
    char ch = 0;
    for (int i = 0; i < chars; i++) {
        ch = getchar();
        if (ch >= '0' && ch <= '9') {
            x = (x << 4) + (ch - '0');
        } else {
            x = (x << 4) + ch - 'a' + 10;
        }
    }
    return x;
}

inline void write(uint num) {
    for (int i = 3; i >= 0; i--) {
        uint hex = (num >> (i * 4)) & 0xf;
        if (hex >= 0 && hex <= 9) {
            putchar(hex + '0');
        } else {
            putchar(hex - 10 + 'a');
        }
    }
}
// abcd abcd
// 10 chars
inline void writeStream(uint num1, uint num2) {
    char pbuf[10];
    for (int i = 3; i >= 0; i--) {
        uint hex = (num1 >> (i * 4)) & 0xf;
        if (hex >= 0 && hex <= 9) {
            pbuf[3 - i] = hex + '0';
        } else {
            pbuf[3 - i] = hex - 10 + 'a';
        }
    }
    pbuf[4] = ' ';
    for (int i = 3; i >= 0; i--) {
        uint hex = (num2 >> (i * 4)) & 0xf;
        if (hex >= 0 && hex <= 9) {
            pbuf[8 - i] = hex + '0';
        } else {
            pbuf[8 - i] = hex - 10 + 'a';
        }
    }
    pbuf[9] = '\n';
    fwrite(pbuf, 1, 10, stdout);
}
inline uint putAllSbox(uint num, uint* sbox_) { return sbox_[num]; }

inline uint putPbox(uint num, uint* pbox_) { return pbox_[num]; }

void initKeyMap(uint key) {
    for (int i = 5; i > 0; i--) {
        keyMap[i] = key & 0xffff;
        key >>= 4;
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
    vr = putAllSbox(ur, allSbox);
    uint result = vr ^ keyMap[ROUND + 1];
    return result;
}

uint decrypt(uint key, uint cipher) {
    uint wr = cipher;
    uint kr, ur, vr;
    vr = cipher ^ keyMap[ROUND + 1];
    ur = putAllSbox(vr, allRevSbox);
    wr = ur ^ keyMap[ROUND];
    for (int r = ROUND - 1; r > 0; r--) {
        kr = keyMap[r];
        vr = putPbox(wr, allRevPbox);
        ur = putAllSbox(vr, allRevSbox);
        wr = ur ^ kr;
    }
    return wr;
}

int main() {
    initSbox();
    initPbox();
    initSPbox();
    int n;
    scanf("%d", &n);
    getchar();
    for (int i = 0; i < n; i++) {
        uint key = read(8);
        getchar();
        uint plain = read(4);
        getchar();
        // uint key, plain;
        // readStream(&key, &plain);

        initKeyMap(key);
        uint curPlain = plain;
        uint curCipher = encrypt(key, plain);
        uint nextPlain = decrypt(key, curCipher ^ 1);

        writeStream(curCipher, nextPlain);
    }
    return 0;
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
        for (int j = 0; j < 16; j++) {
            for (int k = 0; k < 16; k++) {
                for (int l = 0; l < 16; l++) {
                    uint key = (i << 12) | (j << 8) | (k << 4) | l;
                    uint val = (sbox[i] << 12) | (sbox[j] << 8) |
                               (sbox[k] << 4) | sbox[l];
                    allSbox[key] = val;
                    allRevSbox[val] = key;
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
        allRevPbox[result] = num;
    }
}

void initSPbox() {
    for (int i = 0; i < 65536; i++) {
        spbox[i] = allPbox[allSbox[i]];
    }
}