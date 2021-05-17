//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

#ifndef _MURMURHASH3_H_
#define _MURMURHASH3_H_


#ifdef __cplusplus
# include <cstdint>
#else
# include <stdint.h>
#endif



#ifdef __cplusplus
extern "C" {
#endif



uint32_t murmurhash3_x86_32  (const void * key, int len, uint32_t seed);


#ifdef __cplusplus
}           /* closing brace for extern "C" */
#endif


#endif // _MURMURHASH3_H_
