#include <u.h>
#include <hash.h>

#define LOR32(shift,x)\
  ((((x)<<(shift))&0xFFFFFFFF)|((x)>>(32-(shift))))

static void input(uchar *, int, SHA1Digest *);
static void pad(SHA1Digest *);
static void process(SHA1Digest *);

void
input(uchar *data, int len, SHA1Digest *state) {
  if(len == 0 || state->err)
    return;
  while(len-- && !state->err)   {
    state->msg[state->i++] = (*data&0xFF);
    state->low += 8;
    state->low &= 0xFFFFFFFF;
    if(state->low == 0) {
      state->high++;
      state->high &= 0xFFFFFFFF;
      if(state->high == 0)
        state->err = 1;
    }
    if(state->i == 64)
      process(state);
    data++;
  }
}

void
pad(SHA1Digest *state) {
  if(state->i > 55) {
    state->msg[state->i++] = 0x80;
    while(state->i < 64)
      state->msg[state->i++] = 0;
    process(state);
    while(state->i < 56)
      state->msg[state->i++] = 0;
  } else {
    state->msg[state->i++] = 0x80;
    while(state->i < 56)
      state->msg[state->i++] = 0;
  }
  state->msg[56] = (state->high>>24)&0xFF;
  state->msg[57] = (state->high>>16)&0xFF;
  state->msg[58] = (state->high>>8)&0xFF;
  state->msg[59] = (state->high)&0xFF;
  state->msg[60] = (state->low>>24)&0xFF;
  state->msg[61] = (state->low>>16)&0xFF;
  state->msg[62] = (state->low>>8)&0xFF;
  state->msg[63] = (state->low)&0xFF;
  process(state);
}

void
process(SHA1Digest *state) {
  uint32 A, B, C, D, E, K[4], W[80];
  uint32 temp;
  int i;

  for(i = 0; i < 16; i++) {
    W[i] = ((uint32)state->msg[i*4])<<24;
    W[i] |= ((uint32)state->msg[i*4+1])<<16;
    W[i] |= ((uint32)state->msg[i*4+2])<<8;
    W[i] |= ((uint32)state->msg[i*4+3]);
  }
  for(i = 16; i < 80; i++)
    W[i] = LOR32(1, W[i-3]^W[i-8]^W[i-14]^W[i-16]);
  A = state->h[0];
  B = state->h[1];
  C = state->h[2];
  D = state->h[3];
  E = state->h[4];
  K[0] = 0x5A827999;
  K[1] = 0x6ED9EBA1;
  K[2] = 0x8F1BBCDC;
  K[3] = 0xCA62C1D6;
  for(i = 0; i < 20; i++) {
    temp =  LOR32(5, A)+((B&C)|((~B)&D))+E+W[i]+K[0];
    temp &= 0xFFFFFFFF;
    E = D;
    D = C;
    C = LOR32(30, B);
    B = A;
    A = temp;
  }
  for(i = 20; i < 40; i++) {
    temp = LOR32(5, A)+(B^C^D)+E+W[i]+K[1];
    temp &= 0xFFFFFFFF;
    E = D;
    D = C;
    C = LOR32(30, B);
    B = A;
    A = temp;
  }
  for(i = 40; i < 60; i++) {
    temp = LOR32(5, A)+
      ((B&C)|(B&D)|(C&D))+E+W[i]+K[2];
    temp &= 0xFFFFFFFF;
    E = D;
    D = C;
    C = LOR32(30, B);
    B = A;
    A = temp;
  }
  for(i = 60; i < 80; i++) {
    temp = LOR32(5, A)+(B^C^D)+E+W[i]+K[3];
    temp &= 0xFFFFFFFF;
    E = D;
    D = C;
    C = LOR32(30, B);
    B = A;
    A = temp;
  }
  state->h[0] = (state->h[0]+A)&0xFFFFFFFF;
  state->h[1] = (state->h[1]+B)&0xFFFFFFFF;
  state->h[2] = (state->h[2]+C)&0xFFFFFFFF;
  state->h[3] = (state->h[3]+D)&0xFFFFFFFF;
  state->h[4] = (state->h[4]+E)&0xFFFFFFFF;
  state->i = 0;
}

SHA1Digest *
sha1(uchar *data, int len, SHA1Digest *state) {
  if(state == nil) {
    state = malloc(sizeof(SHA1Digest));
    if(state == nil)
      return nil;
    memset(state, 0, sizeof(SHA1Digest));
    state->h[0] = 0x67452301;
    state->h[1] = 0xEFCDAB89;
    state->h[2] = 0x98BADCFE;
    state->h[3] = 0x10325476;
    state->h[4] = 0xC3D2E1F0;
  }
  input(data, len, state);
  if(!state->err)
    return state;
  free(state);
  return nil;
}

char *
sha1pickle(SHA1Digest *state) {
  SHA1Digest d;
  char *p;

  memcpy(&d, state, sizeof(SHA1Digest));
  pad(&d);
  p = malloc(SHA1_LEN+1);
  if(p == nil)
    return nil;
  sprintf(p, "%08x%08x%08x%08x%08x", state->h[0], state->h[1],
    state->h[2], state->h[3], state->h[4]);
  return p;
}
