/***********************************************************************/
/*                                                                     */
/*   Module:  autolm.h                                                 */
/*   Version: 2020.0                                                   */
/*   Purpose: Header file for AutoLM, the Automatic License Manager    */
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
#include "base/common.h"
#include "base/sha1.h"
#include "base/md5.h"

/***********************************************************************/
/* Configuration                                                       */
/***********************************************************************/

/*
 * Build options
 */
// Ethereum Blockchain Connectivity Options
#define CURL_HOST_URL          ROPSTEN_INFURA_URL
#define ADD_BLOCK_CHAIN_CHECK  1  /* append blockchain special char */
#define BLOCK_CHAIN_CHAR       ':' /* use colon as special char */
#define ROPSTEN_INFURA_URL     "https://ropsten.infura.io/v3/"
#define LOCAL_GANACHE_URL      "http://localhost:8545/"


// Immutable Ecosystem
#define LICENSE_STATUS_ID      "0x9277d3d6" // Keccak256 ("licenseStatus(uint256, uint256, uint256)") = 0x9277d3d6b97556c788e9717ce4902c3a0c92314558dc2f0dad1e0d0727f04629
#define ROPSTEN_IMMUTABLE_LICENSE_CONTRACT "0x21027DD05168A559330649721D3600196aB0aeC2"
#define GANACHE_IMMUTABLE_LICENSE_CONTRACT "0x67B5656d60a809915323Bf2C40A8bEF15A152e3e"
#define IMMUTABLE_LICENSE_CONTRACT ROPSTEN_IMMUTABLE_LICENSE_CONTRACT

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
  noVendorMatch,
  noApplicationMatch,
  newerRevision,
  expiredLicense, 
  authenticationFailed, 
  authFieldInvalid,
  authFieldWrongLength,
  compidInvalid,
  serverFull, /* 10 */
  serverExpire,
  serverNoResponse,
  serverBuildError,
  serverParseError,
  serverRetry,
  serverLostState,
  dongleNotFound,
  blockchainVendoridNoMatch,
  blockchainProductidNoMatch,
  blockchainExpiredLicense,
  blockchainAuthenticationFailed,
  otherLicenseError
};

/*
** AutoLM configuration structure
*/
typedef struct AutoLmConfig
{
  char vendor[21];
  size_t vendorlen;
  ui64 vendorid;
  char product[21];
  size_t productlen;
  ui64 productid;
  ui8 password[21];
  int mode;
  ui64 (*blockchainValidate)(ui64, ui64, char*, char*, time_t*);
  int (*computer_id)(char *);
  char* infuraProductId;
} AutoLmConfig;

/***********************************************************************/
/* Global definitions                                                  */
/***********************************************************************/
// From compid.cpp
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

  int AutoLmInit(char* vendor, ui64 vendorId, char* product,
                    ui64 productId, int mode, char* password, 
                    unsigned int pwdLength, int (*computer_id)(char*),
                    char* infuraId);

  int AutoLmValidateLicense(char* filename, time_t *exp_date,
                            char* buyActivationId, ui64 *resultValue);
  int AutoLmCreateLicense(char* filename);

  /*********************************************************************/
  /* Private  declarations                                             */
  /*********************************************************************/
private:
  int AutoLmStringToHex(char *hexstring, ui8 *result);
  int AutoLmHashLicense(char *appstr, char *computerid,
                           ui8 *hashresult);
  void AutoLmPwdToKeyMd5(
     char *password,  /* IN */
     int passwordlen, /* IN */
     ui8 *locstr,   /* IN  - pointer to unique ID  */
     ui32 locstrlen,  /* IN  - length of unique ID */
     ui8 *key);     /* OUT - pointer to resulting 16-byte buffer */
  void AutoLmPwdToKeySha(
     char *password,  /* IN */
     int passwordlen, /* IN */
     ui8 *locstr,   /* IN  - pointer to unique ID  */
     ui32 locstrlen,  /* IN  - length of unique ID */
     ui8 *key);     /* OUT - pointer to resulting 20-byte buffer */
  int AutoLmCalculateHash(int type, ui8 *wholeMsg,
                           int wholeMsglen, ui8 *result);

  AutoLmConfig AutoLmOne;
  CSha Csha_inst;
  md5 Cmd5_inst;
};

#endif /* _AUTOLM_H */
