#ifndef _HASH_H_
#define _HASH_H_

#define CRC32(n) (~(n))
uint32 crc32(uchar *, int, uint32);

#define SHA1_LEN 40
typedef struct {
    uint32 h[5];
    uint32 low, high;
    uchar msg[64];
    int i;
    int err;
} SHA1Digest;

extern SHA1Digest *sha1(uchar *, int, SHA1Digest *);
extern char *sha1pickle(SHA1Digest *);

#endif /* _HASH_H_ */
