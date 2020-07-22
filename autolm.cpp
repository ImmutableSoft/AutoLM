/***********************************************************************/
/*                                                                     */
/*   Module:  autolm.cpp                                               */
/*   Version: 2020.0                                                   */
/*   Purpose: Implementation of AutoLM, the Automated License Manager  */
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
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <memory.h>
#include <ctype.h>
#include "autolm.h"

#ifdef _MINGW
#define SOCKET int  // avoid winsock2.h/socket.h
#include "process.h"
#endif

#include "curl/curl.h"

#define MAX_SIZE_JSON_RESPONSE     1024
#define BLOCK_CHAIN_CHAR           ':' /* use colon as special char */

/*
 Example command line (Entity 2, Product 0, Activation 1)
 curl --data "{\"jsonrpc\":\"2.0\",\"method\": \"eth_call\", \"params\": [{\"to\": \"0x21027DD05168A559330649721D3600196aB0aeC2\", \"data\": \"0x9277d3d6000000000000000000000000000000000000000000000000000000000000000200000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001\"}, \"latest\"], \"id\": 1}" https://ropsten.infura.io/v3/<ProductId>
*/

#ifndef _ONLYCREATE

/***********************************************************************/
/* pack_hash_to_32bytes: Pack a hash string to 32 byte array.          */
/*                                                                     */
/*       Input: hash = the hash as a hex string                        */
/*      Output: buf = the resulting hash as a 32byte array             */
/*                                                                     */
/*    (ex. sha3 0x75c412bb58976d1b83bcade11e0df6679c47a281)            */
/*                                                                     */
/***********************************************************************/
static void pack_hash_to_32bytes(char* hash, char* buf)
{
  char * bytes = buf;

  // Fill result buffer with zero strings and NULL terminate
  for (int i = 0; i < 64; i++)
    bytes[i] = '0';
  bytes[64] = 0;

  // Fill the end of the buffer up with the hash
  for (int i = 0; i < 40; i++)
    bytes[63 - i] = hash[39 - i];
}

/***********************************************************************/
/* pack_ll32bytes: Pack 64 bit ll into 32 byte array (256 bit integer).*/
/*                                                                     */
/*       Input: ll = the 64 bit unsigned integer                       */
/*      Output: buf = the resulting integer as a 32byte array          */
/*                                                                     */
/*69 0x0000000000000000000000000000000000000000000000000000000000000045*/
/*                                                                     */
/***********************************************************************/
static void pack_ll32bytes(ui64 ll, char* buf)
{
  int modulo = -1;
  ui64 remain = ll;
  int bytesCount = 0;
  char* bytes = buf;
  int offset = 32 - 1;

  // Fill result buffer with zero strings and NULL terminate
  for (int i = 0; i < 64; i++)
            bytes[i] = '0';
  bytes[64] = 0;

  PRINTF("ll = %llu, ll = 0x%llx\n", ll, ll);

  while (remain > 0)
  {
    modulo = remain % 256;
    remain = remain / 256;
    if (!(remain == 0 && modulo == 0))
    {
      char temp[3];

      // String print the byte and copy to result
      sprintf(temp, "%02x", modulo);
      bytes[offset * 2] = temp[0];
      bytes[(offset * 2) + 1] = temp[1];
      bytesCount++;
      offset--;
    }
    else
      break;
  }
}

/***********************************************************************/
/* encode_json_params: Encode the JSON params for three parameter call */
/*               anyFunction(uint256, uint256, uint256)                */
/*                                                                     */
/*      Inputs: functionId = Keccak256(anyFunction(uint256, uint256,   */
/*                                                 uint256))           */
/*              entityId  = the unique entity identifier               */
/*              productId = the product identifier (entity dependent)  */
/*              hash      = the activation identifier                  */
/*     Outputs: params    = the resulting parameters as a hex string   */
/*                                                                     */
/***********************************************************************/
static void encode_json_params(char* functionId, ui64 entitiId,
                               ui64 productId, char* hash, char* params)
{
  size_t nLen = strlen(functionId);

  // First encode the function identifier
  memcpy(params, functionId, nLen);
  params[nLen] = 0;

  // The first parameter is the entity identifier
  pack_ll32bytes(entitiId, params + nLen); //3rd
  nLen = strlen(params);

  // Then encode the product identifier
  pack_ll32bytes(productId, params + nLen);
  nLen = strlen(params);

  // Finally encode the activation identifier hash
  pack_hash_to_32bytes(&hash[2], params + nLen);
}

/***********************************************************************/
/* unpack_param_value: Convert 256 bit integer hex string to ui64 value*/
/*                                                                     */
/*      Inputs: param = the 256 bit integer hex string                 */
/*                                                                     */
/*     Returns: the unpacked ui64 bit value of the parameter           */
/*                                                                     */
/***********************************************************************/
static ui64 unpack_param_value(char* param)
{
  ui64 ll = 0;
  int n = 0;
  ui64 llpower16 = 0;
  for (int i = 0; i < 64; i++)
  {
    if (isdigit(param[63 - i]))
    {
      n = param[63 - i] - 0x30; // 0x30 - 0
    }
    else if (isxdigit(param[63 - i]))
    {
      if (isupper(param[63 - i]))
      {
        n = param[63 - i] - 0x41 + 10; // 0x65 - A
      }
      else
      {
        n = param[63 - i] - 0x61 + 10; // 0x61 - a
      }
    }
    if (i == 0)
    {
      ll += n;
      llpower16 = 1;
    }
    else
    {
      llpower16 *= 16;
      if (n > 0)
      {
        ll += n * llpower16;
      }
    }
  }

  PRINTF("ll = %llu\n", ll);
  return ll;
}

/***********************************************************************/
/* parse_json_result: Parse resulting licenseStatus() activation value */
/*                                                                     */
/*      Inputs: param = the 256 bit integer hex string                 */
/*                                                                     */
/*     Returns: the unpacked ui64 bit value of the parameter           */
/*                                                                     */
/***********************************************************************/
static int parse_json_result(const char* jsonResult, time_t* exp_dat,
                             ui64* ret_value)
{
  size_t cnt = 0;
  int ret = 0;
 
  PRINTF("parse_json_result()\n jsonResult-%s\n", jsonResult);

  /*-------------------------------------------------------------------*/
  /* see if json result has 0x hex string                              */
  /*-------------------------------------------------------------------*/
  if (strstr(jsonResult, "0x"))
  {
    char tmpstr[3 * 64 + 5], s_tmp[3 * 64 + 5];
    const char* hexstring = strstr(jsonResult, "0x");
    strcpy(tmpstr, hexstring);
    strcpy(s_tmp, &(tmpstr[2]));
    strcpy(tmpstr, s_tmp);
    if (strchr(tmpstr, '"'))
    {
      strchr(tmpstr, '"')[0] = 0;
    }

    cnt = strlen(tmpstr);
    if (cnt > 0)
    {
      int nParamsCount = (int)cnt / 64;
      PRINTF("nParamsCount = %d\n", nParamsCount);
      if (nParamsCount == 3)
      {
        ui64 llParams[3];
        for (int i = 0; i < nParamsCount; i++)
        {
          strncpy(s_tmp, &(tmpstr[i * 64]), 64);
          s_tmp[64] = 0;
          PRINTF("param[%d] - %s\n", i, s_tmp);
          llParams[i] = unpack_param_value(s_tmp);
        }
        if ((llParams[0] == 0) && (llParams[1] == 0) &&
            (llParams[2] == 0))
        {
           if (exp_dat)
             *exp_dat = 0;
           if (ret_value)
             *ret_value = 0;

           ret = blockchainExpiredLicense;
        }

        // If the activation value is one or greater (valid), return it
        else if (llParams[0] >= 1)
        {
          if (exp_dat)
            *exp_dat = llParams[1];
          if (ret_value)
            *ret_value = llParams[0];

          ret = licenseValid;
        }

        // Otherwise not found/valid
        else
          ret = blockchainAuthenticationFailed;
      }
      else
      {
        PRINTF("json params count not valid count = %d\n",
               nParamsCount);
        ret = blockchainAuthenticationFailed;
      }
    }
    else
    {
      PRINTF("json params not found returned 0x\n");
      ret = blockchainAuthenticationFailed;
    }
  }
  else
  {
    PRINTF("json params not found returned\n");
    ret = blockchainAuthenticationFailed;
  }
  return ret;
}

/***********************************************************************/
/* curl_write_memory_callback: write the HTTP response to memory       */
/*                                                                     */
/*    Inputs: contents = the buffer in                                 */
/*            size = the inbound buffer size                           */
/*            nmemb = the resulting buffer current size                */
/*    Output: userp = the user supplied MemoryStruct buffer            */
/*                                                                     */
/*     Returns: the new size of the resulting MemoryStruct buffer      */
/*                                                                     */
/***********************************************************************/
static size_t curl_write_memory_callback(void *contents, size_t size,
                                         size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  char* resultBuffer = (char *)userp;
  size_t resultSize = strlen(resultBuffer);

  if(resultSize + realsize > MAX_SIZE_JSON_RESPONSE)
  {
    /* out of memory! */ 
    PRINTF("not enough memory (increase MAX_SIZE_JSON_RESPONSE)\n");
    return 0;
  }
 
  memcpy(&(resultBuffer[resultSize]), contents, realsize);
  resultBuffer[resultSize + realsize] = 0;
 
  return realsize;
}

/***********************************************************************/
/* autolm_read_activation: licenseStatus contract call, parse result   */
/*                                                                     */
/*      Inputs: infuraId = the Infura ProductID to use                 */
/*              jsonData = the encoded Json data for function call     */
/*      Output: exp_dat = the expiration date read from the blockchain */
/*                                                                     */
/*       Returns: the value of any license activation returned         */
/*                                                                     */
/***********************************************************************/
static int autolm_read_activation(char* infuraId, char* jsonData,
                                 time_t* exp_dat, ui64* ret_value)
{
  int res;

  // For reading an HTML response string into memory with curl
  char curlResponseMemory[MAX_SIZE_JSON_RESPONSE];
  curlResponseMemory[0] = 0; // start with empty string

  // Initialize curl
  curl_global_init(CURL_GLOBAL_ALL);

  // Easy object to handle the connection.
  CURL *easy = curl_easy_init();

  /* send all data to this function  */
  curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION,
                   curl_write_memory_callback);
 
  /* we pass our 'chunk' struct to the callback function */
  curl_easy_setopt(easy, CURLOPT_WRITEDATA, (void *)curlResponseMemory);
 
  // You can choose between 1L and 0L (enable verbose log or disable)
#if AUTOLM_DEBUG
  curl_easy_setopt(easy, CURLOPT_VERBOSE, 1L);
#else
  curl_easy_setopt(easy, CURLOPT_VERBOSE, 0L);
#endif
  curl_easy_setopt(easy, CURLOPT_HEADER, 1L);
  curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L); //what ???

  // Let's create an object which will contain a list of headers.
  PRINTF("jsonData = %s\n", jsonData);
  PRINTF("strlen (jsonData) = %d\n", (int)strlen(jsonData));

#ifdef _WINDOWS
#ifdef _OPENSSL
  // Point curl to a root certificate store (file required)
  curl_easy_setopt(easy, CURLOPT_CAINFO, "./cacert.pem");
#endif
#endif

  /* Post json data */
  curl_easy_setopt(easy, CURLOPT_POSTFIELDS, jsonData);
  unsigned int jsonDataLength = (int)strlen(jsonData);
  PRINTF("CURLOPT_POSTFIELDSIZE set to jsonDataLength = %d\n",
         jsonDataLength);
  /* set the size of the postfields data */
  curl_easy_setopt(easy, CURLOPT_POSTFIELDSIZE, jsonDataLength);

  struct curl_slist *head = NULL;
 
  // Set the content type header to application/json
  const char* headerContentType = "Content-type: application/json";
  PRINTF("headerContentType = %s\n", headerContentType);
  head = curl_slist_append(head, headerContentType);

  // Add the headers to the easy object.
  curl_easy_setopt(easy, CURLOPT_HTTPHEADER, head);

  // Your URL.
  const char* url;
  url = CURL_HOST_URL; //  "http://localhost:8545/"
  char urlBuf[128];
  if (strcmp(url, ROPSTEN_INFURA_URL) == 0)
    sprintf(urlBuf, "%s%s", url, infuraId);
  else
    sprintf(urlBuf, "%s", url);

  PRINTF("URL = %s\n", urlBuf);
  curl_easy_setopt(easy, CURLOPT_URL, urlBuf);
 
  // Perform the HTTP request
  if (curl_easy_perform(easy) == 0)
  {
    // Parse the result value and expiration date
    res = parse_json_result(curlResponseMemory, exp_dat, ret_value);
  }
  else
  {
    PRINTF("Error performing curl request");
    res = curlPerformFailed;
  }

  // Destroy easy curl objects and return the result
  curl_easy_cleanup(easy);
  curl_global_cleanup();
  return res;
}

/***********************************************************************/
/* validate_onchain: validate a license file on the blockchain         */
/*                                                                     */
/*      Inputs: entityId = the Entity Id (creator id) of application   */
/*              productId = the product Id of the application          */
/*              hashId = license activation hash identifier to check   */
/*              infuraId = the Infura ProductId to use for access      */
/*      Output: exp_date = the expiration date of the activation (or 0)*/
/*                                                                     */
/*     Returns: the value of any license activation returned           */
/*                                                                     */
/***********************************************************************/
int validate_onchain(ui64 entityId, ui64 productId, char* hashId,
                     char* infuraId, time_t* exp_date, ui64* ret_value)
{
  char jsonParams[(4 * 64) + 10 + 1];// 4x 256 values + 10 functionId + 1
  char funcId[] = LICENSE_STATUS_ID;

  // Encode the function parameters as JSON paramters
  encode_json_params(funcId, entityId, productId, hashId, jsonParams);
  PRINTF("%s\n", jsonParams);

  char jsonDataPrefixBuf[256];
  const char* jsonDataPrefix =
    "{\"jsonrpc\":\"2.0\",\"method\": \"eth_call\", \"params\":[{\"to\": \"%s\", \"data\":\"";

  sprintf(jsonDataPrefixBuf, jsonDataPrefix, IMMUTABLE_LICENSE_CONTRACT);
  PRINTF("jsonDataPrefixBuf = %s\n", jsonDataPrefixBuf);
  size_t nLengthPrefix = strlen(jsonDataPrefixBuf);

    // Hard code the suffix
    //   (TODO: make 8 a random id and check in response)
  char jsonDataSuffix[] = "\"}, \"latest\"],\"id\": 8}";
  PRINTF("jsonDataEndfix = %s\n", jsonDataSuffix);
  size_t nLengthSuffix = strlen(jsonDataSuffix);

  char jsonDataAll[2048];
  strcpy(jsonDataAll, jsonDataPrefixBuf);

  // Add the JSON encoded parameters to the buffer
  size_t nLen = strlen(jsonDataAll);
  size_t nLengthParams = strlen(jsonParams);
  strcpy(&jsonDataAll[nLen], jsonParams);

  // Add the data suffix, sanitize and call read_activation
  nLen = strlen(jsonDataAll);
  strcpy(&jsonDataAll[strlen(jsonDataAll)], jsonDataSuffix);
  size_t nDataLength = nLengthPrefix + nLengthParams + nLengthSuffix;
  jsonDataAll[nDataLength] = 0;
  PRINTF("jsonData = %s\n", jsonDataAll);
  return autolm_read_activation(infuraId, jsonDataAll, exp_date,
                                ret_value);
}
#endif /* ifndef _ONLYCREATE */

#ifdef __cplusplus
/***********************************************************************/
/* AutoLm: AutoLM constructor                                          */
/*                                                                     */
/***********************************************************************/
AutoLm::AutoLm()
{
  memset(&AutoLmOne, 0, sizeof(AutoLmConfig));
}

/***********************************************************************/
/* ~AutoLm: AutoLM destructor                                          */
/*                                                                     */
/***********************************************************************/
AutoLm::~AutoLm()
{
}
#endif

/***********************************************************************/
/* AutoLmInit: Initialize AutoLM with vendor/product credentials       */
/*                                                                     */
/*     Returns: 0 if success, otherwise an error occurred              */
/*                                                                     */
/***********************************************************************/
int DECLARE(AutoLm) AutoLmInit(const char* vendor, ui64 vendorId,
                               const char* product, ui64 productId,
                               int mode, const char* password,
                               ui32 pwdLength, int (*computer_id)(char*),
                               const char* infuraId)
{
  ui8 compid_octet[36];
  unsigned int compidlen;

  if ((vendor == NULL) || (product == NULL) ||
      (password == NULL))
    return otherLicenseError;

#ifndef _ONLYCREATE
  if (infuraId == NULL)
    return otherLicenseError;
#endif

  strcpy(AutoLmOne.vendor, vendor);
  AutoLmOne.vendorlen = strlen(AutoLmOne.vendor);
  AutoLmOne.vendorid = vendorId;
 
  strcpy(AutoLmOne.product, product);
  AutoLmOne.productlen = strlen(AutoLmOne.product);
  AutoLmOne.productid = productId;
#ifdef _ONLYCREATE
  AutoLmOne.blockchainValidate = NULL;
#else
  AutoLmOne.blockchainValidate = validate_onchain;
#endif
  if (infuraId)
    strcpy(AutoLmOne.infuraProductId, infuraId);
  AutoLmOne.getComputerId = computer_id;
  if (AutoLmOne.getComputerId == NULL)
    AutoLmOne.getComputerId = AutoLmMachineId;

  /*-------------------------------------------------------------------*/
  /* Set the authentication node; 1 - noAuth, 2 - MD5, 3 - SHA1        */
  /*-------------------------------------------------------------------*/
  AutoLmOne.mode = mode;
 
  /*-------------------------------------------------------------------*/
  /* Get and convert the computer id returning if error.               */
  /*-------------------------------------------------------------------*/
  compidlen = AutoLmOne.getComputerId(AutoLmOne.computerId);
  if (compidlen > 0)
    compidlen = AutoLmStringToHex(AutoLmOne.computerId, compid_octet);
  if (compidlen <= 0)
    return compidInvalid;

  /*-------------------------------------------------------------------*/
  /* Localize the computer id into the password.                       */
  /*-------------------------------------------------------------------*/
  if (AutoLmOne.mode == 2)
    AutoLmPwdToKeyMd5(password, pwdLength, compid_octet, compidlen,
                      AutoLmOne.password);
  else if (AutoLmOne.mode == 3)
    AutoLmPwdToKeySha(password, pwdLength, compid_octet, compidlen,
                      AutoLmOne.password);
  else
    return authenticationFailed;

  /*-------------------------------------------------------------------*/
  /* Clear the password from the stack                                 */
  /*-------------------------------------------------------------------*/
//  for (i = 0; i < pwdLength; i++)
//    password[i] = 0;

  return 0;
}

#ifndef _ONLYCREATE

/***********************************************************************/
/* AutoLmValidateLicense: Determine validity of a license file         */
/*                                                                     */
/*      Inputs: filename = full filename of license file (may change)  */
/*              exp_date = OUT the resulting expiration day/time       */
/*              buyHashId = OUT resulting activation hash to purchase  */
/*              buyHashId = OUT resulting value of the activation      */
/*                                                                     */
/*     Returns: the immutable value of license, otherwise zero (0)     */
/*                                                                     */
/***********************************************************************/
int DECLARE(AutoLm) AutoLmValidateLicense(const char *filename,
                    time_t *exp_date, char* buyHashId, ui64 *resultValue)
{
  FILE *pFILE;
  char tmp[170],tmpstr[170], tmpstr1[170];
  char loc_hostVendorProductIds[170];
  int nBuffersSize = 170; // vendorId/productId are 20 chars max length
  char loc_hash[44]; /* hash is 20 octets for SHA1 and 16 for MD5 */
                     /* as a string it could be up to 44 characters */
  char loc_hostid[120], loc_vendor[20], loc_app[30];
  char loc_vendorId[40], loc_productId[40];
  ui8 gen_lMAC[20], hash_octet[20];
  int i, authlen, hashlen, rval, hostidlen;
  ui64 loc_vendorid, loc_productid;
  bool in_vendor = false, in_app = false, app_parsed = false;

  rval = otherLicenseError;

  /*-------------------------------------------------------------------*/
  /* First, open the application license.elm file.                     */
  /*-------------------------------------------------------------------*/
  pFILE = fopen(filename, "r");

  /*-------------------------------------------------------------------*/
  /* If a license file was found (dongle or application), check it.    */
  /*-------------------------------------------------------------------*/
  if (pFILE)
  {
//    bool bBlockchainCheckNeeded = false;
    while (fgets(tmp, nBuffersSize, pFILE))
    {
      if ((tmp[0] == '[') && (strchr(tmp, ']') != NULL))
      {
        /*-------------------------------------------------------------*/
        /* Read and check the vendor name with the configured vendor   */
        /*-------------------------------------------------------------*/
        in_vendor = false;
        strchr(tmp, ']')[0] = 0;

        if (strlen(tmp) <= 20)
          strcpy(loc_vendor, &(tmp[1]));
        else
          continue;

        if (strcmp(loc_vendor, AutoLmOne.vendor) == 0)
          in_vendor = true;
      }
      else if (in_vendor)
      {
        /*-------------------------------------------------------------*/
        /* Read and check the application name with the requested      */
        /*-------------------------------------------------------------*/
        strcpy(tmpstr1, tmp);
        strcpy(tmpstr, tmp);

        /*-------------------------------------------------------------*/
        /* Make sure this isn't the last parameter                     */
        /*-------------------------------------------------------------*/
        if (strchr(tmpstr1, ' '))
          strchr(tmpstr1, ' ')[0] = 0;
        else
          continue;

        if (strlen(tmpstr1) <= 20)
          strcpy(loc_app, tmpstr1);
        else
        {
          rval = noApplicationMatch;
          continue;
        }

        if (strcmp(loc_app, AutoLmOne.product) == 0)
//        if (strcmp(loc_app, application) == 0)
          in_app = true;
        else
        {
          /*-----------------------------------------------------------*/
          /* this is not this app, so continue to the next line        */
          /*-----------------------------------------------------------*/
          in_app = false;
          continue;
        }

        /*-------------------------------------------------------------*/
        /* Read and check the computer id with the requested           */
        /*-------------------------------------------------------------*/
        strcpy(tmp, &(strchr(tmpstr, ' ')[1]));
        strcpy(tmpstr1, tmp);
        strcpy(tmpstr, tmp);
        if (strchr(tmpstr1, ' '))
          strchr(tmpstr1, ' ')[0] = 0;
        else
          continue;

        /*-------------------------------------------------------------*/
        /* If the id ends with blockchain check character.             */
        /*-------------------------------------------------------------*/
        if (tmpstr1[strlen(tmpstr1) - 1] == BLOCK_CHAIN_CHAR)
        {
          // Remove the trailing colon (:)
          strcpy(loc_hostVendorProductIds, tmpstr1);
          tmpstr1[strlen(tmpstr1) - 1] = 0;
          PRINTF("requires block chain Validation()\n");

          strcpy(tmpstr1, &(strchr(tmpstr1, ':')[1]));
          strchr(tmp, ':')[0] = 0; // remove entity id

          /*-----------------------------------------------------------*/
          /* If the id length valid: app 2 for 0x and 32 for 16 hex.   */
          /*-----------------------------------------------------------*/
          if (strlen(tmp) <= 34)
              strcpy(loc_hostid, tmp);
          else
            continue;

          strcpy(tmp, &(strchr(tmpstr1, ':')[1]));
          strchr(tmpstr1, ':')[0] = 0; // remove product id
          if (strlen(tmpstr1) <= 40)
            strcpy(loc_vendorId, tmpstr1);
          else
            continue;

          char* stopstring;
          loc_vendorid = strtoull(loc_vendorId, &stopstring, 10);
          if (loc_vendorid != AutoLmOne.vendorid)
          {
            rval = blockchainVendoridNoMatch;
            continue;
          }
          if (strlen(tmp) <= 40)
            strcpy(loc_productId, tmp);
          else
            continue;

          loc_productid = strtoull(loc_productId, &stopstring, 10);
          if (loc_productid != AutoLmOne.productid)
          {
            rval = blockchainProductidNoMatch;
            continue;
          }
        }
        else
        {
          // Error and continue, blockchain check is required
          rval = otherLicenseError;
          continue;
        }

        /*-------------------------------------------------------------*/
        /* Check that the computer id matches this computer,           */
        /* so convert the string to a hex number                       */
        /*-------------------------------------------------------------*/
        if (strcmp(loc_hostid, "0x") != 0)
        {
          char machineId[64];

          if (AutoLmMachineId(machineId) <= 0)
          {
            rval = compidInvalid;
            continue;
          }

          // Compare the computer id lengths
          hostidlen = strlen(loc_hostid);
          if (strlen(machineId) != (size_t)hostidlen)
          {
            rval = compidInvalid;
            continue;
          }

          // Compare each character of the hex string (4 bit nibble)
          for (i = 0; i < hostidlen; i++)
          {
            // Convert to uppercase and compare to avoid case issues
            if (toupper(machineId[i]) != toupper(loc_hostid[i]))
            {
              rval = compidInvalid;
              continue;
            }
          }
        }
        else
        {
          rval = compidInvalid;
          continue;
        }

        /*-------------------------------------------------------------*/
        /* Put the hash string of the license file into tmpstr1        */
        /*-------------------------------------------------------------*/
        strcpy(tmp, &(strchr(tmpstr, ' ')[1]));
        strcpy(tmpstr1, tmp);
        strcpy(tmpstr, tmp);
        if (strchr(tmpstr1, ' '))
          strchr(tmpstr1, ' ')[0] = 0;


        /*-------------------------------------------------------------*/
        /* Pre-configure HMAC length based on authentication mode.     */
        /*-------------------------------------------------------------*/
        if (AutoLmOne.mode == 3)
          authlen = 20;
        else
          authlen = 16;

        /*-------------------------------------------------------------*/
        /* Force string alignment, disregard ^M and \n                 */
        /*-------------------------------------------------------------*/
        if (strlen(tmpstr1) > (size_t)authlen * 2 + 2)
          tmpstr1[authlen * 2 + 2] = 0;

        /*-------------------------------------------------------------*/
        /* Return invalid if length greater max. SHA is 40 + 2         */
        /*  TODO: Increase this/check mode for SHA2 (256bit is 64 + 2) */
        /*-------------------------------------------------------------*/
        if (((int)strlen(tmpstr1)) > authlen * 2 + 2)
        {
            rval = authFieldInvalid;
            goto done;
        }

        // Copy the hash and mark the application as parsed
        strcpy(loc_hash, tmpstr1);
        app_parsed = true;

        /*-------------------------------------------------------------*/
        /* Convert hex string to hex value                             */
        /*-------------------------------------------------------------*/
        hashlen = AutoLmStringToHex(loc_hash, hash_octet);
        if (hashlen < 0)
        {
          rval = authFieldInvalid;
          goto done;
        }

        if (hashlen != authlen)
        {
          rval = authFieldWrongLength;
          app_parsed = false;
          goto done;
        }

        // Copy back the vendor/product ids before hashing
        strcpy(loc_hostid, loc_hostVendorProductIds);

        /*-------------------------------------------------------------*/
        /* Compute the expected hash for the license file              */
        /*-------------------------------------------------------------*/
        if (AutoLmHashLicense(loc_app, loc_hostid, gen_lMAC) == 0)
        {
          /*-----------------------------------------------------------*/
          /* Check that the expected hash matches the license file.    */
          /*-----------------------------------------------------------*/
          for (i = 0; i < authlen; i++)
          {
            if (gen_lMAC[i] != hash_octet[i])
            {
              rval = authenticationFailed;
              break;
            }
          }

          /*-----------------------------------------------------------*/
          /* If not all octets (bytes) match, the license is invalid.  */
          /*-----------------------------------------------------------*/
          if (i != authlen)
          {
            rval = authenticationFailed;
            app_parsed = false;
            goto done;
          }

          // Fall through on success
          rval = licenseValid;
        }
        else
        {
          rval = authenticationFailed;
          app_parsed = false;
          goto done;
        }
      }
    }

    // Close the license file
    fclose(pFILE);

    // If a license found and valid, check Ethereum database
    if (in_vendor && in_app && app_parsed && (rval == licenseValid))
    {
      PRINTF("llEntityId = %llu, llProductId = %llu\n", loc_vendorid,
             loc_productid);

      // Query the Ethereum database for the activation value
      rval = AutoLmOne.blockchainValidate(loc_vendorid, loc_productid,
                                          loc_hash, // hash is activation
                                          AutoLmOne.infuraProductId,
                                          exp_date, resultValue);

      // If the license is expired copy the activation id for caller
      if (rval == blockchainExpiredLicense)
      {
          strcpy(buyHashId, loc_hash);
          PRINTF("buyHashId-%s\n", buyHashId);
      }

      // Return success or error, resultValue has activation value
      return rval;
    }

    // Otherwise no acceptable license for this application
    else
      return noApplicationMatch;
  }
  else
    return noLicenseFile;
  return otherLicenseError;
done:
  fclose(pFILE);
  return rval;
}
#endif

/***********************************************************************/
/* imtAutoLmHashLicense: Calculate authentication hash of a license    */
/*                                                                     */
/*      Inputs: appstr = the application name                          */
/*              computerid = the computer identifier, in string form   */
/*      Output: hashresult = the resulting hash                        */
/*                                                                     */
/*     Returns: 0 if success, otherwise an error occurred              */
/*                                                                     */
/***********************************************************************/
int DECLARE(AutoLm) AutoLmHashLicense(const char *appstr,
                                      const char *computerid,
                                      ui8 *hashresult)
{
  char theMsg[120];
  int len;

  /*-------------------------------------------------------------------*/
  /* Create a long string with the license information                 */
  /*-------------------------------------------------------------------*/
  theMsg[0] = 0;

  /*-------------------------------------------------------------------*/
  /* Add the vendor name                                               */
  /*-------------------------------------------------------------------*/
  strcat(theMsg, AutoLmOne.vendor);

  /*-------------------------------------------------------------------*/
  /* Add the application name                                          */
  /*-------------------------------------------------------------------*/
  strcat(theMsg, AutoLmOne.product);


  /*-------------------------------------------------------------------*/
  /* Add the computerId and for blockchain vendorId, productId         */
  /*-------------------------------------------------------------------*/
  strcat(theMsg, computerid);
  len = (int)strlen(theMsg);

  /*-------------------------------------------------------------------*/
  /* Calculate the hash                                                */
  /*-------------------------------------------------------------------*/
  if (AutoLmCalculateHash(AutoLmOne.mode,(ui8 *)theMsg, len, hashresult))
    return 0;
  else
    return 1;
}

/***********************************************************************/
/* AutoLmStringToHex: convert a hex string to raw hex                  */
/*                                                                     */
/*       Input: hexstring = the hexidecimal string                     */
/*      Output: result = the resulting hexidecimal sequence            */
/*                                                                     */
/*     Returns: 0 if success, otherwise error code                     */
/*                                                                     */
/***********************************************************************/
int DECLARE(AutoLm) AutoLmStringToHex(const char *hexstring, ui8 *result)
{
  char tmpstr[80], s_tmp[80+2], s_tmp2[80];
  int i = 0, cnt = 0, tmpval;

  /*-------------------------------------------------------------------*/
  /* see if string starts with 0x meaning hex string                   */
  /*-------------------------------------------------------------------*/
  if (strstr(hexstring,"0x") == hexstring)
  {
    if (strlen(hexstring) > 2)
    {
      strcpy(tmpstr, hexstring);
      strcpy(s_tmp, &(tmpstr[2]));
      strcpy(tmpstr, s_tmp);
      cnt = (int)strlen(tmpstr);
      if (cnt%2)
      {
        return -1;
      }
      else
      {
        for (i = cnt/2, cnt = 0; i >= 0; i--)
        {
          if (strlen(tmpstr) == 0)
            break;
          strcpy(s_tmp, tmpstr);
          s_tmp[2] = 0;
          strcpy(s_tmp2, s_tmp);
          sprintf(s_tmp,"0x%s", s_tmp2); /* add the 0x */
          sscanf(s_tmp, "%x", &tmpval);
          result[cnt++] = (ui8)tmpval;
          strcpy(s_tmp, tmpstr);
          strcpy(tmpstr, &(s_tmp[2]));
        }
      }

    }
  }
  return cnt;
}

/***********************************************************************/
/* AutoLmCreateLicense: Create blockchain activation license file      */
/*                                                                     */
/*       Input: filename to write the blockchain license               */
/*                                                                     */
/*     Returns: 0 if success, otherwise error code                     */
/*                                                                     */
/***********************************************************************/
int DECLARE(AutoLm) AutoLmCreateLicense(const char* filename)
{
  char infoString[201];
  char hashtext[42 + 1];
  ui8 hashstr[20];
  int i, hostidlen;
  char hostidstr[136], tmpstr[86];

  // Check for configuration errors
  if ((filename == NULL) || (AutoLmOne.vendorid <= 0) ||
      (AutoLmOne.productid < 0) || (AutoLmOne.mode < 0))
    return -1;

  hostidlen = (int)strlen(AutoLmOne.computerId);
  if (hostidlen > 0)
  {
    /*-----------------------------------------------------------------*/
    /* Add the vendor and the product id for immutable ecosystem       */
    /*-----------------------------------------------------------------*/
    snprintf(hostidstr, 135, "%s:%llu:%llu:", AutoLmOne.computerId,
             AutoLmOne.vendorid, AutoLmOne.productid);
	}
  else
    return -2;

  /*-------------------------------------------------------------------*/
  /* Hash the license information                                      */
  /*-------------------------------------------------------------------*/
  if (AutoLmHashLicense(AutoLmOne.product, hostidstr,
                        hashstr) == 0)
  {
    /*-----------------------------------------------------------------*/
    /* convert hash to string representation of hex number             */
    /*-----------------------------------------------------------------*/
    strcpy(hashtext , "0x");
    for (i = 0; i < 20; i++)
    {
      sprintf(tmpstr, "%x", hashstr[i]);
      if (strlen(tmpstr) == 1)
      {
        tmpstr[1] = tmpstr[0];
        tmpstr[0] = '0';
        tmpstr[2] = 0;
      }
      strcat(hashtext, tmpstr);
    }
    if (filename)
    {
      FILE *pFILE = fopen(filename, "w");
      infoString[0] = 0;
      if (pFILE)
      {
        /*-------------------------------------------------------------*/
        /* Write the license to file                                   */
        /*-------------------------------------------------------------*/
        sprintf(infoString, "[%s]\n", AutoLmOne.vendor);
        fputs(infoString, pFILE);
        sprintf(infoString, "%s %s %s\n", AutoLmOne.product,
                hostidstr, hashtext);
        fputs(infoString, pFILE);
        fclose(pFILE);
      }
    }
  }
  else
  {
    return -4;
  }
  return 0;
}

/***********************************************************************/
/* AutoLmPwdToKeyMd5: localize the password using MD5                  */
/*                                                                     */
/*      Inputs: password = the password                                */
/*              passwordlen = the password length                      */
/*              locstr = the unique localization string                */
/*              locstrlen = the unique localization string             */
/*              key = the resulting localized key                      */
/*                                                                     */
/***********************************************************************/
void DECLARE(AutoLm) AutoLmPwdToKeyMd5(
  const char *password,  /* IN */
  int passwordlen, /* IN */
  ui8 *locstr,   /* IN  - pointer to unique id  */
  ui32 locstrlen,  /* IN  - length of id */
  ui8 *key)      /* OUT - pointer to caller 16-octet buffer */
{
  Md5Ctx  MD;
  ui8     password_buf[64];
  ui32    password_index = 0;
  ui32    count = 0, i;

  /*-------------------------------------------------------------------*/
  /* check for zero (0) length password                                */
  /*-------------------------------------------------------------------*/
  if (passwordlen <= 0)
    return;
  Cmd5_inst.Md5Init (&MD);   /* initialize MD5 */

  /*-------------------------------------------------------------------*/
  /* Use while loop until we've done 1 Megabyte                        */
  /*-------------------------------------------------------------------*/
  while (count < 1048576)
  {
    for (i = 0; i < 64; i++)
    {
      /*---------------------------------------------------------------*/
      /* Take the next octet of the password, wrapping                 */
      /* to the beginning of the password as necessary.                */
      /*---------------------------------------------------------------*/
      password_buf[i] = password[password_index++ % passwordlen];
    }
    Cmd5_inst.Md5Update (&MD, password_buf, 64);
    count += 64;
  }
  Cmd5_inst.Md5Final (&MD, key);  /* tell MD5 we're done */

  /*-------------------------------------------------------------------*/
  /* Now localize the key with the locstr and pass                     */
  /* through MD5 to produce final key                                  */
  /* May want to ensure that locstrlen <= 32,                          */
  /* otherwise need to use a buffer larger than 64                     */
  /*-------------------------------------------------------------------*/
  memcpy(password_buf, key, 16);
  memcpy(password_buf+16, locstr, locstrlen);
  memcpy(password_buf+16+locstrlen, key, 16);

  Cmd5_inst.Md5Init(&MD);
  Cmd5_inst.Md5Update(&MD, password_buf, 32+locstrlen);
  Cmd5_inst.Md5Final(&MD, key);
  return;
}

/***********************************************************************/
/* AutoLmPwdToKeySha: localize the password using SHA                  */
/*                                                                     */
/*      Inputs: password = the password                                */
/*              passwordlen = the password length                      */
/*              locstr = the unique localization string                */
/*              locstrlen = the unique localization string             */
/*              key = the resulting localized key                      */
/*                                                                     */
/***********************************************************************/
void DECLARE(AutoLm) AutoLmPwdToKeySha(
  const char *password,  /* IN */
  int passwordlen, /* IN */
  ui8 *locstr,   /* IN  - pointer to unique ID  */
  ui32 locstrlen,  /* IN  - length of unique ID */
  ui8 *key)      /* OUT - pointer to caller 20-octet buffer */
{
  ui8     password_buf[72];
  ui32      password_index = 0;
  ui32      count = 0, i;
  ShaCtx    SH;

  /*-------------------------------------------------------------------*/
  /* check for zero (0) length password                                */
  /*-------------------------------------------------------------------*/
  if (passwordlen <= 0)
    return;

  Csha_inst.ShaInit(&SH);   /* initialize SHA */

  /*-------------------------------------------------------------------*/
  /* Use while loop until we've done 1 Megabyte                        */
  /*-------------------------------------------------------------------*/
  while (count < 1048576)
  {
    for (i = 0; i < 64; i++)
    {
      /*---------------------------------------------------------------*/
      /* Take the next octet of the password, wrapping                 */
      /* to the beginning of the password as necessary.                */
      /*---------------------------------------------------------------*/
      password_buf[i] = password[password_index++ % passwordlen];
    }
    Csha_inst.ShaUpdate (&SH, password_buf, 64);
    count += 64;
  }
  Csha_inst.ShaFinal(&SH, key);    /* tell SHA we're done */

  /*-------------------------------------------------------------------*/
  /* Now localize the key with the locstr and pass                     */
  /* through SHA to produce final key                                  */
  /* May want to ensure that locstrlen <= 32,                          */
  /* otherwise need to use a buffer larger than 72                     */
  /*-------------------------------------------------------------------*/
  memcpy(password_buf, key, 20);
  memcpy(password_buf + 20, locstr, locstrlen);
  memcpy(password_buf + 20 + locstrlen, key, 20);

  Csha_inst.ShaInit(&SH);   /* initialize SHA */
  Csha_inst.ShaUpdate(&SH, password_buf, 40 + locstrlen);
  Csha_inst.ShaFinal(&SH, key);    /* tell SHA we're done */
  return;
}

/***********************************************************************/
/* AutoLmCalculateHash: Calculate the HMAC auth hash of a string       */
/*                                                                     */
/*      Inputs: type = the type of hash, 2 is MD5, 3 is SHA            */
/*              wholeMsg = the string/message to calculate hash of     */
/*              wholeMsglen = length the wholeMsg                      */
/*              result = the resulting hash                            */
/*                                                                     */
/*     Returns: TRUE (1) if success, FALSE (0) if error occured        */
/*                                                                     */
/***********************************************************************/
int DECLARE(AutoLm) AutoLmCalculateHash(int type, ui8 *wholeMsg,
  int wholeMsglen, ui8 *result)
{
  ui8 preMAC[MAX_AUTHKEY_LEN], K1[64], K2[64], IPAD[64], OPAD[64],
    extAuthKey[64];
  ui8 theMsg[MAX_MSG_SIZE + 64];
  Md5Ctx MD;
  ShaCtx SH;
  int i, loclen = 0;

  if ((type == 2 ) || (type == 10)) /* MD5 */
    loclen = 16;
  else if (type == 3) /* SHA */
    loclen = 20;
  else
    return 0; /* not supported */

  /*-------------------------------------------------------------------*/
  /*   Derived from the HMAC algorithm (see FIPS 140.1)                */
  /*   4)  From the secret authKey, two keys K1 and K2 are derived:    */
  /*                                                                   */
  /*       a) extend the authKey to 64 octets by appending 48 zero     */
  /*          octets; save it as extendedAuthKey                       */
  /*       b) obtain IPAD by replicating the octet 0x36 64 times;      */
  /*       c) obtain K1 by XORing extendedAuthKey with IPAD;           */
  /*       d) obtain OPAD by replicating the octet 0x5C 64 times;      */
  /*       e) obtain K2 by XORing extendedAuthKey with OPAD.           */
  /*-------------------------------------------------------------------*/
  for (i = 0; i < 64; i++)
  {
    IPAD[i] = 0x36;
    OPAD[i] = 0x5C;
    if (i < loclen)
      extAuthKey[i] = AutoLmOne.password[i];
    else
      extAuthKey[i] = 0;
    K1[i] = extAuthKey[i] ^ IPAD[i];
    K2[i] = extAuthKey[i] ^ OPAD[i];
  }

  /*-------------------------------------------------------------------*/
  /*   Derived from the HMAC algorithm (see FIPS 140.1)                */
  /*   5)  The MAC is calculated over the wholeMsg:                    */
  /*                                                                   */
  /*       a) prepend K1 to the wholeMsg and calculate the digest      */
  /*          over it;                                                 */
  /*       b) prepend K2 to the result of step 5.a and calculate the   */
  /*          MD5 digest over it;                                      */
  /*       c) first 12 octets of the result of step 5.b is the MAC.    */
  /*-------------------------------------------------------------------*/
  for (i = 0; i < wholeMsglen + 64; i ++)
  {
    if (i < 64)
      theMsg[i] = K1[i];
    else
      theMsg[i] = wholeMsg[i - 64];
  }
  if ((type == 2) || (type == 10))
  {
    Cmd5_inst.Md5Init(&MD);
    Cmd5_inst.Md5Update(&MD, theMsg, wholeMsglen + 64);
    Cmd5_inst.Md5Final(&MD, preMAC);
  }
  else if (type == 3)
  {
    Csha_inst.ShaInit(&SH);
    Csha_inst.ShaUpdate(&SH, theMsg, wholeMsglen + 64);
    Csha_inst.ShaFinal(&SH, preMAC);    /* tell SHA we're done */
  }
  else
    return 0;
  for (i = 0; i < 64 + loclen; i ++)
  {
    if (i < 64)
      theMsg[i] = K2[i];
    else
      theMsg[i] = preMAC[i - 64];
  }
  if ((type == 2) || (type == 10))
  {
    Cmd5_inst.Md5Init(&MD);
    Cmd5_inst.Md5Update(&MD, theMsg, 64 + loclen);
    Cmd5_inst.Md5Final(&MD, result);
  }
  else if (type == 3)
  {
    Csha_inst.ShaInit(&SH);
    Csha_inst.ShaUpdate(&SH, theMsg, 64 + loclen);
    Csha_inst.ShaFinal(&SH, result);    /* tell SHA we're done */
  }
  else
    return 0;
  return 1;
}
