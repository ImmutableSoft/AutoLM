/***********************************************************************/
/*                                                                     */
/*   Module:  sha1.h                                                   */
/*   Version: 2007.0                                                   */
/*   Purpose: C/C++ SHA1 algorithm                                     */
/*                                                                     */
/*---------------------------------------------------------------------*/
/*                                                                     */
/*                 Copyright © 2020 ImmutableSoft Inc.                 */
/*                                                                     */
/* Permission is hereby granted, free of charge, to any person         */
/* obtaining a copy of this software and associated documentation      */
/* files (the “Software”), to deal in the Software without             */
/* restriction, including without limitation the rights to use, copy,  */
/* modify, merge, publish, distribute, sublicense, and/or sell copies  */
/* of the Software, and to permit persons to whom the Software is      */
/* furnished to do so, subject to the following conditions:            */
/*                                                                     */
/* The above copyright notice and this permission notice shall be      */
/* included in all copies or substantial portions of the Software.     */
/*                                                                     */
/* THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND,     */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF  */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND               */
/* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS */
/* BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN  */
/* ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN   */
/* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE    */
/* SOFTWARE.                                                           */
/*                                                                     */
/***********************************************************************/
#ifndef _SHA_H_
#define _SHA_H_

/***********************************************************************/
/* Configuration                                                       */
/***********************************************************************/

/***********************************************************************/
/* Define to 1 for FIPS 180.1 version (with extra rotate in            */
/* prescheduling), 0 for FIPS 180 version (with the mysterious         */
/* "weakness" that the NSA isn't talking about).                       */
/***********************************************************************/
#define SHAVERSION 1

#define SHABLOCKBYTES 64
#define SHA_BLOCKWORDS 16

#define SHAHASHBYTES 20
#define SHAHASHWORDS 5

/***********************************************************************/
/* Type Definitions                                                    */
/***********************************************************************/
typedef unsigned char            u_int8_t;
typedef short                     int16_t;
typedef unsigned short          u_int16_t;
typedef int                       int32_t;
typedef unsigned int            u_int32_t;

typedef long long                 i64_t;
typedef unsigned long long      u_i64_t;

/***********************************************************************/
/* SHA context                                                         */
/***********************************************************************/
typedef struct SHAContext {
   unsigned int key[SHA_BLOCKWORDS];
   u_int32_t iv[SHAHASHWORDS];
#ifdef __GNUC__
   u_i64_t bytes;
#else
  u_int32_t bytesHi, bytesLo;
#endif
} ShaCtx;

#if __cplusplus
class CSha {
public:
#endif
  void   ShaInit(ShaCtx *);
  void   ShaUpdate(ShaCtx *, const unsigned char *, unsigned int);
  void   ShaFinal(ShaCtx *, unsigned char [SHAHASHBYTES]);
#if __cplusplus
};
#endif

#endif /* _SHA_H_ */
