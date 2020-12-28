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

#ifndef _CREATEONLY

#include "EthereumCalls.h"

#define BLOCK_CHAIN_CHAR           ':' /* use colon as special char */


#endif /* ifndef _CREATEONLY */

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
/* AutoLmInit: Initialize AutoLM with entity/product credentials       */
/*                                                                     */
/*      Inputs: entity = full entity name (may change)                 */
/*              entityId =  the Immutable Entity Id                    */
/*              product =  full product name (may change)              */
/*              productId = the Immutable Entity specific Product Id   */
/*              mode = cryptographic algorithm to use for hash         */
/*              password =  password seeded into cryptographic hash    */
/*              pwdLength =  password length in bytes                  */
/*              computer_id =  optional function to generate comp id   */
/*              infuraId =  the Infura product Id assigned the creator */
/*                                                                     */
/*     Returns: 0 if success, otherwise an error occurred              */
/*                                                                     */
/***********************************************************************/
int DECLARE(AutoLm) AutoLmInit(const char* entity, ui64 entityId,
                               const char* product, ui64 productId,
                               int mode, const char* password,
                               ui32 pwdLength, int (*computer_id)(char*),
                               const char* infuraId)
{
  ui8 compid_octet[36];
  int compidlen;

  if ((entity == NULL) || (product == NULL) ||
      (password == NULL))
    return otherLicenseError;

#ifndef _CREATEONLY
  if (infuraId == NULL)
    return otherLicenseError;
#endif

  strcpy(AutoLmOne.entity, entity);
  AutoLmOne.entitylen = strlen(AutoLmOne.entity);
  AutoLmOne.entityid = entityId;
 
  strcpy(AutoLmOne.product, product);
  AutoLmOne.productlen = strlen(AutoLmOne.product);
  AutoLmOne.productid = productId;
#if 0

#ifdef _CREATEONLY
  AutoLmOne.blockchainValidate = NULL;
#else
  AutoLmOne.blockchainValidate = validate_onchain;
  if (infuraId)
    strcpy(AutoLmOne.infuraProductId, infuraId);
#endif

#endif

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
  return 0;
}

#ifndef _CREATEONLY

/***********************************************************************/
/* AutoLmValidateLicense: Determine validity of a license file         */
/*                                                                     */
/*       Input: filename = full filename of license file (may change)  */
/*     Outputs: exp_date = the resulting expiration day/time           */
/*              buyHashId = resulting activation hash to purchase      */
/*              languages = resulting language limitations             */
/*              version_plat = resulting version or platform limits    */
/*                                                                     */
/*     Returns: the immutable value of license, otherwise zero (0)     */
/*                                                                     */
/***********************************************************************/
int DECLARE(AutoLm) AutoLmValidateLicense(const char *filename,
                    time_t *exp_date, char* buyHashId, ui64 *languages,
                    ui64 *version_plat)
{
  FILE *pFILE;
  char tmp[170],tmpstr[170], tmpstr1[170];
  char loc_hostEntityProductIds[170];
  int nBuffersSize = 170; // vendorId/productId are 20 chars max length
  char loc_hash[44]; /* hash is 20 octets for SHA1 and 16 for MD5 */
                     /* as a string it could be up to 44 characters */
  char loc_hostid[120], loc_entity[20], loc_app[30];
  char loc_entityId[40], loc_productId[40];
  char* stopstring;
  ui8 gen_lMAC[20], hash_octet[20];
  int i, authlen, hashlen, rval, hostidlen;
  ui64 loc_entityid = 0, loc_productid = 0;
  bool in_entity = false, in_app = false, app_parsed = false;

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
        /* Read and check the entity name with the configured entity   */
        /*-------------------------------------------------------------*/
        in_entity = false;
        strchr(tmp, ']')[0] = 0;

        if (strlen(tmp) <= 20)
          strcpy(loc_entity, &(tmp[1]));
        else
          continue;

        if (strcmp(loc_entity, AutoLmOne.entity) == 0)
          in_entity = true;
      }
      else if (in_entity)
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
          strcpy(loc_hostEntityProductIds, tmpstr1);
          tmpstr1[strlen(tmpstr1) - 1] = 0;
          PRINTF("requires block chain Validation()\n");

          /*-----------------------------------------------------------*/
          /* Copy entity/product id to tmpstr1, and null terminate     */
          /* computer/host id in tmp.                                  */
          /*-----------------------------------------------------------*/
          strcpy(tmpstr1, &(strchr(tmpstr1, ':')[1]));
          strchr(tmp, ':')[0] = 0; // Null terminate at the colon

          /*-----------------------------------------------------------*/
          /* Copy computer id if the id length is valid.               */
          /*   2 for 0x and 32 for 16 hex.                             */
          /*-----------------------------------------------------------*/
          if (strlen(tmp) <= 34)
              strcpy(loc_hostid, tmp);
          else
            continue;

          /*-----------------------------------------------------------*/
          /* Copy the entity Id from between the colons.               */
          /*-----------------------------------------------------------*/
          strcpy(tmp, &(strchr(tmpstr1, ':')[1]));
          strchr(tmpstr1, ':')[0] = 0; // Null terminate at the colon
          if (strlen(tmpstr1) <= 40)
            strcpy(loc_entityId, tmpstr1);
          else
            continue;

          /*-----------------------------------------------------------*/
          /* Convert entity Id to integer and check for a match.       */
          /*-----------------------------------------------------------*/
          loc_entityid = strtoull(loc_entityId, &stopstring, 10);
          if (loc_entityid != AutoLmOne.entityid)
          {
            rval = blockchainEntityIdNoMatch;
            continue;
          }
          if (strlen(tmp) <= 40)
            strcpy(loc_productId, tmp);
          else
            continue;

          /*-----------------------------------------------------------*/
          /* Convert product Id to integer and check for a match.      */
          /*-----------------------------------------------------------*/
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

          // Retrieve the globally unique computer or machine Id
          if (AutoLmOne.getComputerId(machineId) <= 0)
          {
            rval = compidInvalid;
            continue;
          }

          // Compare the computer id lengths
          hostidlen = (int)strlen(loc_hostid);
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
              goto done;
            }
          }
        }
        else
        {
          rval = compidInvalid;
          goto done;
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

        // Copy back the entity/product ids before hashing
        strcpy(loc_hostid, loc_hostEntityProductIds);

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
    if (in_entity && in_app && app_parsed && (rval == licenseValid))
    {
      PRINTF("llEntityId = %llu, llProductId = %llu\n", loc_entityid,
             loc_productid);

      // Query the Ethereum database for the activation value
      rval = EthereumValidateActivation(loc_entityid, loc_productid,
                                        loc_hash, // hash is activation
                                        AutoLmOne.infuraProductId,
                                        exp_date, languages, version_plat);

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
#endif /* ifndef _CREATEONLY */

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
  /* Add the entity name                                               */
  /*-------------------------------------------------------------------*/
  strcat(theMsg, AutoLmOne.entity);

  /*-------------------------------------------------------------------*/
  /* Add the application name                                          */
  /*-------------------------------------------------------------------*/
  strcat(theMsg, AutoLmOne.product);


  /*-------------------------------------------------------------------*/
  /* Add computerId to the license message                             */
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
        PRINTF("String length is not a multiple of 2");
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

#ifndef _VALIDATEONLY
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
  char infoString[308];
  char hashtext[42 + 1];
  ui8 hashstr[20];
  int i, hostidlen;
  char hostidstr[136], tmpstr[86];

  // Check for configuration errors
  if ((filename == NULL) || (AutoLmOne.entityid <= 0) ||
      (AutoLmOne.productid < 0) || (AutoLmOne.mode < 0))
    return -1;

  hostidlen = (int)strlen(AutoLmOne.computerId);
  if (hostidlen > 0)
  {
    /*-----------------------------------------------------------------*/
    /* Add the entity and the product id for immutable ecosystem       */
    /*-----------------------------------------------------------------*/
    snprintf(hostidstr, 135, "%s:%llu:%llu:", AutoLmOne.computerId,
             AutoLmOne.entityid, AutoLmOne.productid);
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
        sprintf(infoString, "[%s]\n", AutoLmOne.entity);
        fputs(infoString, pFILE);
        sprintf(infoString, "%s %s %s\n", AutoLmOne.product,
                hostidstr, hashtext);
        fputs(infoString, pFILE);
        fclose(pFILE);
      }
      else
        return -5; // could not open for writing
    }
  }
  else
  {
    return -4;
  }
  return 0;
}
#endif /* ifndef _VALIDATEONLY */

/***********************************************************************/
/* AutoLmPwdStringToBytes: Convert escape characters to byte values    */
/*                                                                     */
/*       Input: password = the password string array                   */
/*      Output: byteResult = the resulting byte array                  */
/*                                                                     */
/*     Returns: The byte buffer result length                          */
/*                                                                     */
/***********************************************************************/
int DECLARE(AutoLm) AutoLmPwdStringToBytes(const char* password,
                                           char *byteResult)
{
  int entityPwdLength = 10; // init entity password length
  const char *entityPassword = password;
  size_t entityPasswordLength = strlen(entityPassword);

  if (entityPasswordLength > 20)
  {
    PRINTF("entityPassword '%s' length %zd, max length is 20\n",
           entityPassword, entityPasswordLength);
    return -1;
  }

  //convert escape chars in password \char to char hex value
  int j = 0;
  int nPwdLength = 0;
  for (int i = 0; i < (int)entityPasswordLength; i++)
  {
    if (entityPassword[i] == '\\')
    {
      i++;
      if (isdigit(entityPassword[i]))
      {
        byteResult[j++] = char(entityPassword[i] - 0x30);// 0x30 - 0
      }
      else if (isxdigit(entityPassword[i]))
      {
        if (isupper(entityPassword[i]))
          byteResult[j++] = char(entityPassword[i] - 0x41 + 10); // 0x65 - A
        else
          byteResult[j++] = char(entityPassword[i] - 0x61 + 10); // 0x61 - a
      }
      else
      {
        printf("entityPassword char not valid decimal or hex number - '%c'\n",
               entityPassword[i]);
        return -1;
      }
    }
    else
      byteResult[j++] = entityPassword[i];

    nPwdLength++;
  }

  entityPwdLength = nPwdLength;

#if AUTOLM_DEBUG
  PRINTF("entityPassword - '%s', entityPwdLength = %zd\n", entityPassword,
         entityPasswordLength);
  PRINTF("entityPassword - [");

  for (int i = 0; i < entityPwdLength; i++)
  {
    if (byteResult[i] > 15)
    {
      PRINTF("%c,", byteResult[i]);
    }
    else if (byteResult[i] < 10)
    {
      PRINTF("\\%c,", byteResult[i] + 0x30);
    }
    else
    {
      PRINTF("\\%c,", byteResult[i] - 10 + 0x41);
    }
  }
  PRINTF("], entityPwdLength = %d\n", entityPwdLength);
#endif /* AUTOLM_DEBUG */

  // Ensure password is of minimum length
  if (entityPwdLength < 10)
  {
    printf("entity password length = %d. Must be 10 to 20 characters\n",
           entityPwdLength);
    return -1;
  }

  return entityPwdLength;
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
  const ui8 *locstr,   /* IN  - pointer to unique id  */
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
  const ui8 *locstr,   /* IN  - pointer to unique ID  */
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
