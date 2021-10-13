// ref: glibc-2.34

#include "string.h"

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

/* _wordcopy_fwd_aligned -- Copy block beginning at SRCP to
   block beginning at DSTP with LEN `op_t' words (not LEN bytes!).
   Both SRCP and DSTP should be aligned for memory operations on `op_t's.  */

#ifndef WORDCOPY_FWD_ALIGNED
#define WORDCOPY_FWD_ALIGNED _wordcopy_fwd_aligned
#endif

void WORDCOPY_FWD_ALIGNED(long int dstp, long int srcp, size_t n) {
  op_t a0, a1;

  switch (n % 8) {
  case 2:
    a0 = ((op_t *)srcp)[0];
    srcp -= 6 * OPSIZ;
    dstp -= 7 * OPSIZ;
    n += 6;
    goto do1;
  case 3:
    a1 = ((op_t *)srcp)[0];
    srcp -= 5 * OPSIZ;
    dstp -= 6 * OPSIZ;
    n += 5;
    goto do2;
  case 4:
    a0 = ((op_t *)srcp)[0];
    srcp -= 4 * OPSIZ;
    dstp -= 5 * OPSIZ;
    n += 4;
    goto do3;
  case 5:
    a1 = ((op_t *)srcp)[0];
    srcp -= 3 * OPSIZ;
    dstp -= 4 * OPSIZ;
    n += 3;
    goto do4;
  case 6:
    a0 = ((op_t *)srcp)[0];
    srcp -= 2 * OPSIZ;
    dstp -= 3 * OPSIZ;
    n += 2;
    goto do5;
  case 7:
    a1 = ((op_t *)srcp)[0];
    srcp -= 1 * OPSIZ;
    dstp -= 2 * OPSIZ;
    n += 1;
    goto do6;

  case 0:
    if (OP_T_THRES <= 3 * OPSIZ && n == 0)
      return;
    a0 = ((op_t *)srcp)[0];
    srcp -= 0 * OPSIZ;
    dstp -= 1 * OPSIZ;
    goto do7;
  case 1:
    a1 = ((op_t *)srcp)[0];
    srcp -= -1 * OPSIZ;
    dstp -= 0 * OPSIZ;
    n -= 1;
    if (OP_T_THRES <= 3 * OPSIZ && n == 0)
      goto do0;
    goto do8; /* No-op.  */
  }

  do {
  do8:
    a0 = ((op_t *)srcp)[0];
    ((op_t *)dstp)[0] = a1;
  do7:
    a1 = ((op_t *)srcp)[1];
    ((op_t *)dstp)[1] = a0;
  do6:
    a0 = ((op_t *)srcp)[2];
    ((op_t *)dstp)[2] = a1;
  do5:
    a1 = ((op_t *)srcp)[3];
    ((op_t *)dstp)[3] = a0;
  do4:
    a0 = ((op_t *)srcp)[4];
    ((op_t *)dstp)[4] = a1;
  do3:
    a1 = ((op_t *)srcp)[5];
    ((op_t *)dstp)[5] = a0;
  do2:
    a0 = ((op_t *)srcp)[6];
    ((op_t *)dstp)[6] = a1;
  do1:
    a1 = ((op_t *)srcp)[7];
    ((op_t *)dstp)[7] = a0;

    srcp += 8 * OPSIZ;
    dstp += 8 * OPSIZ;
    n -= 8;
  } while (n != 0);

  /* This is the right position for do0.  Please don't move
     it into the loop.  */
do0:
  ((op_t *)dstp)[0] = a1;
}

/* _wordcopy_fwd_dest_aligned -- Copy block beginning at SRCP to
   block beginning at DSTP with LEN `op_t' words (not LEN bytes!).
   DSTP should be aligned for memory operations on `op_t's, but SRCP must
   *not* be aligned.  */

#ifndef WORDCOPY_FWD_DEST_ALIGNED
#define WORDCOPY_FWD_DEST_ALIGNED _wordcopy_fwd_dest_aligned
#endif

void WORDCOPY_FWD_DEST_ALIGNED(long int dstp, long int srcp, size_t n) {
  op_t a0, a1, a2, a3;
  int sh_1, sh_2;

  /* Calculate how to shift a word read at the memory operation
     aligned srcp to make it aligned for copy.  */

  sh_1 = 8 * (srcp % OPSIZ);
  sh_2 = 8 * OPSIZ - sh_1;

  /* Make SRCP aligned by rounding it down to the beginning of the `op_t'
     it points in the middle of.  */
  srcp &= -OPSIZ;

  switch (n % 4) {
  case 2:
    a1 = ((op_t *)srcp)[0];
    a2 = ((op_t *)srcp)[1];
    srcp -= 1 * OPSIZ;
    dstp -= 3 * OPSIZ;
    n += 2;
    goto do1;
  case 3:
    a0 = ((op_t *)srcp)[0];
    a1 = ((op_t *)srcp)[1];
    srcp -= 0 * OPSIZ;
    dstp -= 2 * OPSIZ;
    n += 1;
    goto do2;
  case 0:
    if (OP_T_THRES <= 3 * OPSIZ && n == 0)
      return;
    a3 = ((op_t *)srcp)[0];
    a0 = ((op_t *)srcp)[1];
    srcp -= -1 * OPSIZ;
    dstp -= 1 * OPSIZ;
    n += 0;
    goto do3;
  case 1:
    a2 = ((op_t *)srcp)[0];
    a3 = ((op_t *)srcp)[1];
    srcp -= -2 * OPSIZ;
    dstp -= 0 * OPSIZ;
    n -= 1;
    if (OP_T_THRES <= 3 * OPSIZ && n == 0)
      goto do0;
    goto do4; /* No-op.  */
  }

  do {
  do4:
    a0 = ((op_t *)srcp)[0];
    ((op_t *)dstp)[0] = MERGE(a2, sh_1, a3, sh_2);
  do3:
    a1 = ((op_t *)srcp)[1];
    ((op_t *)dstp)[1] = MERGE(a3, sh_1, a0, sh_2);
  do2:
    a2 = ((op_t *)srcp)[2];
    ((op_t *)dstp)[2] = MERGE(a0, sh_1, a1, sh_2);
  do1:
    a3 = ((op_t *)srcp)[3];
    ((op_t *)dstp)[3] = MERGE(a1, sh_1, a2, sh_2);

    srcp += 4 * OPSIZ;
    dstp += 4 * OPSIZ;
    n -= 4;
  } while (n != 0);

  /* This is the right position for do0.  Please don't move
     it into the loop.  */
do0:
  ((op_t *)dstp)[0] = MERGE(a2, sh_1, a3, sh_2);
}

/* _wordcopy_bwd_aligned -- Copy block finishing right before
   SRCP to block finishing right before DSTP with LEN `op_t' words
   (not LEN bytes!).  Both SRCP and DSTP should be aligned for memory
   operations on `op_t's.  */

#ifndef WORDCOPY_BWD_ALIGNED
#define WORDCOPY_BWD_ALIGNED _wordcopy_bwd_aligned
#endif

void WORDCOPY_BWD_ALIGNED(long int dstp, long int srcp, size_t n) {
  op_t a0, a1;

  switch (n % 8) {
  case 2:
    srcp -= 2 * OPSIZ;
    dstp -= 1 * OPSIZ;
    a0 = ((op_t *)srcp)[1];
    n += 6;
    goto do1;
  case 3:
    srcp -= 3 * OPSIZ;
    dstp -= 2 * OPSIZ;
    a1 = ((op_t *)srcp)[2];
    n += 5;
    goto do2;
  case 4:
    srcp -= 4 * OPSIZ;
    dstp -= 3 * OPSIZ;
    a0 = ((op_t *)srcp)[3];
    n += 4;
    goto do3;
  case 5:
    srcp -= 5 * OPSIZ;
    dstp -= 4 * OPSIZ;
    a1 = ((op_t *)srcp)[4];
    n += 3;
    goto do4;
  case 6:
    srcp -= 6 * OPSIZ;
    dstp -= 5 * OPSIZ;
    a0 = ((op_t *)srcp)[5];
    n += 2;
    goto do5;
  case 7:
    srcp -= 7 * OPSIZ;
    dstp -= 6 * OPSIZ;
    a1 = ((op_t *)srcp)[6];
    n += 1;
    goto do6;

  case 0:
    if (OP_T_THRES <= 3 * OPSIZ && n == 0)
      return;
    srcp -= 8 * OPSIZ;
    dstp -= 7 * OPSIZ;
    a0 = ((op_t *)srcp)[7];
    goto do7;
  case 1:
    srcp -= 9 * OPSIZ;
    dstp -= 8 * OPSIZ;
    a1 = ((op_t *)srcp)[8];
    n -= 1;
    if (OP_T_THRES <= 3 * OPSIZ && n == 0)
      goto do0;
    goto do8; /* No-op.  */
  }

  do {
  do8:
    a0 = ((op_t *)srcp)[7];
    ((op_t *)dstp)[7] = a1;
  do7:
    a1 = ((op_t *)srcp)[6];
    ((op_t *)dstp)[6] = a0;
  do6:
    a0 = ((op_t *)srcp)[5];
    ((op_t *)dstp)[5] = a1;
  do5:
    a1 = ((op_t *)srcp)[4];
    ((op_t *)dstp)[4] = a0;
  do4:
    a0 = ((op_t *)srcp)[3];
    ((op_t *)dstp)[3] = a1;
  do3:
    a1 = ((op_t *)srcp)[2];
    ((op_t *)dstp)[2] = a0;
  do2:
    a0 = ((op_t *)srcp)[1];
    ((op_t *)dstp)[1] = a1;
  do1:
    a1 = ((op_t *)srcp)[0];
    ((op_t *)dstp)[0] = a0;

    srcp -= 8 * OPSIZ;
    dstp -= 8 * OPSIZ;
    n -= 8;
  } while (n != 0);

  /* This is the right position for do0.  Please don't move
     it into the loop.  */
do0:
  ((op_t *)dstp)[7] = a1;
}

/* _wordcopy_bwd_dest_aligned -- Copy block finishing right
   before SRCP to block finishing right before DSTP with LEN `op_t'
   words (not LEN bytes!).  DSTP should be aligned for memory
   operations on `op_t', but SRCP must *not* be aligned.  */

#ifndef WORDCOPY_BWD_DEST_ALIGNED
#define WORDCOPY_BWD_DEST_ALIGNED _wordcopy_bwd_dest_aligned
#endif

void WORDCOPY_BWD_DEST_ALIGNED(long int dstp, long int srcp, size_t n) {
  op_t a0, a1, a2, a3;
  int sh_1, sh_2;

  /* Calculate how to shift a word read at the memory operation
     aligned srcp to make it aligned for copy.  */

  sh_1 = 8 * (srcp % OPSIZ);
  sh_2 = 8 * OPSIZ - sh_1;

  /* Make srcp aligned by rounding it down to the beginning of the op_t
     it points in the middle of.  */
  srcp &= -OPSIZ;
  srcp += OPSIZ;

  switch (n % 4) {
  case 2:
    srcp -= 3 * OPSIZ;
    dstp -= 1 * OPSIZ;
    a2 = ((op_t *)srcp)[2];
    a1 = ((op_t *)srcp)[1];
    n += 2;
    goto do1;
  case 3:
    srcp -= 4 * OPSIZ;
    dstp -= 2 * OPSIZ;
    a3 = ((op_t *)srcp)[3];
    a2 = ((op_t *)srcp)[2];
    n += 1;
    goto do2;
  case 0:
    if (OP_T_THRES <= 3 * OPSIZ && n == 0)
      return;
    srcp -= 5 * OPSIZ;
    dstp -= 3 * OPSIZ;
    a0 = ((op_t *)srcp)[4];
    a3 = ((op_t *)srcp)[3];
    goto do3;
  case 1:
    srcp -= 6 * OPSIZ;
    dstp -= 4 * OPSIZ;
    a1 = ((op_t *)srcp)[5];
    a0 = ((op_t *)srcp)[4];
    n -= 1;
    if (OP_T_THRES <= 3 * OPSIZ && n == 0)
      goto do0;
    goto do4; /* No-op.  */
  }

  do {
  do4:
    a3 = ((op_t *)srcp)[3];
    ((op_t *)dstp)[3] = MERGE(a0, sh_1, a1, sh_2);
  do3:
    a2 = ((op_t *)srcp)[2];
    ((op_t *)dstp)[2] = MERGE(a3, sh_1, a0, sh_2);
  do2:
    a1 = ((op_t *)srcp)[1];
    ((op_t *)dstp)[1] = MERGE(a2, sh_1, a3, sh_2);
  do1:
    a0 = ((op_t *)srcp)[0];
    ((op_t *)dstp)[0] = MERGE(a1, sh_1, a2, sh_2);

    srcp -= 4 * OPSIZ;
    dstp -= 4 * OPSIZ;
    n -= 4;
  } while (n != 0);

  /* This is the right position for do0.  Please don't move
     it into the loop.  */
do0:
  ((op_t *)dstp)[3] = MERGE(a0, sh_1, a1, sh_2);
}

#define CMP_LT_OR_GT(a, b) memcmp_bytes((a), (b))

/* The strategy of this memcmp is:

   1. Compare bytes until one of the block pointers is aligned.

   2. Compare using memcmp_common_alignment or
      memcmp_not_common_alignment, regarding the alignment of the other
      block after the initial byte operations.  The maximum number of
      full words (of type op_t) are compared in this way.

   3. Compare the few remaining bytes.  */

/* memcmp_bytes -- Compare A and B bytewise in the byte order of the machine.
   A and B are known to be different.
   This is needed only on little-endian machines.  */

static int memcmp_bytes(op_t a, op_t b) {
  long int srcp1 = (long int)&a;
  long int srcp2 = (long int)&b;
  op_t a0, b0;

  do {
    a0 = ((byte *)srcp1)[0];
    b0 = ((byte *)srcp2)[0];
    srcp1 += 1;
    srcp2 += 1;
  } while (a0 == b0);
  return a0 - b0;
}

/* memcmp_common_alignment -- Compare blocks at SRCP1 and SRCP2 with LEN `op_t'
   objects (not LEN bytes!).  Both SRCP1 and SRCP2 should be aligned for
   memory operations on `op_t's.  */
static int memcmp_common_alignment(long int srcp1, long int srcp2, size_t len) {
  op_t a0, a1;
  op_t b0, b1;

  switch (len % 4) {
  default: /* Avoid warning about uninitialized local variables.  */
  case 2:
    a0 = ((op_t *)srcp1)[0];
    b0 = ((op_t *)srcp2)[0];
    srcp1 -= 2 * OPSIZ;
    srcp2 -= 2 * OPSIZ;
    len += 2;
    goto do1;
  case 3:
    a1 = ((op_t *)srcp1)[0];
    b1 = ((op_t *)srcp2)[0];
    srcp1 -= OPSIZ;
    srcp2 -= OPSIZ;
    len += 1;
    goto do2;
  case 0:
    if (OP_T_THRES <= 3 * OPSIZ && len == 0)
      return 0;
    a0 = ((op_t *)srcp1)[0];
    b0 = ((op_t *)srcp2)[0];
    goto do3;
  case 1:
    a1 = ((op_t *)srcp1)[0];
    b1 = ((op_t *)srcp2)[0];
    srcp1 += OPSIZ;
    srcp2 += OPSIZ;
    len -= 1;
    if (OP_T_THRES <= 3 * OPSIZ && len == 0)
      goto do0;
    /* Fall through.  */
  }

  do {
    a0 = ((op_t *)srcp1)[0];
    b0 = ((op_t *)srcp2)[0];
    if (a1 != b1)
      return CMP_LT_OR_GT(a1, b1);

  do3:
    a1 = ((op_t *)srcp1)[1];
    b1 = ((op_t *)srcp2)[1];
    if (a0 != b0)
      return CMP_LT_OR_GT(a0, b0);

  do2:
    a0 = ((op_t *)srcp1)[2];
    b0 = ((op_t *)srcp2)[2];
    if (a1 != b1)
      return CMP_LT_OR_GT(a1, b1);

  do1:
    a1 = ((op_t *)srcp1)[3];
    b1 = ((op_t *)srcp2)[3];
    if (a0 != b0)
      return CMP_LT_OR_GT(a0, b0);

    srcp1 += 4 * OPSIZ;
    srcp2 += 4 * OPSIZ;
    len -= 4;
  } while (len != 0);

  /* This is the right position for do0.  Please don't move
     it into the loop.  */
do0:
  if (a1 != b1)
    return CMP_LT_OR_GT(a1, b1);
  return 0;
}

/* memcmp_not_common_alignment -- Compare blocks at SRCP1 and SRCP2 with LEN
   `op_t' objects (not LEN bytes!).  SRCP2 should be aligned for memory
   operations on `op_t', but SRCP1 *should be unaligned*.  */
static int memcmp_not_common_alignment(long int srcp1, long int srcp2,
                                       size_t len) {
  op_t a0, a1, a2, a3;
  op_t b0, b1, b2, b3;
  op_t x;
  int shl, shr;

  /* Calculate how to shift a word read at the memory operation
     aligned srcp1 to make it aligned for comparison.  */

  shl = 8 * (srcp1 % OPSIZ);
  shr = 8 * OPSIZ - shl;

  /* Make SRCP1 aligned by rounding it down to the beginning of the `op_t'
     it points in the middle of.  */
  srcp1 &= -OPSIZ;

  switch (len % 4) {
  default: /* Avoid warning about uninitialized local variables.  */
  case 2:
    a1 = ((op_t *)srcp1)[0];
    a2 = ((op_t *)srcp1)[1];
    b2 = ((op_t *)srcp2)[0];
    srcp1 -= 1 * OPSIZ;
    srcp2 -= 2 * OPSIZ;
    len += 2;
    goto do1;
  case 3:
    a0 = ((op_t *)srcp1)[0];
    a1 = ((op_t *)srcp1)[1];
    b1 = ((op_t *)srcp2)[0];
    srcp2 -= 1 * OPSIZ;
    len += 1;
    goto do2;
  case 0:
    if (OP_T_THRES <= 3 * OPSIZ && len == 0)
      return 0;
    a3 = ((op_t *)srcp1)[0];
    a0 = ((op_t *)srcp1)[1];
    b0 = ((op_t *)srcp2)[0];
    srcp1 += 1 * OPSIZ;
    goto do3;
  case 1:
    a2 = ((op_t *)srcp1)[0];
    a3 = ((op_t *)srcp1)[1];
    b3 = ((op_t *)srcp2)[0];
    srcp1 += 2 * OPSIZ;
    srcp2 += 1 * OPSIZ;
    len -= 1;
    if (OP_T_THRES <= 3 * OPSIZ && len == 0)
      goto do0;
    /* Fall through.  */
  }

  do {
    a0 = ((op_t *)srcp1)[0];
    b0 = ((op_t *)srcp2)[0];
    x = MERGE(a2, shl, a3, shr);
    if (x != b3)
      return CMP_LT_OR_GT(x, b3);

  do3:
    a1 = ((op_t *)srcp1)[1];
    b1 = ((op_t *)srcp2)[1];
    x = MERGE(a3, shl, a0, shr);
    if (x != b0)
      return CMP_LT_OR_GT(x, b0);

  do2:
    a2 = ((op_t *)srcp1)[2];
    b2 = ((op_t *)srcp2)[2];
    x = MERGE(a0, shl, a1, shr);
    if (x != b1)
      return CMP_LT_OR_GT(x, b1);

  do1:
    a3 = ((op_t *)srcp1)[3];
    b3 = ((op_t *)srcp2)[3];
    x = MERGE(a1, shl, a2, shr);
    if (x != b2)
      return CMP_LT_OR_GT(x, b2);

    srcp1 += 4 * OPSIZ;
    srcp2 += 4 * OPSIZ;
    len -= 4;
  } while (len != 0);

  /* This is the right position for do0.  Please don't move
     it into the loop.  */
do0:
  x = MERGE(a2, shl, a3, shr);
  if (x != b3)
    return CMP_LT_OR_GT(x, b3);
  return 0;
}

/* Find the length of S, but scan at most MAXLEN characters.  If no
   '\0' terminator is found in that many characters, return MAXLEN.  */

size_t __strnlen(const char *str, size_t maxlen) {
  const char *char_ptr, *end_ptr = str + maxlen;
  const unsigned long int *longword_ptr;
  unsigned long int longword, himagic, lomagic;

  if (maxlen == 0)
    return 0;

  if (end_ptr < str)
    end_ptr = (const char *)~0UL;

  /* Handle the first few characters by reading one character at a time.
     Do this until CHAR_PTR is aligned on a longword boundary.  */
  for (char_ptr = str;
       ((unsigned long int)char_ptr & (sizeof(longword) - 1)) != 0; ++char_ptr)
    if (*char_ptr == '\0') {
      if (char_ptr > end_ptr)
        char_ptr = end_ptr;
      return char_ptr - str;
    }

  /* All these elucidatory comments refer to 4-byte longwords,
     but the theory applies equally well to 8-byte longwords.  */

  longword_ptr = (unsigned long int *)char_ptr;

  /* Bits 31, 24, 16, and 8 of this number are zero.  Call these bits
     the "holes."  Note that there is a hole just to the left of
     each byte, with an extra at the end:

     bits:  01111110 11111110 11111110 11111111
     bytes: AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD

     The 1-bits make sure that carries propagate to the next 0-bit.
     The 0-bits provide holes for carries to fall into.  */
  himagic = 0x80808080L;
  lomagic = 0x01010101L;
  if (sizeof(longword) > 4) {
    /* 64-bit version of the magic.  */
    /* Do the shift in two steps to avoid a warning if long has 32 bits.  */
    himagic = ((himagic << 16) << 16) | himagic;
    lomagic = ((lomagic << 16) << 16) | lomagic;
  }

  /* Instead of the traditional loop which tests each character,
     we will test a longword at a time.  The tricky part is testing
     if *any of the four* bytes in the longword in question are zero.  */
  while (longword_ptr < (unsigned long int *)end_ptr) {
    /* We tentatively exit the loop if adding MAGIC_BITS to
       LONGWORD fails to change any of the hole bits of LONGWORD.

       1) Is this safe?  Will it catch all the zero bytes?
       Suppose there is a byte with all zeros.  Any carry bits
       propagating from its left will fall into the hole at its
       least significant bit and stop.  Since there will be no
       carry from its most significant bit, the LSB of the
       byte to the left will be unchanged, and the zero will be
       detected.

       2) Is this worthwhile?  Will it ignore everything except
       zero bytes?  Suppose every byte of LONGWORD has a bit set
       somewhere.  There will be a carry into bit 8.  If bit 8
       is set, this will carry into bit 16.  If bit 8 is clear,
       one of bits 9-15 must be set, so there will be a carry
       into bit 16.  Similarly, there will be a carry into bit
       24.  If one of bits 24-30 is set, there will be a carry
       into bit 31, so all of the hole bits will be changed.

       The one misfire occurs when bits 24-30 are clear and bit
       31 is set; in this case, the hole at bit 31 is not
       changed.  If we had access to the processor carry flag,
       we could close this loophole by putting the fourth hole
       at bit 32!

       So it ignores everything except 128's, when they're aligned
       properly.  */

    longword = *longword_ptr++;

    if ((longword - lomagic) & himagic) {
      /* Which of the bytes was the zero?  If none of them were, it was
         a misfire; continue the search.  */

      const char *cp = (const char *)(longword_ptr - 1);

      char_ptr = cp;
      if (cp[0] == 0)
        break;
      char_ptr = cp + 1;
      if (cp[1] == 0)
        break;
      char_ptr = cp + 2;
      if (cp[2] == 0)
        break;
      char_ptr = cp + 3;
      if (cp[3] == 0)
        break;
      if (sizeof(longword) > 4) {
        char_ptr = cp + 4;
        if (cp[4] == 0)
          break;
        char_ptr = cp + 5;
        if (cp[5] == 0)
          break;
        char_ptr = cp + 6;
        if (cp[6] == 0)
          break;
        char_ptr = cp + 7;
        if (cp[7] == 0)
          break;
      }
    }
    char_ptr = end_ptr;
  }

  if (char_ptr > end_ptr)
    char_ptr = end_ptr;
  return char_ptr - str;
}

size_t strlen(const char *s) {
  const char *char_ptr;
  const unsigned long int *longword_ptr;
  unsigned long int longword, himagic, lomagic;

  /* Handle the first few characters by reading one character at a time.
     Do this until CHAR_PTR is aligned on a longword boundary.  */
  for (char_ptr = s;
       ((unsigned long int)char_ptr & (sizeof(longword) - 1)) != 0; ++char_ptr)
    if (*char_ptr == '\0')
      return char_ptr - s;

  /* All these elucidatory comments refer to 4-byte longwords,
     but the theory applies equally well to 8-byte longwords.  */

  longword_ptr = (unsigned long int *)char_ptr;

  /* Bits 31, 24, 16, and 8 of this number are zero.  Call these bits
     the "holes."  Note that there is a hole just to the left of
     each byte, with an extra at the end:

     bits:  01111110 11111110 11111110 11111111
     bytes: AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD

     The 1-bits make sure that carries propagate to the next 0-bit.
     The 0-bits provide holes for carries to fall into.  */
  himagic = 0x80808080L;
  lomagic = 0x01010101L;
  if (sizeof(longword) > 4) {
    /* 64-bit version of the magic.  */
    /* Do the shift in two steps to avoid a warning if long has 32 bits.  */
    himagic = ((himagic << 16) << 16) | himagic;
    lomagic = ((lomagic << 16) << 16) | lomagic;
  }

  /* Instead of the traditional loop which tests each character,
     we will test a longword at a time.  The tricky part is testing
     if *any of the four* bytes in the longword in question are zero.  */
  for (;;) {
    longword = *longword_ptr++;

    if (((longword - lomagic) & ~longword & himagic) != 0) {
      /* Which of the bytes was the zero?  If none of them were, it was
          a misfire; continue the search.  */

      const char *cp = (const char *)(longword_ptr - 1);

      if (cp[0] == 0)
        return cp - s;
      if (cp[1] == 0)
        return cp - s + 1;
      if (cp[2] == 0)
        return cp - s + 2;
      if (cp[3] == 0)
        return cp - s + 3;
      if (sizeof(longword) > 4) {
        if (cp[4] == 0)
          return cp - s + 4;
        if (cp[5] == 0)
          return cp - s + 5;
        if (cp[6] == 0)
          return cp - s + 6;
        if (cp[7] == 0)
          return cp - s + 7;
      }
    }
  }
}

char *strcpy(char *dst, const char *src) {
  return memcpy(dst, src, strlen(src) + 1);
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t size = __strnlen(src, n);
  if (size != n)
    memset(dst + size, '\0', n - size);
  return memcpy(dst, src, size);
}

char *strcat(char *dst, const char *src) {
  strcpy(dst + strlen(dst), src);
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  const unsigned char *p1 = (const unsigned char *)s1;
  const unsigned char *p2 = (const unsigned char *)s2;
  unsigned char c1, c2;

  do {
    c1 = (unsigned char)*p1++;
    c2 = (unsigned char)*p2++;
    if (c1 == '\0')
      return c1 - c2;
  } while (c1 == c2);

  return c1 - c2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  unsigned char c1 = '\0';
  unsigned char c2 = '\0';

  if (n >= 4) {
    size_t n4 = n >> 2;
    do {
      c1 = (unsigned char)*s1++;
      c2 = (unsigned char)*s2++;
      if (c1 == '\0' || c1 != c2)
        return c1 - c2;
      c1 = (unsigned char)*s1++;
      c2 = (unsigned char)*s2++;
      if (c1 == '\0' || c1 != c2)
        return c1 - c2;
      c1 = (unsigned char)*s1++;
      c2 = (unsigned char)*s2++;
      if (c1 == '\0' || c1 != c2)
        return c1 - c2;
      c1 = (unsigned char)*s1++;
      c2 = (unsigned char)*s2++;
      if (c1 == '\0' || c1 != c2)
        return c1 - c2;
    } while (--n4 > 0);
    n &= 3;
  }

  while (n > 0) {
    c1 = (unsigned char)*s1++;
    c2 = (unsigned char)*s2++;
    if (c1 == '\0' || c1 != c2)
      return c1 - c2;
    n--;
  }

  return c1 - c2;
}

void *memset(void *s, int c, size_t n) {
  long int dstp = (long int)s;

  if (n >= 8) {
    size_t xlen;
    op_t cccc;

    cccc = (unsigned char)c;
    cccc |= cccc << 8;
    cccc |= cccc << 16;
    if (OPSIZ > 4)
      /* Do the shift in two steps to avoid warning if long has 32 bits.  */
      cccc |= (cccc << 16) << 16;

    /* There are at least some bytes to set.
       No need to test for LEN == 0 in this alignment loop.  */
    while (dstp % OPSIZ != 0) {
      ((byte *)dstp)[0] = c;
      dstp += 1;
      n -= 1;
    }

    /* Write 8 `op_t' per iteration until less than 8 `op_t' remain.  */
    xlen = n / (OPSIZ * 8);
    while (xlen > 0) {
      ((op_t *)dstp)[0] = cccc;
      ((op_t *)dstp)[1] = cccc;
      ((op_t *)dstp)[2] = cccc;
      ((op_t *)dstp)[3] = cccc;
      ((op_t *)dstp)[4] = cccc;
      ((op_t *)dstp)[5] = cccc;
      ((op_t *)dstp)[6] = cccc;
      ((op_t *)dstp)[7] = cccc;
      dstp += 8 * OPSIZ;
      xlen -= 1;
    }
    n %= OPSIZ * 8;

    /* Write 1 `op_t' per iteration until less than OPSIZ bytes remain.  */
    xlen = n / OPSIZ;
    while (xlen > 0) {
      ((op_t *)dstp)[0] = cccc;
      dstp += OPSIZ;
      xlen -= 1;
    }
    n %= OPSIZ;
  }

  /* Write the last few bytes.  */
  while (n > 0) {
    ((byte *)dstp)[0] = c;
    dstp += 1;
    n -= 1;
  }

  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  unsigned long int dstp = (long int)dst;
  unsigned long int srcp = (long int)src;

  /* This test makes the forward copying code be used whenever possible.
     Reduces the working set.  */
  if (dstp - srcp >= n) {
    /* Copy from the beginning to the end.  */

    /* If there not too few bytes to copy, use word copy.  */
    if (n >= OP_T_THRES) {
      /* Copy just a few bytes to make DSTP aligned.  */
      n -= (-dstp) % OPSIZ;
      BYTE_COPY_FWD(dstp, srcp, (-dstp) % OPSIZ);

      /* Copy from SRCP to DSTP taking advantage of the known
          alignment of DSTP.  Number of bytes remaining is put
          in the third argument, i.e. in LEN.  This number may
          vary from machine to machine.  */

      WORD_COPY_FWD(dstp, srcp, n, n);

      /* Fall out and copy the tail.  */
    }

    /* There are just a few bytes to copy.  Use byte memory operations.  */
    BYTE_COPY_FWD(dstp, srcp, n);
  } else {
    /* Copy from the end to the beginning.  */
    srcp += n;
    dstp += n;

    /* If there not too few bytes to copy, use word copy.  */
    if (n >= OP_T_THRES) {
      /* Copy just a few bytes to make DSTP aligned.  */
      n -= dstp % OPSIZ;
      BYTE_COPY_BWD(dstp, srcp, dstp % OPSIZ);

      /* Copy from SRCP to DSTP taking advantage of the known
          alignment of DSTP.  Number of bytes remaining is put
          in the third argument, i.e. in LEN.  This number may
          vary from machine to machine.  */

      WORD_COPY_BWD(dstp, srcp, n, n);

      /* Fall out and copy the tail.  */
    }

    /* There are just a few bytes to copy.  Use byte memory operations.  */
    BYTE_COPY_BWD(dstp, srcp, n);
  }

  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  unsigned long int dstp = (long int)out;
  unsigned long int srcp = (long int)in;

  /* Copy from the beginning to the end.  */

  /* If there not too few bytes to copy, use word copy.  */
  if (n >= OP_T_THRES) {
    /* Copy just a few bytes to make DSTP aligned.  */
    n -= (-dstp) % OPSIZ;
    BYTE_COPY_FWD(dstp, srcp, (-dstp) % OPSIZ);

    /* Copy from SRCP to DSTP taking advantage of the known alignment of
       DSTP.  Number of bytes remaining is put in the third argument,
       i.e. in LEN.  This number may vary from machine to machine.  */

    WORD_COPY_FWD(dstp, srcp, n, n);

    /* Fall out and copy the tail.  */
  }

  /* There are just a few bytes to copy.  Use byte memory operations.  */
  BYTE_COPY_FWD(dstp, srcp, n);

  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  op_t a0;
  op_t b0;
  long int srcp1 = (long int)s1;
  long int srcp2 = (long int)s2;
  op_t res;

  if (n >= OP_T_THRES) {
    /* There are at least some bytes to compare.  No need to test
       for LEN == 0 in this alignment loop.  */
    while (srcp2 % OPSIZ != 0) {
      a0 = ((byte *)srcp1)[0];
      b0 = ((byte *)srcp2)[0];
      srcp1 += 1;
      srcp2 += 1;
      res = a0 - b0;
      if (res != 0)
        return res;
      n -= 1;
    }

    /* SRCP2 is now aligned for memory operations on `op_t'.
       SRCP1 alignment determines if we can do a simple,
       aligned compare or need to shuffle bits.  */

    if (srcp1 % OPSIZ == 0)
      res = memcmp_common_alignment(srcp1, srcp2, n / OPSIZ);
    else
      res = memcmp_not_common_alignment(srcp1, srcp2, n / OPSIZ);
    if (res != 0)
      return res;

    /* Number of bytes remaining in the interval [0..OPSIZ-1].  */
    srcp1 += n & -OPSIZ;
    srcp2 += n & -OPSIZ;
    n %= OPSIZ;
  }

  /* There are just a few bytes to compare.  Use byte memory operations.  */
  while (n != 0) {
    a0 = ((byte *)srcp1)[0];
    b0 = ((byte *)srcp2)[0];
    srcp1 += 1;
    srcp2 += 1;
    res = a0 - b0;
    if (res != 0)
      return res;
    n -= 1;
  }

  return 0;
}
