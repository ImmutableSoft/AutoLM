/***********************************************************************/
/*                                                                     */
/*   Module:  easylm.h                                                 */
/*   Version: 2010.0                                                   */
/*   Purpose: Header file for EasyLM, the Easy License Manager         */
/*                                                                     */
/*---------------------------------------------------------------------*/
/*                                                                     */
/*               Copyright 2008, Mibtonix Inc.                         */
/*                      ALL RIGHTS RESERVED                            */
/*                                                                     */
/*   Licensees have the non-exclusive right to use, modify, or extract */
/*   this computer program for software development at a single site.  */
/*   This program may be resold or disseminated in executable format   */
/*   only. The source code may not be redistributed or resold.         */
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
#define CURL_HOST_URL         ROPSTEN_INFURA_URL
#define ADD_BLOCK_CHAIN_CHECK 1  /* append blockchain check special char */
#define BLOCK_CHAIN_CHAR     ':' /* use colon as special char */
#define ROPSTEN_INFURA_URL   "https://ropsten.infura.io/v3/"
#define LOCAL_GANACHE_URL    "http://localhost:8545/"

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
  int vendorlen;
  ui64 vendorid;
  char product[21];
  int productlen;
  ui64 productid;
  ui8 password[21];
  int mode;
  int (*blockchainValidate)(ui64, ui64, char*, char*, time_t*);
  int (*computer_id)(char *);
  char* infuraProductId;
} AutoLmConfig;

/***********************************************************************/
/* Global definitions                                                  */
/***********************************************************************/

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
                            char* buyActivationId);
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
