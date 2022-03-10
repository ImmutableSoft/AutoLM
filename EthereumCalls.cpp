/***********************************************************************/
/*                                                                     */
/*   Module:  EthereumCalls.cpp                                        */
/*   Version: 2020.0                                                   */
/*   Purpose: Implementation of calls into the Immutable Ecosystem     */
/*            Smart Contracts                                          */
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

/***********************************************************************/
/* pack_hash_to_bytes: Pack a hex string to 64 byte (256 bit) string.  */
/*                                                                     */
/*       Input: hash = the hash as a hex string                        */
/*      Output: buf = the resulting hash as a 64byte array             */
/*                                                                     */
/*    (ex. sha3 0x75c412bb58976d1b83bcade11e0df6679c47a281)            */
/*                                                                     */
/***********************************************************************/
static void pack_hash_to_bytes(const char* hash, char* buf)
{
  char * bytes = buf;
  size_t length = strlen(hash);
  ui32 i;

  // Fill result buffer with zero strings and NULL terminate
  for (i = 0; i < 64; i++)
    bytes[i] = '0';
  bytes[64] = 0;

  // Fill the buffer from the end with the hash
  for (i = 0; i < length; i++)
    bytes[(64 - 1) - i] = hash[(length - 1) - i];
}

/***********************************************************************/
/* pack_ll32bytes: Pack 64 bit ll into 32 byte array (256 bit integer).*/
/*                                                                     */
/*       Input: ll = the 64 bit unsigned integer                       */
/*      Output: buf = the resulting integer as a 32byte array          */
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
/* encode_activate_json: Encode JSON params for three parameter call   */
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
static void encode_activate_json(char* functionId, ui64 entitiId,
                                ui64 productId, char* hash, char* params)
{
  size_t nLen = strlen(functionId);

  // First encode the function identifier
  memcpy(params, functionId, nLen);
  params[nLen] = 0;

  // activateStatus(uint256 entityId, uint256 productId, uint256 hash)

  // The first parameter is the entity identifier
  pack_ll32bytes(entitiId, params + nLen); //1st param
  nLen = strlen(params);

  // Then encode the product identifier
  pack_ll32bytes(productId, params + nLen); //2nd param
  nLen = strlen(params);

  // Finally encode the activation identifier hash //3rd param
  pack_hash_to_bytes(&hash[2], params + nLen);
}

/***********************************************************************/
/* encode_authenticate_json: Encode JSON params for one parameter call */
/*               anyFunction(uint256)                                  */
/*                                                                     */
/*      Inputs: functionId = Keccak256(anyFunction(uint256))           */
/*              hash      = the file sha256 checksum and identifier    */
/*     Outputs: params    = the resulting parameters as a hex string   */
/*                                                                     */
/***********************************************************************/
static void encode_authenticate_json(char* functionId, const char* hash,
                                     char* params)
{
  size_t nLen = strlen(functionId);

  // First encode the function identifier
  memcpy(params, functionId, nLen);
  params[nLen] = 0;

  // productReleaseHashDetails(uint256 fileHash)

  // Encode the file sha256 checksum identifier
  pack_hash_to_bytes(hash, params + nLen); //1st param
}

#if 0 //Deprecated
/***********************************************************************/
/* unpack_param_value: Convert 256 bit integer hex string to ui64 value*/
/*                                                                     */
/*      Inputs: param = the 256 bit integer hex string                 */
/*                                                                     */
/*     Returns: the unpacked ui64 bit value of the parameter           */
/*                                                                     */
/***********************************************************************/
static ui64 unpack_256bit_value(char* param)
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
#endif

/***********************************************************************/
/* unpack_param_value: Convert 256 bit integer hex string to ui64 value*/
/*                                                                     */
/*      Inputs: param = the 256 bit integer hex string                 */
/*                                                                     */
/*     Returns: the unpacked ui64 bit value of the parameter           */
/*                                                                     */
/***********************************************************************/
static ui64 unpack_64bit_value(char* param)
{
  ui64 ll = 0;
  int n = 0;
  ui64 llpower16 = 0;
  for (int i = 0; i < 16; i++)
  {
    if (isdigit(param[15 - i]))
    {
      n = param[15 - i] - 0x30; // 0x30 - 0
    }
    else if (isxdigit(param[15 - i]))
    {
      if (isupper(param[15 - i]))
      {
        n = param[15 - i] - 0x41 + 10; // 0x65 - A
      }
      else
      {
        n = param[15 - i] - 0x61 + 10; // 0x61 - a
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
/* parse_activation_json: Parse resulting activateStatus() activation  */
/*                                                                     */
/*      Input: jsonResult = the 256 bit integer hex string             */
/*    Outputs: exp_dat = the resulting license expiration day/time     */
/*             languages = the resulting licensed language flags       */
/*             version_plat = the version and platform flags           */
/*                                                                     */
/*     Returns: licenseValid on success, otherwise error               */
/*                                                                     */
/***********************************************************************/
static int parse_activation_json(const char* jsonResult, time_t* exp_dat,
                                 ui64* languages, ui64 *version_plat)
{
  size_t cnt = 0;
  int ret = 0;
 
  PRINTF("parse_activation_json()\n jsonResult-%s\n", jsonResult);

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
      int nParamsCount = (int)cnt / 16;
      PRINTF("nParamsCount = %d\n", nParamsCount);

      // 2 X 256 bit parameters is 8 different 64 bit parameters
      if (nParamsCount == 8) // 8 64 bit parameters
      {
        ui64 llParams[8];
        for (int i = 0; i < nParamsCount; i++)
        {
          strncpy(s_tmp, &(tmpstr[i * 16]), 16);
          s_tmp[16] = 0;
          PRINTF("param[%d] - %s\n", i, s_tmp);
          llParams[i] = unpack_64bit_value(s_tmp);
        }

        // If entity, product and flags, expiration zero, not found
        if ((llParams[1] == 0) && (llParams[2] == 0))
        {
           if (exp_dat)
             *exp_dat = 0;
           if (languages)
             *languages = 0;
           if (version_plat)
             *version_plat = 0;

           PRINTF("No Activation present\n");
           ret = blockchainExpiredLicense;
        }

        // If the flags/expiration value is one or greater (valid), parse
        else if (llParams[1] > 0)
        {
          if (exp_dat)
          {
            // If expiration flag set, save the expiration
            if ((llParams[1] >> 32) & 1)
              *exp_dat = (llParams[1] & 0xFFFFFFFF); // lsb of bits 128-192
            else
              *exp_dat = 0; // lsb of bits 128-192
            PRINTF("Expiration set %d\n", (ui32)*exp_dat);
          }

          // Activation has limitation flag set
          if ((llParams[1] >> 32) & 2)
          {
            if (languages)
            {
              // Save the languages limitations
              *languages = llParams[2];
            }
            if (version_plat)
            {
              // Save the languages limitations
              *version_plat = llParams[3];
            }
            PRINTF("License valid, languages 0x%llx, version/plat 0x%llx\n",
                   *languages, *version_plat);
            ret = licenseValid;
          }

          // Activation has feature flag set
          else if ((llParams[1] >> 32) & 8)
          {
            if (languages)
            {
              // Save the languages limitations
              *languages = llParams[2];
            }
            if (version_plat)
            {
              // Save the languages limitations
              *version_plat = llParams[3];
            }
            PRINTF("Feature found, languages 0x%llx, version/plat 0x%llx\n",
                   *languages, *version_plat);
            ret = applicationFeature;
          }
        }

        // Otherwise not found/valid
        else
        {
           if (exp_dat)
             *exp_dat = 0;
           if (languages)
             *languages = 0;
           if (version_plat)
             *version_plat = 0;

           ret = blockchainExpiredLicense;
        }
      }
      else
      {
        PRINTF("json params count not valid count = %d\n",
               nParamsCount);
           if (exp_dat)
             *exp_dat = 0;
           if (languages)
             *languages = 0;
           if (version_plat)
             *version_plat = 0;

           ret = blockchainExpiredLicense;
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
/* parse_authentication_json: Parse productReleaseHashDetails() result */
/*                                                                     */
/*      Input: jsonResult = the 256 bit integer hex string             */
/*    Outputs: entityId = the resulting file entity identifier         */
/*             productId = the resulting file product identifier       */
/*             releaseId = the resulting file release identifier       */
/*             languages = the resulting licensed language flags       */
/*             version = the version of the release                    */
/*             uri = the official URI of the file (to download)        */
/*                                                                     */
/*     Returns: licenseValid on success, otherwise error               */
/*                                                                     */
/***********************************************************************/
static int parse_authentication_json(const char *jsonResult,
                    ui64 *entityId, ui64 *productId, ui64 *releaseId,
                    ui64 *languages, ui64 *version, char *uri)
{
  size_t cnt = 0;
  int ret = 0;

  PRINTF("parse_activation_json()\n jsonResult-%s\n", jsonResult);

  /*-------------------------------------------------------------------*/
  /* see if json result has 0x hex string                              */
  /*-------------------------------------------------------------------*/
  if (strstr(jsonResult, "0x"))
  {
    char tmpstr[1024], s_tmp[1024];
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
      int nParamsCount = (int)cnt / 16;
      PRINTF("nParamsCount = %d\n", nParamsCount);

      // 4 X 256 bit parameters is 16 different 64 bit parameters
      if (nParamsCount >= 16) // 16 64 bit parameters
      {
        ui64 llParams[16];
        for (int i = 0; i < 16; i++)
        {
          strncpy(s_tmp, &(tmpstr[i * 16]), 16);
          s_tmp[16] = 0;
          PRINTF("param[%d] - %s\n", i, s_tmp);
          llParams[i] = unpack_64bit_value(s_tmp);
        }

        // If entity and product not found (each a 256 bit integer)
        if ((llParams[3] == 0) && (llParams[7] == 0))
        {
          if (entityId)
            *entityId = 0;
          if (productId)
            *productId = 0;
          if (releaseId)
            *releaseId = 0;
          if (languages)
            *languages = 0;
          if (version)
            *version = 0;
          if (uri)
            uri[0] = '\0';

          PRINTF("No Authentication present\n");
          puts("error no entity/product");
          ret = blockchainNotFound;
        }

        // If the flags/expiration value is one or greater (valid), parse
        else
        {
          // Assign the return values if pointers are valid
          if (entityId)
            *entityId = llParams[3];
          if (productId)
            *productId = llParams[7];
          if (releaseId)
            *releaseId = llParams[11];
          if (languages)
            *languages = llParams[14];
          if (version)
            *version = llParams[15];

//          puts("authentication found");

          // If URI present, copy it
          if (uri && (cnt / 16 > 24))
          {
            char byte[3];
            tmpstr[cnt - 1] = '\0';

            // Convert each hex nibble of result URI into string
            for (int i = 16 * 24; i < (int)cnt; i += 2)
            {
              byte[0] = tmpstr[i];
              byte[1] = tmpstr[i + 1];
              byte[2] = '\0';
              uri[(i - (16 * 24)) / 2] = (char)strtol(byte, NULL, 16);
              if ((byte[0] == 0) && (byte[1] == 0))
                break;
            }
          }
          ret = licenseValid;
        }
      }
      else
      {
        PRINTF("json params count not valid count = %d\n",
          nParamsCount);
        if (entityId)
          *entityId = 0;
        if (productId)
          *productId = 0;
        if (releaseId)
          *releaseId = 0;
        if (languages)
          *languages = 0;
        if (version)
          *version = 0;
        if (uri)
          uri[0] = '\0';

        puts("count mismatch");
        ret = blockchainNotFound;
      }
    }
    else
    {
      puts("params error");

      PRINTF("json params not found returned 0x\n");
      ret = blockchainAuthenticationFailed;
    }
  }
  else
  {
    puts("params error2");
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
/* autolm_read_activation: activateStatus() contract call, parse result*/
/*                                                                     */
/*      Inputs: infuraId = the Infura ProductID to use                 */
/*              jsonData = the encoded Json data for function call     */
/*     Outputs: exp_dat = the expiration date read from the blockchain */
/*              languages = the resulting licensed language flags      */
/*              version_plat = the version and platform flags          */
/*                                                                     */
/*       Returns: the value of any license activation returned         */
/*                                                                     */
/***********************************************************************/
static int autolm_read_activation(char* infuraId, char* jsonData,
  time_t* exp_dat, ui64* languages, ui64* version_plat)
{
  int res;

  // For reading an HTML response string into memory with curl
  char curlResponseMemory[MAX_SIZE_JSON_RESPONSE];
  curlResponseMemory[0] = 0; // start with empty string

  // Initialize curl
  curl_global_init(CURL_GLOBAL_ALL);

  // Easy object to handle the connection.
  CURL* easy = curl_easy_init();

  /* send all data to this function  */
  curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION,
    curl_write_memory_callback);

  /* we pass our 'chunk' struct to the callback function */
  curl_easy_setopt(easy, CURLOPT_WRITEDATA, (void*)curlResponseMemory);

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

  struct curl_slist* head = NULL;

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
  if (strcmp(url, LOCAL_GANACHE_URL) == 0)
    sprintf(urlBuf, "%s", url);
  else
    sprintf(urlBuf, "%s%s", url, infuraId);

  PRINTF("URL = %s\n", urlBuf);
  curl_easy_setopt(easy, CURLOPT_URL, urlBuf);

  // Perform the HTTP request
  if (curl_easy_perform(easy) == 0)
  {
    // Parse the result value and expiration date
    res = parse_activation_json(curlResponseMemory, exp_dat, languages,
      version_plat);
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
/* autolm_read_authentication: productReleaseHashDetails() call,       */
/*                             parse result                            */
/*                                                                     */
/*      Inputs: infuraId = the Infura ProductID to use                 */
/*              jsonData = the encoded Json data for function call     */
/*     Outputs: entityId = the entity identifier that created file     */
/*              productId = the product identifier of the file         */
/*              releaseId = the release identifier of the file         */
/*              languages = the resulting licensed language flags      */
/*              version = the version of the release (X.X.X.X)         */
/*              uri = the official URI of the file (to download)       */
/*                                                                     */
/*       Returns: the value of any license activation returned         */
/*                                                                     */
/***********************************************************************/
static int autolm_read_authentication(const char *infuraId,
    const char *jsonData, ui64 *entityId, ui64 * productId,
    ui64 * releaseId, ui64 * languages, ui64 * version, char *uri)
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
//  curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L); //what ???
   //single call, disable keepalive
  curl_easy_setopt(easy, CURLOPT_TCP_KEEPALIVE, 0L);
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
    res = parse_authentication_json(curlResponseMemory,
      entityId, productId, releaseId, languages, version, uri);
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
/* Global function definitions                                         */
/***********************************************************************/

/***********************************************************************/
/* EthereumValidateActivation: validate a license hash with blockchain */
/*                                                                     */
/*      Inputs: entityId = the Entity Id (creator id) of application   */
/*              productId = the product Id of the application          */
/*              hashId = license activation hash identifier to check   */
/*              infuraId = the Infura ProductId to use for access      */
/*     Outputs: exp_date = expiration date of the activation (or 0)    */
/*              languages = language flags for the file                */
/*              version_plat = version and platform flags              */
/*                                                                     */
/*     Returns: the value of any license activation returned           */
/*                                                                     */
/***********************************************************************/
int EthereumValidateActivation(ui64 entityId, ui64 productId,
      char* hashId, char* infuraId, time_t* exp_date, ui64* languages,
      ui64 *version_plat)
{
  char jsonParams[(4 * 64) + 10 + 1];// 4x 256 values + 10 functionId + 1
  char funcId[] = ACTIVATE_STATUS_ID;

  // Encode the function parameters as JSON paramters
  encode_activate_json(funcId, entityId, productId, hashId, jsonParams);
  PRINTF("%s\n", jsonParams);

  char jsonDataPrefixBuf[256];
  const char* jsonDataPrefix =
    "{\"jsonrpc\":\"2.0\",\"method\": \"eth_call\", \"params\":[{\"to\": \"%s\", \"data\":\"";

  sprintf(jsonDataPrefixBuf, jsonDataPrefix, IMMUTABLE_ACTIVATE_CONTRACT);
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
                                languages, version_plat);
}

/***********************************************************************/
/* EthereumAuthenticateFile: lookup file authenticity on blockchain    */
/*                                                                     */
/*      Inputs: hashId = file SHA256 checksum hex string to lookup     */
/*              infuraId = Infura ProductId hex string used for access */
/*     Outputs: entityId = the Entity Id (creator id) of application   */
/*              productId = the product Id of the application          */
/*              releaseId = the product release index of file          */
/*              languages = the 64 bit language flags of file          */
/*              version = the version, 4 x 16 bits (X.X.X.X)           */
/*              uri = the URI string pointing to the release file      */
/*                                                                     */
/*     Returns: the zero on success, negative on error                 */
/*                                                                     */
/***********************************************************************/
int EthereumAuthenticateFile(const char* hashId,
  const char* infuraId, ui64* entityId, ui64* productId,
  ui64* releaseId, ui64* languages, ui64* version, char* uri)
{
  char jsonParams[(4 * 64) + 10 + 1];// 4x 256 values + 10 functionId + 1
  char funcId[] = CREATOR_STATUS_ID;

  // Encode the function parameters as JSON paramters
  encode_authenticate_json(funcId, hashId, jsonParams);
  PRINTF("%s\n", jsonParams);

  char jsonDataPrefixBuf[256];
  const char* jsonDataPrefix =
    "{\"jsonrpc\":\"2.0\",\"method\": \"eth_call\", \"params\":[{\"to\": \"%s\", \"data\":\"";

  // productReleaseHashDetails(uint256) is in ImmutableProduct contract
  sprintf(jsonDataPrefixBuf, jsonDataPrefix, IMMUTABLE_CREATOR_CONTRACT);
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
  return autolm_read_authentication(infuraId, jsonDataAll, entityId,
    productId, releaseId, languages, version, uri);
}
