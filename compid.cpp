/***********************************************************************/
/*                                                                     */
/*   Module:  compid.cpp                                               */
/*   Version: 2020.0                                                   */
/*   Purpose: Replaceable computer id for AutoLm, AutoLmMachineId      */
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
#ifdef _WINDOWS
#include <memory.h>
#include <string.h>
#include <string>
#include <windows.h>
#include "autolm.h"

/***********************************************************************/
/* string_from_HKLM: convert from HLKM (Registry) to wchar_t string    */
/*                                                                     */
/*      Inputs: appstr = the Registry subkey to read from              */
/*              regValue = the registry element to read                */
/*              valueBuf = OUT the resulting value from HKLM as string */
/*                                                                     */
/*     Returns: length of OUT (valueBuf) if success, otherwise error   */
/*                                                                     */
/***********************************************************************/
static int string_from_HKLM(const wchar_t* regSubKey,
                            const wchar_t* regValue,
                            wchar_t* valueBuf, int nBufSize)
{
  DWORD cbData = nBufSize * sizeof(wchar_t);
  memset(valueBuf, 0, cbData);
  LONG lResult;

  HKEY hkey;
  lResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE, regSubKey, 0,
                          KEY_WOW64_64KEY + KEY_READ, &hkey);
  if (lResult == 0)
  {
    lResult = RegGetValueW(hkey, L"", regValue, RRF_RT_REG_SZ,
                           NULL, valueBuf, &cbData);
#if AUTOLM_DEBUG
    if (lResult == 0)
    {
      char tmp[128];

      // Convert to C string form
      wcstombs(tmp, valueBuf, 128);
      tmp[127] = 0;

      PRINTF("Key SOFTWARE\\Microsoft\\Cryptography\\MachineGUID value - '%s'\n", tmp);
    }
    else
    {
      PRINTF("Error %d during RegGetValue of MachineGUID\n", lResult);
    }
#endif

    RegCloseKey(hkey);
  }
  else
  {
    PRINTF("Error %d opening HKEY_LOCAL_MACHINE, SOFTWARE\\Microsoft\\Cryptography\n", lResult);
  }
  if (lResult == ERROR_SUCCESS)
  {
    cbData /= sizeof(wchar_t);
    return cbData;
  }
  else
  {
    // Return negative error code
    if (lResult > 0)
      return -lResult;
    else
      return lResult;
  }
}


/***********************************************************************/
/* AutoLmMachineId: Find and return the Windows MachineGuid            */
/*                                                                     */
/*      Inputs: comp_id = OUT the MachineGuid from the Registry        */
/*                                                                     */
/*     Returns: length of OUT comp_id if success, otherwise error      */
/*                                                                     */
/***********************************************************************/
int AutoLmMachineId(char *comp_id)
{
    char tmp[128];
    int rval;
    const wchar_t *regSubKey = L"SOFTWARE\\Microsoft\\Cryptography";

    const wchar_t *regValue(L"MachineGuid");
    wchar_t valueFromRegistry[37];

    rval = string_from_HKLM(regSubKey, regValue, valueFromRegistry, 37);

    // Convert to C string form
    wcstombs(tmp, valueFromRegistry, 128);
    rval = (int)strlen(tmp);

    // Strip out dashes from the machine id
    for (int i = 0; i < rval; ++i)
    {
      if (tmp[i] == '-')
      {
        memmove(&tmp[i], &tmp[i + 1], rval - i);
        --rval;
      }
    }

    // Prepend with 0x indicating hex string and return
    sprintf(comp_id, "0x%s", tmp);
    return (int)strlen(comp_id);
}

#else /* Otherwise Linux/BSD/MacOS */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/***********************************************************************/
/* AutoLmMachineId: Find and return the Unix Dbus machine-id           */
/*                                                                     */
/*      Inputs: comp_id = OUT the dbus machine-id from Unix            */
/*                                                                     */
/*     Returns: length of OUT comp_id if success, otherwise error      */
/*                                                                     */
/***********************************************************************/
// Get the MachineId in string, hexidecimal format
//   OUT: comp_id array size must be 35 bytes or larger
int AutoLmMachineId(char *comp_id)
{
  char tmp[64];
  FILE *pFILE;

  tmp[0] = 0;
  pFILE = fopen("/var/lib/dbus/machine-id", "r");
  if(pFILE) {
    fgets(tmp, 32, pFILE);
    tmp[32] = 0;
  }
  sprintf(comp_id, "0x%s", tmp);
  return strlen(comp_id); // Return string length
}
#endif

#ifdef _STANDALONE
// To compile with GCC/MSYS2
//gcc -o compid.exe -D_WINDOWS -D_STANDALONE compid.cpp -mwindows -lstdc++ -ladvapi32
// To compile with GCC
//gcc -o compid.exe compid.cpp -lstdc++
int main()
{
  char tmp[128];

  if (AutoLmMachineId(tmp) > 0)
    puts(tmp);
}
#endif
