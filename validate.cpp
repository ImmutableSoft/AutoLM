/***********************************************************************/
/*                                                                     */
/*   Module:  validate.cpp                                             */
/*   Version: 2020.0                                                   */
/*   Purpose: Command line local activation file blockchain validation */
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
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "autolm.h"

int main(int argc, const char **argv)
{
  AutoLm *lm = new AutoLm();

//(char *vendorStr, char* appStr, 
//    char* hexcompid, char *password, int mode, char* filename)
  // Entity, EntityId, Application, AppId, Mode, Password, CompId, Filename
  if ((argc < 8) || (argc > 9))
  {
    printf("Invalid number of arguments %d", argc);
    puts("");
    puts("validate <entity name> <entity id> <app name> <app id>");
    puts("         <mode> <password> <file name>");
    puts("");
    puts("  Validate a locate product activation license file");
    puts("");
    puts("  <entity name> is Immutable Ecosystem Entity name");
    puts("  <entity id> is Immutable Ecosystem Entity Id");
    puts("  <product name> is Immutable Ecosystem Product name");
    puts("  <product id> is Immutable Ecosystem Product Id");
    puts("  <mode> is authenticate mode, 2 is MD5, 3 is SHA1");
    puts("  <password> password string, supports escape characters ie. 'Passw\\0rd'");
    puts("  <infura id> the product ID from Infura.io");
    puts("  [file name] Optional. Default is ./license.elm");

    return -1;
  }

  char vendorPassword[20 + 1];
  int nVendorPwdLength = 10; // init vendor password length

  const char* strVendorPassword = argv[6];
  size_t nVendorPasswordStrLength = strlen(strVendorPassword);
  if (nVendorPasswordStrLength > 20)
  {
    printf("vendorPassword argument '%s' length is %zd, max value is 20 characters\n", strVendorPassword, nVendorPasswordStrLength);
    return 1;
  }
  else
  {
    //convert escape chars in password \char to char hex value
    int j = 0;
    int nPwdLength = 0;
    for (int i = 0; i < (int)nVendorPasswordStrLength; i++)
    {
      if (strVendorPassword[i] != '\\')
      {
        vendorPassword[j++] = strVendorPassword[i];
      }
      else
      {
        i++;
        if (isdigit(strVendorPassword[i]))
        {
          vendorPassword[j++] = char(strVendorPassword[i] - 0x30); // 0x30 - 0
        }
        else if (isxdigit(strVendorPassword[i]))
        {
          if (isupper(strVendorPassword[i]))
          {
            vendorPassword[j++] = char(strVendorPassword[i] - 0x41 + 10); // 0x65 - A
          }
          else
          {
            vendorPassword[j++] = char(strVendorPassword[i] - 0x61 + 10); // 0x61 - a
          }
        }
        else
        {
          printf("vendorPassword char is not valid decimal or hex number - '%c'\n", strVendorPassword[i]);
          return 1;
        }
      }
      nPwdLength++;
    }
    nVendorPwdLength = nPwdLength;
  }
  PRINTF("vendorPassword - '%s', vendorPwdLength = %d\n", strVendorPassword, nVendorPasswordStrLength);

  PRINTF("vendorPassword - [");
  for (int i = 0; i < nVendorPwdLength; i++)
  {
    if (isalpha(vendorPassword[i]))
    {
      PRINTF("%c,", vendorPassword[i]);
    }
    else if (vendorPassword[i] < 10)
    {
      PRINTF("\\%c,", vendorPassword[i] + 0x30);
    }
    else
    {
      PRINTF("\\%c,", vendorPassword[i] - 10 + 0x41);
    }
  }
  PRINTF("], vendorPwdLength = %d\n", nVendorPwdLength);
  if (nVendorPwdLength < 10)
  {
    printf("vendor password length = %d. Needs to be 10 to 20 characters long\n", nVendorPwdLength);
    return 1;
  }

  int res;
  res = lm->AutoLmInit(argv[1], atoi(argv[2]), argv[3], atoi(argv[4]), atoi(argv[5]),
                       vendorPassword, nVendorPwdLength, NULL, argv[7]);

  PRINTF(" %d Validating license file...", res);
  if (res == 0)
  {
    time_t expireTime = 0;
    char buyHashId[67] = "";
    ui64 resultValue = 0;
    if (argc == 9)
      res = lm->AutoLmValidateLicense(argv[8], &expireTime, buyHashId, &resultValue);
    else if ((argc == 8) && ((argv[7][0] != '0') && (argv[7][1] != 'x')))
      res = lm->AutoLmValidateLicense("./license.elm", &expireTime, buyHashId, &resultValue);
    PRINTF(" validated %d, expires on %lu\n", res, expireTime);
    printf("%llu\n", resultValue);
    return (int)resultValue;
  }
  else
    printf("0\n");
}
