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
#include "autolm.h"

// References:
// https://stackoverflow.com/questions/60700302/win32-api-to-get-machine-uuid
// https://docs.microsoft.com/en-us/windows/win32/cimwin32prov/win32-computersystemproduct?redirectedfrom=MSDN
//

// Firmware table is typically less than 3000 bytes?
#define FIRMWARE_TABLE_MAX_SIZE   4096

typedef struct _dmi_header
{
  ui8 type;
  ui8 length;
  ui16 handle;
} dmi_header;

typedef struct _RawSMBIOSData
{
  ui8 Used20CallingMethod;
  ui8 SMBIOSMajorVersion;
  ui8 SMBIOSMinorVersion;
  ui8 DmiRevision;
  ui32 Length;
  ui8 SMBIOSTableData[1];
} RawSMBIOSData;

/***********************************************************************/
/* get_dmi_system_uuid: Read UUID from byte array into result string   */
/*                                                                     */
/*      Inputs: comp_id = OUT the MachineGuid from the Registry        */
/*                                                                     */
/*     Returns: length of OUT comp_id if success, otherwise error      */
/*                                                                     */
/***********************************************************************/
static int get_dmi_system_uuid(const ui8* ptr, ui16 ver, char *result)
{
  int only0xFF = 1, only0x00 = 1;
  int i;

  // Null the result string
  result[0] = 0;

  // Check first 16 bytes for 0xFF or 0x00
  for (i = 0; i < 16 && (only0x00 || only0xFF); i++)
  {
    // If zero found, clear the only0x00 flag
    if (ptr[i] != 0x00)
      only0x00 = 0;

    // If 0xFF found, clear the only0xFF flag
    if (ptr[i] != 0xFF)
      only0xFF = 0;
  }

  // Return error if only flags were not cleared (why?)
  if (only0xFF || only0x00)
  {
    printf("Byte with all zeros (0x00) and all one's (0xFF) not found.");
    return -1;
  }

  // Depending on the version, convert UUID bytes into result string
  if (ver >= 0x0206)
    sprintf(result, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
      ptr[3], ptr[2], ptr[1], ptr[0], ptr[5], ptr[4], ptr[7], ptr[6],
      ptr[8], ptr[9], ptr[10], ptr[11], ptr[12], ptr[13], ptr[14], ptr[15]);
  else
    sprintf(result, "-%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
      ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7],
      ptr[8], ptr[9], ptr[10], ptr[11], ptr[12], ptr[13], ptr[14], ptr[15]);

  // Return the result string length
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
  ui32 bufsize = 0;
  ui8 buf[FIRMWARE_TABLE_MAX_SIZE] = { 0 };
  int ret = 0;
  RawSMBIOSData *Smbios;
  dmi_header *hdr = NULL;

  // Read the firmware table with NULL buffer to get size
  ret = GetSystemFirmwareTable('RSMB', 0, 0, 0);
  if (!ret)
  {
    printf("GetSystemFirmwareTable failed to read length.\n");
    return 1;
  }

  // Ensure buffer size is not too large
  bufsize = ret;
  if (bufsize > FIRMWARE_TABLE_MAX_SIZE)
  {
    printf("Firmware buffer size %d more than max %d.\n", bufsize,
           FIRMWARE_TABLE_MAX_SIZE);
    return -1;
  }

  // Read the firmware table into the buffer, return if error
  ret = GetSystemFirmwareTable('RSMB', 0, buf, bufsize);
  if (!ret)
  {
    printf("GetSystemFirmwareTable failed.\n");
    return -1;
  }

  // Cast the buffer to the raw SMBIOS data structure
  Smbios = (RawSMBIOSData*)buf;
  ui8 *ptr = Smbios->SMBIOSTableData;

  // If header length does not match buffer size return error
  if (Smbios->Length != bufsize - 8)
  {
    printf("Smbios length error\n");
    return -1;
  }

  // Loop through the SMBIOS data, reviewing each DMI header
  for (ui32 i = 0; i < Smbios->Length; i++)
  {
    hdr = (dmi_header*)ptr;

    // If DMI header type is System Information, read the UUID
    if (hdr->type == 1)
    {
      char tmp[128];
      int rval;

      // Read out the UUID based on version
      rval = get_dmi_system_uuid(ptr + 0x8, Smbios->SMBIOSMajorVersion * 0x100 +
                                 Smbios->SMBIOSMinorVersion, tmp);

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

    // Move to the next DMI header
    ptr += hdr->length;
    while ((*(ui16 *)ptr) != 0)
      ptr++;
    ptr += 2;
  }

  // If System Information missing, return empty machine id
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
