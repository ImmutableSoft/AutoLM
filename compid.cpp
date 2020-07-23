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
#include <Windows.h>
#include <string>
#include <tchar.h>


typedef struct _dmi_header
{
  BYTE type;
  BYTE length;
  WORD handle;
}dmi_header;

typedef struct _RawSMBIOSData
{
  BYTE    Used20CallingMethod;
  BYTE    SMBIOSMajorVersion;
  BYTE    SMBIOSMinorVersion;
  BYTE    DmiRevision;
  DWORD   Length;
  BYTE    SMBIOSTableData[];
}RawSMBIOSData;

static int get_dmi_system_uuid(const BYTE* p, short ver, char *result)
{
  int only0xFF = 1, only0x00 = 1;
  int i;

  // Null the result string
  result[0] = 0;

  // Huh?
  for (i = 0; i < 16 && (only0x00 || only0xFF); i++)
  {
    if (p[i] != 0x00) only0x00 = 0;
    if (p[i] != 0xFF) only0xFF = 0;
  }

  // Huh?
  if (only0xFF)
  {
    printf("Not Present");
    return -1;
  }

  // Depending on the version, read UUID
  if (ver >= 0x0206)
    sprintf(result, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
      p[3], p[2], p[1], p[0], p[5], p[4], p[7], p[6],
      p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
  else
    sprintf(result, "-%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
      p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
      p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);

  // Return string length
  return (int)strlen(result);
}

/***********************************************************************/
/* AutoLmMachineId: Find and return the Windows MachineGuid            */
/*                                                                     */
/*      Inputs: comp_id = OUT the MachineGuid from the Registry        */
/*                                                                     */
/*     Returns: length of OUT comp_id if success, otherwise error      */
/*                                                                     */
/***********************************************************************/
int AutoLmMachineId(char* comp_id)
{
  DWORD bufsize = 0;
  BYTE buf[65536] = { 0 };
  int ret = 0;
  RawSMBIOSData* Smbios;
  dmi_header* h = NULL;
  int flag = 1;

  ret = GetSystemFirmwareTable('RSMB', 0, 0, 0);
  if (!ret)
  {
    printf("Function failed!\n");
    return 1;
  }

  bufsize = ret;

  ret = GetSystemFirmwareTable('RSMB', 0, buf, bufsize);

  if (!ret)
  {
    printf("Function failed!\n");
    return 1;
  }

  Smbios = (RawSMBIOSData*)buf;
  BYTE* p = Smbios->SMBIOSTableData;

  if (Smbios->Length != bufsize - 8)
  {
    printf("Smbios length error\n");
    return 1;
  }

  for (int i = 0; i < Smbios->Length; i++) {
    h = (dmi_header*)p;

    if (h->type == 1) {
      char tmp[128];
      int rval;

      // Read out the UUID
      rval = get_dmi_system_uuid(p + 0x8, Smbios->SMBIOSMajorVersion * 0x100 + Smbios->SMBIOSMinorVersion, tmp);

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
    p += h->length;
    while ((*(WORD*)p) != 0) p++;
    p += 2;
  }

  return 0;
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
