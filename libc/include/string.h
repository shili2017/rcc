#ifndef _STRING_H_
#define _STRING_H_

#include <stddef.h>

void *memset(void *s, int c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
void *memmove(void *dst, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
size_t strlen(const char *s);
char *strcat(char *dst, const char *src);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

// ref: glibc-2.34

/* Threshold value for when to enter the unrolled loops.  */
#define OP_T_THRES 16

/* Type to use for aligned memory operations.
   This should normally be the biggest type supported by a single load
   and store.  */
#define op_t unsigned long int
#define OPSIZ (sizeof(op_t))

/* Type to use for unaligned operations.  */
typedef unsigned char byte;

/* Little Endian */
#define MERGE(w0, sh_1, w1, sh_2) (((w0) >> (sh_1)) | ((w1) << (sh_2)))

/* Copy exactly NBYTES bytes from SRC_BP to DST_BP,
   without any assumptions about alignment of the pointers.  */
#define BYTE_COPY_FWD(dst_bp, src_bp, nbytes)                                  \
  do {                                                                         \
    size_t __nbytes = (nbytes);                                                \
    while (__nbytes > 0) {                                                     \
      byte __x = ((byte *)src_bp)[0];                                          \
      src_bp += 1;                                                             \
      __nbytes -= 1;                                                           \
      ((byte *)dst_bp)[0] = __x;                                               \
      dst_bp += 1;                                                             \
    }                                                                          \
  } while (0)

/* Copy exactly NBYTES_TO_COPY bytes from SRC_END_PTR to DST_END_PTR,
   beginning at the bytes right before the pointers and continuing towards
   smaller addresses.  Don't assume anything about alignment of the
   pointers.  */
#define BYTE_COPY_BWD(dst_ep, src_ep, nbytes)                                  \
  do {                                                                         \
    size_t __nbytes = (nbytes);                                                \
    while (__nbytes > 0) {                                                     \
      byte __x;                                                                \
      src_ep -= 1;                                                             \
      __x = ((byte *)src_ep)[0];                                               \
      dst_ep -= 1;                                                             \
      __nbytes -= 1;                                                           \
      ((byte *)dst_ep)[0] = __x;                                               \
    }                                                                          \
  } while (0)

/* Copy *up to* NBYTES bytes from SRC_BP to DST_BP, with
   the assumption that DST_BP is aligned on an OPSIZ multiple.  If
   not all bytes could be easily copied, store remaining number of bytes
   in NBYTES_LEFT, otherwise store 0.  */
extern void _wordcopy_fwd_aligned(long int, long int, size_t);
extern void _wordcopy_fwd_dest_aligned(long int, long int, size_t);
#define WORD_COPY_FWD(dst_bp, src_bp, nbytes_left, nbytes)                     \
  do {                                                                         \
    if (src_bp % OPSIZ == 0)                                                   \
      _wordcopy_fwd_aligned(dst_bp, src_bp, (nbytes) / OPSIZ);                 \
    else                                                                       \
      _wordcopy_fwd_dest_aligned(dst_bp, src_bp, (nbytes) / OPSIZ);            \
    src_bp += (nbytes) & -OPSIZ;                                               \
    dst_bp += (nbytes) & -OPSIZ;                                               \
    (nbytes_left) = (nbytes) % OPSIZ;                                          \
  } while (0)

/* Copy *up to* NBYTES_TO_COPY bytes from SRC_END_PTR to DST_END_PTR,
   beginning at the words (of type op_t) right before the pointers and
   continuing towards smaller addresses.  May take advantage of that
   DST_END_PTR is aligned on an OPSIZ multiple.  If not all bytes could be
   easily copied, store remaining number of bytes in NBYTES_REMAINING,
   otherwise store 0.  */
extern void _wordcopy_bwd_aligned(long int, long int, size_t);
extern void _wordcopy_bwd_dest_aligned(long int, long int, size_t);
#define WORD_COPY_BWD(dst_ep, src_ep, nbytes_left, nbytes)                     \
  do {                                                                         \
    if (src_ep % OPSIZ == 0)                                                   \
      _wordcopy_bwd_aligned(dst_ep, src_ep, (nbytes) / OPSIZ);                 \
    else                                                                       \
      _wordcopy_bwd_dest_aligned(dst_ep, src_ep, (nbytes) / OPSIZ);            \
    src_ep -= (nbytes) & -OPSIZ;                                               \
    dst_ep -= (nbytes) & -OPSIZ;                                               \
    (nbytes_left) = (nbytes) % OPSIZ;                                          \
  } while (0)

#endif // _STRING_H_
