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

#if AUTOLM_DEBUG
/***********************************************************************/
/* OverrideMachineId: Save and return the machine id                   */
/*                                                                     */
/*      Inputs: comp_id = IN/OUT the machine-id from User              */
/*                          First call initializes machine-id          */
/*                          Subsequent calls return machine-id         */
/*                                                                     */
/*     Returns: length of OUT comp_id if success, otherwise error      */
/*                                                                     */
/***********************************************************************/
int OverrideMachineId(char* comp_id)
{
    static char ComputerId[64] = "";

    // First time initialize
    if (ComputerId[0] == 0)
        strcpy(ComputerId, comp_id);
    strcpy(comp_id, ComputerId);
    return (int)strlen(ComputerId);
}
#endif

/***********************************************************************/
/*        main: Main application entry point                           */
/*                                                                     */
/*      Inputs: argc = the number of command line parameters (8 to 9)  */
/*              argv = array of individual command line parameters     */
/*                                                                     */
/*     Returns: Zero on successful license lookup, otherwise error     */
/*             license value output to stdio after querying blockchain */
/*             Output of zero is not valid, greater than zero is valid */
/*                                                                     */
/***********************************************************************/
int main(int argc, const char **argv)
{
  AutoLm *lm = new AutoLm();
  char entityPassword[20 + 1];
  int entityPwdLength;
  const char* strEntityPassword;
  size_t entityPasswordStrLength;
  int res;

#if AUTOLM_DEBUG
  // Entity, EntityId, App, AppId, Mode, Password, infuraId, <CompId>, <Filename>
  if ((argc < 8) || (argc > 10))
#else
  // Entity, EntityId, App, AppId, Mode, Password, infuraId, <Filename>
  if ((argc < 8) || (argc > 9))
#endif
  {
    printf("Invalid number of arguments %d", argc);
    puts("");
    puts("validate <entity name> <entity id> <app name> <app id>");
    puts("         <mode> <password> <infura id> <file name>");
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
#if AUTOLM_DEBUG
    puts("  [comp id] Optional. Computer/Machine Id, or current OS/PC if missing");
#endif
    puts("  [file name] Optional. Default is ./license.elm");

    return -1;
  }

  // Save the password string and length
  strEntityPassword = argv[6];
  entityPasswordStrLength = strlen(strEntityPassword);

  // Return error if password is to long
  if (entityPasswordStrLength > 20)
  {
    printf("vendorPassword argument '%s' length is %zd, max value is 20 characters\n", strEntityPassword, entityPasswordStrLength);
    return -1;
  }

  // Otherwise convert escape chars in password \char to char hex value
  else
    entityPwdLength = lm->AutoLmPwdStringToBytes(strEntityPassword, entityPassword);

#if AUTOLM_DEBUG
  // Assign override machine id to force CompId from parameter list
  if ((argc >= 9) && ((argv[8][0] == '0') && (argv[8][1] == 'x')))
      OverrideMachineId((char*)argv[8]);

  // Initialize the AutoLM class with the passed parameters
  // If computer id passed, use instead of local OS/PC
  if ((argc >= 9) && ((argv[8][0] == '0') && (argv[8][1] == 'x')))
  {
      res = lm->AutoLmInit(argv[1], atoi(argv[2]), argv[3], atoi(argv[4]), atoi(argv[5]),
          entityPassword, entityPwdLength, OverrideMachineId, argv[7]);
  }

  // Otherwise use the default Computer Id
  else
#endif
  {
      res = lm->AutoLmInit(argv[1], atoi(argv[2]), argv[3], atoi(argv[4]), atoi(argv[5]),
          entityPassword, entityPwdLength, NULL, argv[7]);
  }

  // If initialization success, validate the license file
  PRINTF(" %d Validating license file...", res);
  if (res == 0)
  {
    time_t expireTime = 0;
    char buyHashId[67] = "";
    ui64 languages = 0, version_plat = 0;
#if AUTOLM_DEBUG
    if (argc == 10)
      res = lm->AutoLmValidateLicense(argv[9], &expireTime, buyHashId, &languages, &version_plat);
    else if ((argc == 9) && ((argv[8][0] == '0') && (argv[8][1] == 'x')))
        res = lm->AutoLmValidateLicense("./license.elm", &expireTime, buyHashId, &languages, &version_plat);
    else
#endif
    if ((argc == 9) && ((argv[8][0] != '0') && (argv[8][1] != 'x')))
      res = lm->AutoLmValidateLicense(argv[8], &expireTime, buyHashId, &languages, &version_plat);
    else if (argc == 8)
        res = lm->AutoLmValidateLicense("./license.elm", &expireTime, buyHashId, &languages, &version_plat);
    PRINTF(" validated %d, expires on %u\n", res, (ui32)expireTime);
    printf("%llu\n", languages);
    printf("%llu\n", version_plat);
    return (int)res;
  }

  // Otherwise, initialization failed, return zero
  else
  {
    printf("0\n");
    return 0;
  }
}
