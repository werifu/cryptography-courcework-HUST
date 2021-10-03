#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/pem.h>
#include <openssl/pkcs7.h>
#include <openssl/rand.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <stdio.h>

#include <cstring>
#include <iostream>
#include <string>

using namespace std;

const char CA_cert[] =
    "\
-----BEGIN CERTIFICATE----- \n\
MIIB/zCCAaagAwIBAgIJAKKa0PAt9M1FMAoGCCqBHM9VAYN1MFsxCzAJBgNVBAYT \n\
AkNOMQ4wDAYDVQQIDAVIdUJlaTEOMAwGA1UEBwwFV3VIYW4xDTALBgNVBAoMBEhV \n\
U1QxDDAKBgNVBAsMA0NTRTEPMA0GA1UEAwwGY2Fyb290MB4XDTIwMDkyMDIwNTkx \n\
OVoXDTMwMDkxODIwNTkxOVowWzELMAkGA1UEBhMCQ04xDjAMBgNVBAgMBUh1QmVp \n\
MQ4wDAYDVQQHDAVXdUhhbjENMAsGA1UECgwESFVTVDEMMAoGA1UECwwDQ1NFMQ8w \n\
DQYDVQQDDAZjYXJvb3QwWTATBgcqhkjOPQIBBggqgRzPVQGCLQNCAASJ8mm28JJR \n\
bZKLr6DCo1+KWimpKEsiTfZM19Zi5ao7Au6YLosyN71256MWmjwkwXxJeLa0lCfm \n\
kF/YWCX6qGQ0o1MwUTAdBgNVHQ4EFgQUAL5hW3RUzqvsiTzIc1gUHeK5uzQwHwYD \n\
VR0jBBgwFoAUAL5hW3RUzqvsiTzIc1gUHeK5uzQwDwYDVR0TAQH/BAUwAwEB/zAK \n\
BggqgRzPVQGDdQNHADBEAiAaZMmvE5zzXHx/TBgdUhjtpRH3Jpd6OZ+SOAfMtKxD \n\
LAIgdKq/v2Jkmn37Y9U8FHYDfFqk5I0qlQOAmuvbVUi3yvM= \n\
-----END CERTIFICATE----- \n\
";

const char private_key[] =
    "\
-----BEGIN EC PARAMETERS----- \n\
BggqgRzPVQGCLQ== \n\
-----END EC PARAMETERS----- \n\
-----BEGIN EC PRIVATE KEY----- \n\
MHcCAQEEINQhCKslrI3tKt6cK4Kxkor/LBvM8PSv699Xea7kTXTToAoGCCqBHM9V \n\
AYItoUQDQgAEH7rLLiFASe3SWSsGbxFUtfPY//pXqLvgM6ROyiYhLkPxEulwrTe8 \n\
kv5R8/NA7kSSvcsGIQ9EPWhr6HnCULpklw== \n\
-----END EC PRIVATE KEY----- \n\
";
void error() {
    printf("ERROR\n");
    return;
}
X509 *getX509(const char *cert) {
    BIO *bio;
    bio = BIO_new(BIO_s_mem());
    BIO_puts(bio, cert);
    return PEM_read_bio_X509(bio, NULL, NULL, NULL);
}
EVP_PKEY *getpkey(const char *private_key) {
    BIO *bio_pkey = BIO_new_mem_buf((char *)private_key, strlen(private_key));
    if (bio_pkey == NULL) return NULL;
    return PEM_read_bio_PrivateKey(bio_pkey, NULL, NULL, NULL);
}
PKCS7 *getPKCS7(const char *pkcs7) {
    BIO *p7Bio = BIO_new_mem_buf((char *)pkcs7, strlen(pkcs7));
    if (p7Bio == NULL) return NULL;
    return PEM_read_bio_PKCS7(p7Bio, NULL, NULL, NULL);
}

int verify_signature(PKCS7 *p7, BIO *p7Bio, X509 *tcacert) {
    STACK_OF(PKCS7_SIGNER_INFO) *sk = PKCS7_get_signer_info(p7);
    if (!sk) return false;
    X509_STORE *store = X509_STORE_new();
    X509_STORE_CTX *ct = X509_STORE_CTX_new();
    X509_STORE_add_cert(store, tcacert);
    int signNum = sk_PKCS7_SIGNER_INFO_num(sk);
    int flag = true;
    for (int i = 0; i < signNum; ++i) {
        PKCS7_SIGNER_INFO *signInfo = sk_PKCS7_SIGNER_INFO_value(sk, i);
        auto res = PKCS7_dataVerify(store, ct, p7Bio, p7, signInfo);
        PKCS7_SIGNER_INFO_free(signInfo);
        if (res <= 0) {
            flag = false;
            break;
        }
    }
    X509_STORE_free(store);
    X509_STORE_CTX_free(ct);
    sk_PKCS7_SIGNER_INFO_free(sk);
    return flag;
}

void decrypt() {
    char buffer[4096];
    char pkcs7_buf[4096];

    fread(pkcs7_buf, 4095, 1, stdin);
    // get enveloped data
    auto p7 = getPKCS7(pkcs7_buf);
    if (!p7) {
        return error();
    }

    // decrypt
    auto pkey = getpkey(private_key);
    auto p7Bio = PKCS7_dataDecode(p7, pkey, NULL, NULL);
    if (!p7Bio) {
        return error();
    }

    // get cert
    auto cert = getX509(CA_cert);
    if (!cert) {
        return error();
    }

    int len = BIO_read(p7Bio, buffer, sizeof(buffer));
    if (len <= 0) {
        return error();
    }

    // verify
    if (!verify_signature(p7, p7Bio, cert)) {
        return error();
    }

    // print the plaintext
    BIO *out = BIO_new_fd(fileno(stdout), BIO_NOCLOSE);
    BIO_write(out, buffer, len);
    X509_free(cert);

    return;
}

int main() {
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
    // the above two lines are necessary!
    decrypt();
    return 0;
}