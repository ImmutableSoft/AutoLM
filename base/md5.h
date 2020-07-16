/***********************************************************************/
/*                                                                     */
/*   Module:  md5.h                                                    */
/*   Version: 2007.0                                                   */
/*   Purpose: C/C++ MD5 algorithm                                      */
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

/***********************************************************************/
/* Portions below Copyright (C) 1991-2, RSA Data Security, Inc.        */
/* Created 1991. All rights reserved.                                  */
/*                                                                     */
/* License to copy and use this software is granted provided that it   */
/* is identified as the "RSA Data Security, Inc. MD5 Message-Digest    */
/* Algorithm" in all material mentioning or referencing this software  */
/* or this function.                                                   */
/*                                                                     */
/* License is also granted to make and use derivative works provided   */
/* that such works are identified as "derived from the RSA Data        */
/* Security, Inc. MD5 Message-Digest Algorithm" in all material        */
/* mentioning or referencing the derived work.                         */
/*                                                                     */
/* RSA Data Security, Inc. makes no representations concerning either  */
/* the merchantability of this software or the suitability of this     */
/* software for any particular purpose. It is provided "as is"         */
/* without express or implied warranty of any kind.                    */
/*                                                                     */
/* These notices must be retained in any copies of any part of this    */
/* documentation and/or software.                                      */
/*                                                                     */
/***********************************************************************/

#ifndef _MD5_H
#define _MD5_H

/***********************************************************************/
/* POINTER defines a generic pointer type                              */
/***********************************************************************/
typedef unsigned char *POINTER;

/***********************************************************************/
/* UINT2 defines a two byte word                                       */
/***********************************************************************/
typedef unsigned short int UINT2;

/***********************************************************************/
/* UINT4 defines a four byte word                                      */
/***********************************************************************/
typedef unsigned long int UINT4;

/***********************************************************************/
/* MD5 context.                                                        */
/***********************************************************************/
typedef struct {
  UINT4 state[4];                                   /* state (ABCD) */
  UINT4 count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} Md5Ctx;

#if __cplusplus
class md5 {
public:
#endif
  void Md5Init(Md5Ctx *context);
  void Md5Update(Md5Ctx *context, unsigned char *input,
    unsigned int inputLen);
  void Md5Final(Md5Ctx *context, unsigned char digest[16]);
#if __cplusplus
private:
#endif
  STATIC void Md5Transform(UINT4 state[4],
    unsigned char block[64]);
  STATIC void Md5Encode(unsigned char *output, UINT4 *input,
    unsigned int len);
  STATIC void Md5Decode(UINT4 *output, unsigned char *input,
    unsigned int len);
#ifdef __cplusplus
};
#endif

#endif /*_MD5_H*/
