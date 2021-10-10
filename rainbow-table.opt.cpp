#include <malloc.h>
#include <memory.h>
#include <stdio.h>

#include <unordered_map>
#include <vector>

#define DEBUG
#ifdef DEBUG
#include <chrono>
using namespace std::chrono;
#include <iostream>
#endif

using namespace std;
typedef uint uint;
uint SHA1_tmp;
unordered_map<string, char*> hmap;

#define SHA1_ROTL(a, b) \
    (SHA1_tmp = (a),    \
     ((SHA1_tmp >> (32 - b)) & (0x7fffffff >> (31 - b))) | (SHA1_tmp << b))

#define SHA1_F(B, C, D, t)                                        \
    ((t < 40) ? ((t < 20) ? ((B & C) | ((~B) & D)) : (B ^ C ^ D)) \
              : ((t < 60) ? ((B & C) | (B & D) | (C & D)) : (B ^ C ^ D)))

inline void UnitSHA1(const char* str, int length, uint sha1[5]) {
    /**
     * 计算字符串SHA-1
     * 参数说明：
     * str         字符串指针
     * length      字符串长度
     * sha1         用于保存SHA-1的字符串指针
     * 返回值为参数sha1
     */
    unsigned char *pp, *ppend;
    uint l, i, K[80], W[80], TEMP, A, B, C, D, E, H0, H1, H2, H3, H4;
    H0 = 0x67452301, H1 = 0xEFCDAB89, H2 = 0x98BADCFE, H3 = 0x10325476,
    H4 = 0xC3D2E1F0;
    for (i = 0; i < 20; K[i++] = 0x5A827999)
        ;
    for (i = 20; i < 40; K[i++] = 0x6ED9EBA1)
        ;
    for (i = 40; i < 60; K[i++] = 0x8F1BBCDC)
        ;
    for (i = 60; i < 80; K[i++] = 0xCA62C1D6)
        ;
    l = length +
        ((length % 64 > 56) ? (128 - length % 64) : (64 - length % 64));
    if (!(pp = (unsigned char*)malloc((uint)l))) return;
    for (i = 0; i < length; pp[i + 3 - 2 * (i % 4)] = str[i], i++)
        ;
    for (pp[i + 3 - 2 * (i % 4)] = 128, i++; i < l;
         pp[i + 3 - 2 * (i % 4)] = 0, i++)
        ;
    *((uint*)(pp + l - 4)) = length << 3;
    *((uint*)(pp + l - 8)) = length >> 29;
    for (ppend = pp + l; pp < ppend; pp += 64) {
        for (i = 0; i < 16; W[i] = ((uint*)pp)[i], i++)
            ;
        for (i = 16; i < 80;
             W[i] = SHA1_ROTL((W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]), 1),
            i++)
            ;
        A = H0, B = H1, C = H2, D = H3, E = H4;
        for (i = 0; i < 80; i++) {
            TEMP = SHA1_ROTL(A, 5) + SHA1_F(B, C, D, i) + E + W[i] + K[i];
            E = D, D = C, C = SHA1_ROTL(B, 30), B = A, A = TEMP;
        }
        H0 += A, H1 += B, H2 += C, H3 += D, H4 += E;
    }
    free(pp - l);
    sha1[0] = H0, sha1[1] = H1, sha1[2] = H2, sha1[3] = H3, sha1[4] = H4;
    return;
}
void getstr(uint n, char str[8]) {
    str[0] = 'a';
    str[1] = '0';
    str[2] = '0';
    str[3] = '0';
    str[4] = '0';
    str[5] = '0';
    str[6] = '0';
    str[7] = '0';

    int i = 2;
    while (n) {
        uint tmp = n % 36;
        if (tmp < 10) {
            str[i++] = tmp + '0';
        } else {
            str[i++] = tmp - 10 + 'a';
        }
        n /= 36;
    }
}
// reduction function
inline void R(uint sha1[5], char str[8], int i) {
    getstr((sha1[0] + sha1[1] * i) % 2176782336, str);
}

bool equalSHA1(uint a[5], uint b[5]) {
    for (int i = 0; i < 5; i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

bool equalCharStr(char* chars, string str) {
    for (int i = 0; i < 8; i++) {
        if (chars[i] != str[i]) return false;
    }
    return true;
}

bool checkChain(char* head, uint target_sha1[5]) {
    char buf[8];
    uint sha1_tmp[5];
    memcpy(buf, head, sizeof(buf));
    uint buf_size = sizeof(buf);
    // 100 times SHA1 + R makes a chain
    for (int i = 0; i < 100; i++) {
        for (int j = 1; j <= 100; j++) {
            UnitSHA1(buf, buf_size, sha1_tmp);
            if (equalSHA1(sha1_tmp, target_sha1)) {
                printf("%8s\n", buf);
                return true;
            }
            R(sha1_tmp, buf, j);
        }
    }
    return false;
}

int main() {
#ifdef DEBUG
    freopen("./sample/1.in", "r", stdin);
    auto start = high_resolution_clock::now();
#endif
    int m;
    scanf("%d", &m);
    vector<char[8]> head(m), tail(m);
    for (int i = 0; i < m; i++) {
        scanf("%8s %8s", head[i], tail[i]);
        string tail_str = tail[i];
        hmap[tail_str] = head[i];
    }
    uint sha1_value[5];
    scanf("%8x%8x%8x%8x%8x", &sha1_value[0], &sha1_value[1], &sha1_value[2],
          &sha1_value[3], &sha1_value[4]);
    uint sha1_tmp[5];
    char buf[8];
    char* found_head;
    // i is the first R function
    for (int i = 1; i <= 100; i++) {
        memcpy(sha1_tmp, sha1_value, sizeof(sha1_tmp));
        for (int j = 0; j < 10000; j++) {
            int r = (j + i) % 100 == 0 ? 100 : (j + i) % 100;
            R(sha1_tmp, buf, r);
            string buf_str = buf;
            if (r == 100) {
                if (hmap.find(buf_str) != hmap.end()) {
                    // the chain is not useless
                    found_head = hmap[buf_str];
                    // found_flag = true;
                    bool real_found = checkChain(found_head, sha1_value);
                    if (real_found) {
#ifdef DEBUG
                        auto end = high_resolution_clock::now();
                        auto duration =
                            duration_cast<milliseconds>(end - start);
                        printf("All running time: %lu ms\n", duration.count());
#endif
                        return 0;
                    }
                }
            }
            UnitSHA1(buf, sizeof(buf), sha1_tmp);
        }
    }
    printf("None\n");
#ifdef DEBUG
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    printf("All running time: %lu ms\n", duration.count());
#endif
    return 0;
}