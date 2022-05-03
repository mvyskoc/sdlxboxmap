#ifndef __BITEXT_H
#define __BITEXT_H


#ifdef __BITEXT_CPP
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN bool is_little_endian();
EXTERN int to_int16le(int value);

#endif