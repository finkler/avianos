#include <u.h>
#include <hash.h>

#define DBL_INT_ADD(a,b,c) if(a > 0xffffffff-(c)) b++; a += c;
#define LOR32(x,shift) (((x)<<(shift))|((x)>>(32-(shift))))
#define ROR32(x,shift) (((x)>>(shift))|((x)<<(32-(shift))))

#define CH(x,y,z) (((x)&(y))^(~(x)&(z)))
#define MAJ(x,y,z) (((x)&(y))^((x)&(z))^((y)&(z)))
#define EP0(x) (ROR32(x,2)^ROR32(x,13)^ROR32(x,22))
#define EP1(x) (ROR32(x,6)^ROR32(x,11)^ROR32(x,25))
#define SIG0(x) (ROR32(x,7)^ROR32(x,18)^((x)>>3))
#define SIG1(x) (ROR32(x,17)^ROR32(x,19)^((x)>>10))

static void pad(SHA256Digest *);
static void process(SHA256Digest *);
static void update(uchar *, int, SHA256Digest *);

uint k[64] = {
  0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b,
  0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01,
  0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7,
  0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
  0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152,
  0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
  0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc,
  0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
  0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819,
  0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08,
  0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f,
  0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
  0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

void
pad(SHA256Digest *state) {
  uint i;

  i = state->i;
  if (state->i < 56) {
    state->msg[i++] = 0x80;
    while(i < 56)
      state->msg[i++] = 0x00;
  } else {
    state->msg[i++] = 0x80;
    while(i < 64)
      state->msg[i++] = 0x00;
    process(state);
    memset(state->msg, 0, 56);
  }
  DBL_INT_ADD(state->low, state->high, state->i*8);
  state->msg[63] = state->low;
  state->msg[62] = state->low >> 8;
  state->msg[61] = state->low >> 16;
  state->msg[60] = state->low >> 24;
  state->msg[59] = state->high;
  state->msg[58] = state->high >> 8;
  state->msg[57] = state->high >> 16;
  state->msg[56] = state->high >> 24;
  process(state);
}

void
process(SHA256Digest *state) {
  uint a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

  for(i = 0, j = 0; i < 16; i++, j += 4)
    m[i] = (state->msg[j]<<24)|(state->msg[j+1]<<16)|
        (state->msg[j+2]<<8)|(state->msg[j+3]);
  for(; i < 64; ++i)
    m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];
  a = state->h[0];
  b = state->h[1];
  c = state->h[2];
  d = state->h[3];
  e = state->h[4];
  f = state->h[5];
  g = state->h[6];
  h = state->h[7];
  for(i = 0; i < 64; i++) {
     t1 = h + EP1(e) + CH(e,f,g) + k[i] + m[i];
     t2 = EP0(a) + MAJ(a,b,c);
     h = g;
     g = f;
     f = e;
     e = d + t1;
     d = c;
     c = b;
     b = a;
     a = t1 + t2;
  }
  state->h[0] += a;
  state->h[1] += b;
  state->h[2] += c;
  state->h[3] += d;
  state->h[4] += e;
  state->h[5] += f;
  state->h[6] += g;
  state->h[7] += h;
}

SHA256Digest *
sha256(uchar *data, int len, SHA256Digest *state) {
  if(state == nil) {
    state = malloc(sizeof(SHA256Digest));
    if(state == nil)
      return nil;
    memset(state, 0, sizeof(SHA256Digest));
    state->h[0] = 0x6a09e667;
    state->h[1] = 0xbb67ae85;
    state->h[2] = 0x3c6ef372;
    state->h[3] = 0xa54ff53a;
    state->h[4] = 0x510e527f;
    state->h[5] = 0x9b05688c;
    state->h[6] = 0x1f83d9ab;
    state->h[7] = 0x5be0cd19;
  }
  update(data, len, state);
  return state;
}

char *
sha256pickle(SHA256Digest *) {
  uchar hash[64];
  uint i;

  for(i = 0; i < 4; i++) {
    hash[i] = (state->h[0]>>(24-i*8))&0x000000ff;
    hash[i+4] = (state->h[1]>>(24-i*8))&0x000000ff;
    hash[i+8] = (state->h[2]>>(24-i*8))&0x000000ff;
    hash[i+12] = (state->h[3]>>(24-i*8))&0x000000ff;
    hash[i+16] = (state->h[4]>>(24-i*8))&0x000000ff;
    hash[i+20] = (state->h[5]>>(24-i*8))&0x000000ff;
    hash[i+24] = (state->h[6]>>(24-i*8))&0x000000ff;
    hash[i+28] = (state->h[7]>>(24-i*8))&0x000000ff;
  }
  return nil;
}

void
update(uchar *data, uint len, SHA256Digest *state) {
  uint i;

  for(i = 0; i < len; i++) {
    state->msg[state->i] = data[i];
    state->i++;
    if(state->i == 64) {
      process(state);
      DBL_INT_ADD(state->low, state->high, 512);
      state->i = 0;
    }
  }
}
