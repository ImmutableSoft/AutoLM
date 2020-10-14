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

/***********************************************************************/
/* Configuration                                                       */
/***********************************************************************/

/*
 * Build options
 */

// Application Ethereum Blockchain Connectivity Options
//
// Be sure to use the same network for both URL and contract address
//   Both should be for Ropsten or Main Network
#define CURL_HOST_URL              ROPSTEN_INFURA_URL
#define IMMUTABLE_LICENSE_CONTRACT ROPSTEN_ACTIVATE_CONTRACT

// Options are Ropsten or Local Ganache. Main Network coming soon!
#define ROPSTEN_INFURA_URL         "https://ropsten.infura.io/v3/"
#define LOCAL_GANACHE_URL          "http://localhost:8545/"

// Debugging options
#define AUTOLM_DEBUG               0 // 1 to Enable debug output
#if AUTOLM_DEBUG
#define   PRINTF                   printf
#else
#define   PRINTF(...)          
#endif

// Immutable Ecosystem
//   DO NOT EDIT BELOW

// Deprecated 1.0
// Keccak256 ("licenseStatus(uint256, uint256, uint256)") =
// 0x9277d3d6b97556c788e9717ce4902c3a0c92314558dc2f0dad1e0d0727f04629
//#define LICENSE_STATUS_ID          "0x9277d3d6"

// Current 2.0
// Keccak256 ("activateStatus(uint256, uint256, uint256)") =
// 0x1da7d8648240b9b4db8c4f11fcd46bf2ccd74be6fdf20e075e3cd1541a3ecae1
#define LICENSE_STATUS_ID          "0x1da7d864"
// 1.0 deprecated
//#define ROPSTEN_LICENSE_CONTRACT   "0x21027DD05168A559330649721D3600196aB0aeC2"
#define ROPSTEN_ACTIVATE_CONTRACT   "0x5A516379F798b1D5b1875fb3efDCdbCfe199De42"
#define GANACHE_LICENSE_CONTRACT   "0x67B5656d60a809915323Bf2C40A8bEF15A152e3e"

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
  int (*blockchainValidate)(ui64, ui64, char*, char*, time_t*, ui64*, ui64*);
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
