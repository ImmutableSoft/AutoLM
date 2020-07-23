/***********************************************************************/
/*                                                                     */
/*   Module:  common.h                                                 */
/*   Version: 2020.0                                                   */
/*   Purpose: Common Header for AutoLM                                 */
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
#ifndef _COMMON_H
#define _COMMON_H
 
/***********************************************************************/
/* Configuration                                                       */
/***********************************************************************/
#define MAX_MSG_SIZE     1372
#define MAX_AUTHKEY_LEN  32 /*MD5 is 16, SHA is 20, SHA256 is 32 */

/***********************************************************************/
/* Type Definitions                                                    */
/***********************************************************************/

/* -- utility definition -- */
typedef unsigned long  long ui64;
typedef unsigned int   ui32;
typedef unsigned short ui16;
typedef unsigned char  ui8;

/***********************************************************************/
/* Macro Definitions                                                   */
/***********************************************************************/
#ifdef __cplusplus
#define STATIC static
#define DECLARE(n) n::
#define FREE(n) delete n
#else
#define STATIC

#define DECLARE(n) 
#define FREE(n) free(n)
#endif /*__cplusplus*/

#endif /*_COMMON_H */

