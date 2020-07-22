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
  return (int)strlen(ComputerId);
}

int main(int argc, const char **argv)
{
  AutoLm *lm = new AutoLm();
  char vendorPassword[20 + 1];
  int nVendorPwdLength;

  const char* strVendorPassword = argv[6];
  size_t nVendorPasswordStrLength = strlen(strVendorPassword);
  int res;

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

  // Assign override machine id to force CompId from parameter list
  if ((argc >= 8) && ((argv[7][0] == '0') && (argv[7][1] == 'x')))
    OverrideMachineId((char *)argv[7]);

  // Return error if password is to long
  if (nVendorPasswordStrLength > 20)
  {
    printf("vendorPassword argument '%s' length is %zd, max value is 20 characters\n", strVendorPassword, nVendorPasswordStrLength);
    return -1;
  }

  // Otherwise convert escape chars in password \char to char hex value
  else
    nVendorPwdLength = lm->AutoLmPwdStringToBytes(strVendorPassword, vendorPassword);

  // Initialize the AutoLM class with the passed parameters

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

  // If initialization success, create the license file
  if (res == 0)
  {
    PRINTF(" %d Creating license file...", res);
    if (argc == 9)
      res = lm->AutoLmCreateLicense(argv[8]);
    else if ((argc == 8) && ((argv[7][0] != '0') && (argv[7][1] != 'x')))
      res = lm->AutoLmCreateLicense(argv[7]);
    else
      res = lm->AutoLmCreateLicense("./license.elm");
    PRINTF("AutoLmCreateLicense result %d\n", res);
    return res;
  }

  // Otherwise, initialization failed, return error
  else
    PRINTF("Failed to initialize AutoLm %d\n", res);
  return -1;
}
