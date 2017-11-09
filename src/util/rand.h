#pragma once

#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>

// #define UINT64_C(val) (val##ULL)

// fastest PRNG
static uint64_t x; /* The state must be seeded with a nonzero value. */

static uint64_t inline xorshift64star(void) {
  x ^= x >> 12; // a
  x ^= x << 25; // b
  x ^= x >> 27; // c
  return x * UINT64_C(2685821657736338717);
}


// PRNG
/* The state must be seeded so that it is not everywhere zero. If you have
   a 64-bit seed,  we suggest to seed a xorshift64* generator and use its
   output to fill s. */

static uint64_t s[16];
static int p;

/* or use system cryptograhpic source */

static inline size_t system_seed(uint64_t* b, size_t n) {
  /* init b with n uint_64_t from system entropy pool */
  int f = open("/dev/random", O_RDONLY);
  if (f < 0) return f;
  size_t required = n * sizeof(uint64_t);
  size_t r = read(f, b, required);
  close(f);
  if (r != required) return -1;
  else return r;
}


static inline size_t init_prng(void) {
  p = 0;
  return (system_seed(s, 16));
}

// PRNG
/* high quality with large period 2^1024 - 1 */

static inline uint64_t xorshift1024star(void) {
  uint64_t s0 = s[p];
  uint64_t s1 = s[p = (p+1) & 15];
  s1 ^= s1 << 31; // a
  s1 ^= s1 >> 11; // b
  s0 ^= s0 >> 30; // c
  return ( s[p] = s0 ^ s1 ) * UINT64_C(1181783497276652981);
}


/* random numbers */
static inline uint64_t sdmrand(void) {
  return xorshift1024star();
}

/* todo define a size_t [0, n-1] random number */
inline size_t irand(const size_t ub) {
  // modulus truncations will hurt our distribution so so something
  // a bit more rigourous...
  uint64_t r = sdmrand();
  /*
  while (r > ub)
    r = sdmrand();
  */
  return (size_t) (r % ub);
}


/* define a Knuth shuffler */
#define shuffle(type)                                   \
void shuffle_##type(type *list, size_t len) {		\
  int j;						\
  type tmp;                                             \
  while(len) {                                          \
    j = irand(len);                                     \
    if (j != len - 1) {                                 \
      tmp = list[j];                                    \
      list[j] = list[len - 1];                          \
      list[len - 1] = tmp;                              \
    }                                                   \
    len--;                                              \
  }                                                     \
}		


