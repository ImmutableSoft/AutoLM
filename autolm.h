/***********************************************************************/
/*                                                                     */
/*   Module:  autolm.h                                                 */
/*   Version: 2020.0                                                   */
/*   Purpose: Header file for AutoLM, the Automatic License Manager    */
/*            utilizing the Immutable Ecosystem                        */
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
#ifndef _AUTOLM_H
#define _AUTOLM_H
#ifdef _MIBSIM
#include "common.h"
#include "sha1.h"
#include "md5.h"
#else
#include "base/common.h"
#include "base/sha1.h"
#include "base/md5.h"
#endif
#include "EthereumCalls.h"

/***********************************************************************/
/* Type Definitions                                                    */
/***********************************************************************/
/*
** EasyLM response enumeration
*/
enum AutoLmResponse
{
  licenseValid = 0,
  noLicenseFile,
  noEntityMatch,
  noApplicationMatch,
  newerRevision,
  expiredLicense,
  authenticationFailed,
  authFieldInvalid,
  authFieldWrongLength,
  compidInvalid,
  blockchainEntityIdNoMatch, /* 10 */
  blockchainProductidNoMatch,
  blockchainExpiredLicense,
  blockchainNotFound,
  blockchainAuthenticationFailed,
  curlPerformFailed,
  applicationFeature, // Not an error necessarily
  otherLicenseError
};

/*
** AutoLM configuration structure
*/
typedef struct AutoLmConfig
{
  char entity[128];
  size_t entitylen;
  ui64 entityid;
  char product[128];
  size_t productlen;
  ui64 productid;
  ui8 password[21];
  int mode;
  int (*getComputerId)(char *);
  char computerId[35];
  char infuraProductId[35];
} AutoLmConfig;

/***********************************************************************/
/* Global definitions                                                  */
/***********************************************************************/
// From compid.cpp or equivalent
extern int AutoLmMachineId(char* comp_id);

/***********************************************************************/
/* Class  declarations                                                 */
/***********************************************************************/
class AutoLm
{
  /*********************************************************************/
  /* Public  declarations                                              */
  /*********************************************************************/
public:
  AutoLm();
  ~AutoLm();

  int AutoLmInit(const char* entity, ui64 entityId, const char* product,
                 ui64 productId, int mode, const char* password,
                 ui32 pwdLength, int (*computer_id)(char*),
                 const char* infuraId);

  int AutoLmValidateLicense(const char* filename, time_t *exp_date,
                            char* buyActivationId, ui64 *langauges,
                            ui64 *version_plat);
  int AutoLmCreateLicense(const char* filename);

  int AutoLmPwdStringToBytes(const char* password, char* byteResult);

private:
  /*********************************************************************/
  /* Private  declarations                                             */
  /*********************************************************************/
  int AutoLmStringToHex(const char *hexstring, ui8 *result);
  int AutoLmHashLicense(const char *appstr, const char *computerid,
                        ui8 *hashresult);
  void AutoLmPwdToKeyMd5(
     const char *password,  /* IN */
     int passwordlen, /* IN */
     const ui8 *locstr,   /* IN  - pointer to unique ID  */
     ui32 locstrlen,  /* IN  - length of unique ID */
     ui8 *key);     /* OUT - pointer to resulting 16-byte buffer */
  void AutoLmPwdToKeySha(
     const char *password,  /* IN */
     int passwordlen, /* IN */
     const ui8 *locstr,   /* IN  - pointer to unique ID  */
     ui32 locstrlen,  /* IN  - length of unique ID */
     ui8 *key);     /* OUT - pointer to resulting 20-byte buffer */
  int AutoLmCalculateHash(int type, ui8 *wholeMsg,
                          int wholeMsglen, ui8 *result);

  AutoLmConfig AutoLmOne;
  CSha Csha_inst;
  md5 Cmd5_inst;
};

#endif /* _AUTOLM_H */
