/***********************************************************************/
/*                                                                     */
/*   Module:  activate.cpp                                             */
/*   Version: 2020.0                                                   */
/*   Purpose: Command line local activation file creation              */
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

/***********************************************************************/
/* OverrideMachineId: Save and return the machine id                   */
/*                                                                     */
/*      Inputs: comp_id = OUT the dbus machine-id from Unix            */
/*                                                                     */
/*     Returns: length of OUT comp_id if success, otherwise error      */
/*                                                                     */
/***********************************************************************/
// Get the MachineId in string, hexidecimal format
//   OUT: comp_id array size must be 35 bytes or larger
int OverrideMachineId(char* comp_id)
{
  static char ComputerId[64] = "";

  // First time initialize
  if (ComputerId[0] == 0)
    strcpy(ComputerId, comp_id);
  strcpy(comp_id, ComputerId);
  return strlen(ComputerId);
}

int main(int argc, const char **argv)
{
  AutoLm *lm = new AutoLm();

//(char *vendorStr, char* appStr, 
//    char* hexcompid, char *password, int mode, char* filename)
  // Entity, EntityId, Application, AppId, Mode, Password, CompId, Filename
  if ((argc < 7) || (argc > 9))
  {
    printf("Invalid number of arguments %d", argc);
    puts("");
    puts("activate <entity name> <entity id> <app name> <app id>");
    puts("         <mode> <password> [comp id] [file name]");
    puts("");
    puts("  Create a locate product activation license file");
    puts("");
    puts("  <entity name> is Immutable Ecosystem Entity name");
    puts("  <entity id> is Immutable Ecosystem Entity Id");
    puts("  <product name> is Immutable Ecosystem Product name");
    puts("  <product id> is Immutable Ecosystem Product Id");
    puts("  <mode> is authenticate mode, 2 is MD5, 3 is SHA1");
    puts("  <password> password string, supports escape characters ie. 'Passw\\0rd'");
    puts("  [comp id] Optional. Computer/Machine Id, or current OS/PC if missing");
    puts("  [file name] Optional. Default is ./license.elm");

    return -1;
  }

  if ((argc >= 8) && ((argv[7][0] == '0') && (argv[7][1] == 'x')))
    OverrideMachineId((char *)argv[7]);

  char vendorPassword[20 + 1];
  int nVendorPwdLength = 10; // init vendor password length

  const char* strVendorPassword = argv[6];
  int nVendorPasswordStrLength = strlen(strVendorPassword);
  if (nVendorPasswordStrLength > 20)
  {
    printf("vendorPassword argument '%s' length is %d, max value is 20 characters\n", strVendorPassword, nVendorPasswordStrLength);
    return 1;
  }
  else
  {
    //convert escape chars in password \char to char hex value
    int j = 0;
    int nPwdLength = 0;
    for (int i = 0; i < nVendorPasswordStrLength; i++)
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
  printf("vendorPassword - '%s', vendorPwdLength = %d\n", strVendorPassword, nVendorPasswordStrLength);

  printf("vendorPassword - [");
  for (int i = 0; i < nVendorPwdLength; i++)
  {
    if (isalpha(vendorPassword[i]))
    {
      printf("%c,", vendorPassword[i]);
    }
    else if (vendorPassword[i] < 10)
    {
      printf("\\%c,", vendorPassword[i] + 0x30);
    }
    else
    {
      printf("\\%c,", vendorPassword[i] - 10 + 0x41);
    }
  }
  printf("], vendorPwdLength = %d\n", nVendorPwdLength);
  if (nVendorPwdLength < 10)
  {
    printf("vendor password length = %d. Needs to be 10 to 20 characters long\n", nVendorPwdLength);
    return 1;
  }

  int res;
  // If computer id passed, use instead of local OS/PC
  if ((argc >= 8) && ((argv[7][0] == '0') && (argv[7][1] == 'x')))
  {
    res = lm->AutoLmInit(argv[1], atoi(argv[2]), argv[3], atoi(argv[4]), atoi(argv[5]),
                         vendorPassword, nVendorPwdLength, OverrideMachineId, NULL);
  }

  // Otherwise use the default Computer Id
  else
  {
    res = lm->AutoLmInit(argv[1], atoi(argv[2]), argv[3], atoi(argv[4]), atoi(argv[5]),
                  vendorPassword, nVendorPwdLength, NULL, NULL);
  }
  printf(" %d Creating license file...", res);
  if (argc == 9)
    res = lm->AutoLmCreateLicense(argv[8]);
  else if ((argc == 8) && ((argv[7][0] != '0') && (argv[7][1] != 'x')))
    res = lm->AutoLmCreateLicense(argv[7]);
  else
    res = lm->AutoLmCreateLicense("./license.elm");
  printf("  created! %d\n", res);
}