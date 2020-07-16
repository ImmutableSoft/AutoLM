#ifdef WIN32
// To compile with GCC/MSYS2
//gcc -o compid.exe -DWIN32 compid.cpp -mwindows -lstdc++ -ladvapi32

#include <iostream>
#include <string>
#include <exception>
#include <windows.h>

/*! \brief                          Returns a value from HKLM as string.
    \exception  std::runtime_error  Replace with your error handling.
*/
std::wstring GetStringValueFromHKLM(const std::wstring& regSubKey, const std::wstring& regValue)
{
    size_t bufferSize = 0xFFF; // If too small, will be resized down below.
    std::wstring valueBuf; // Contiguous buffer since C++11.
    valueBuf.resize(bufferSize);
    auto cbData = static_cast<DWORD>(bufferSize * sizeof(wchar_t));
    auto rc = RegGetValueW(
        HKEY_LOCAL_MACHINE,
        regSubKey.c_str(),
        regValue.c_str(),
        RRF_RT_REG_SZ,
        nullptr,
        static_cast<void*>((wchar_t *)valueBuf.data()),
        &cbData
    );
    while (rc == ERROR_MORE_DATA)
    {
        // Get a buffer that is big enough.
        cbData /= sizeof(wchar_t);
        if (cbData > static_cast<DWORD>(bufferSize))
        {
            bufferSize = static_cast<size_t>(cbData);
        }
        else
        {
            bufferSize *= 2;
            cbData = static_cast<DWORD>(bufferSize * sizeof(wchar_t));
        }
        valueBuf.resize(bufferSize);
        rc = RegGetValueW(
            HKEY_LOCAL_MACHINE,
            regSubKey.c_str(),
            regValue.c_str(),
            RRF_RT_REG_SZ,
            nullptr,
            static_cast<void*>((wchar_t *)valueBuf.data()),
            &cbData
        );
    }
    if (rc == ERROR_SUCCESS)
    {
        cbData /= sizeof(wchar_t);
        valueBuf.resize(static_cast<size_t>(cbData - 1)); // remove end null character
        return valueBuf;
    }
    else
    {
        throw std::runtime_error("Windows system error code: " + std::to_string(rc));
    }
}

int AutoLmMachineId(char *comp_id)
{
    char tmp[128];
    int rval;
    std::wstring regSubKey;
    regSubKey = L"SOFTWARE\\Microsoft\\Cryptography\\";

    std::wstring regValue(L"MachineGuid");
    std::wstring valueFromRegistry;
    try
    {
        valueFromRegistry = GetStringValueFromHKLM(regSubKey, regValue);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what();
        return -1;
    }
//    std::wcout << valueFromRegistry;
    std::wcstombs(tmp, valueFromRegistry.c_str(), 64);
//    sprintf(comp_id, "0x%s", valueFromRegistry.c_str());
    rval = strlen(tmp);
    for (int i = 0; i < rval; ++i)
    {
      if (tmp[i] == '-')
      {
        memmove(&tmp[i], &tmp[i + 1], rval - i);
        --rval;
      }
    }
    sprintf(comp_id, "0x%s", tmp);
    return strlen(comp_id);
}

#else /* Otherwise Linux/BSD/MacOS */
// To compile with GCC
//gcc -o compid.exe compid.cpp -lstdc++

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Get the MachineId in string, hexidecimal format
//   OUT: comp_id array size must be 35 bytes or larger
int AutoLmMachineId(char *comp_id)
{
  char tmp[64];
  FILE *pFILE;

  tmp[0] = '\0';
  pFILE = fopen("/var/lib/dbus/machine-id", "r");
  if(pFILE) {
    fgets(tmp, 32, pFILE);
    tmp[32] = '\0';
  }
  sprintf(comp_id, "0x%s", tmp);
  return strlen(comp_id); // Return string length
}
#endif

#ifdef _STANDALONE
int main()
{
  char tmp[128];

  if (imtAutoLmMachineId(tmp) > 0)
    puts(tmp);
}
#endif
