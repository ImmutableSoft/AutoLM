/***********************************************************************/
/*                                                                     */
/*   Module:  TestApplication.cpp example of licensing an application  */
/*   Version: 2020.0                                                   */
/*   Purpose: Demonstrating AutoLM, the Automated License Manager      */
/*            that utilizes the Immutable Ecosystem                    */
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
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "autolm.h"
#include "windows.h"

// Configuration
#define INFURA_PROJECT_ID "a2931dfbdd3dddc6233914717a744d19" // Change this!
#define LICENSE_FILE      "./license.elm"

static char BufValidate[100];
static bool Validated = false;

#ifndef _UNIX
#include "framework.h"
#include "TestApplication.h"

#define MAX_LOADSTRING    100

// Global Variables:
HINSTANCE hInst;                            // current instance
WCHAR szTitle[MAX_LOADSTRING];              // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];        // main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    //OutputDebugString(L"*** TestApplication");

    FILE* fp;
    AllocConsole();
    freopen_s(&fp, "CONIN$", "r", stdin);
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TESTAPPLICATION, szWindowClass,
                MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance,
                           MAKEINTRESOURCE(IDC_TESTAPPLICATION));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance,
                            MAKEINTRESOURCE(IDI_TESTAPPLICATION));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TESTAPPLICATION);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance,
                            MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}
#endif /* _UNIX */

/***********************************************************************/
/* launchPurchaseDialog: Launch Dapp to activation purchase page       */
/*                                                                     */
/*      Inputs: entityId = entity ID static in Immutable Ecosystem     */
/*              productId = product ID static in Immutable Ecosystem   */
/*              activationId = local activation identifier to activate */
/*                             in hex string format                    */
/*      Output: purchaseUrl = the resulting URL to open purchase page  */
/*                                                                     */
/*     Returns: result of execution (launch), zero if success          */
/*                                                                     */
/***********************************************************************/
int launchPurchaseDialog(ui64 entityId, ui64 productId,
                         const char* activationId, char *purchaseUrl)
{
  char bufLink[200];
  char bufLaunch[250];
  char entityIdStr[21];
  char productIdStr[21];

  // Convert the entity and product IDs to string equivalent
  sprintf(entityIdStr, "%llu", entityId);
  sprintf(productIdStr, "%llu", productId);

  // Create the purchaseUrl result string if not NULL
  if (purchaseUrl)
  {
    sprintf(purchaseUrl,
      "https://ecosystem.immutablesoft.org/?func=activation&entity=%s&product=%s&identifier=%s",
      entityIdStr, productIdStr, activationId);
  }

  // Create the URL with -app parameter for passing to the Chrome browser
  sprintf(bufLink,
//    "--app=https://ecosystem.immutablesoft.org/?func=activation&entity=%s&product=%s&identifier=%s",
    "https://ecosystem.immutablesoft.org/?func=activation&entity=%s&product=%s&identifier=%s",
    entityIdStr, productIdStr, activationId);
  PRINTF("bufLink - '%s'\n", bufLink);

  sprintf(bufLaunch, "start chrome.exe \"%s\"", bufLink);
  PRINTF("bufLaunch - '%s'\n", bufLaunch);
  int n = 0;
#ifndef _WINDOWS
  sprintf(bufLaunch, "/usr/bin/google-chrome \"%s\"", bufLink);
  PRINTF("lanching '%s'\n", bufLaunch);
  n = system(bufLaunch);
  PRINTF("n = %d\n", n);
#else
  PRINTF("lanching '%s'\n", bufLaunch);
  n = system(bufLaunch);
#endif
  PRINTF("n = %d\n", n);

  // Return system() call result (zero on success)
  return n;
}

#ifndef _UNIX

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
#else /* ifndef _UNIX */

int main()
{
#endif /* ifndef _UNIX */

   // Allocate the Automatic License Manager object
   AutoLm *lm = new AutoLm();
   if (lm)
   {
     time_t exp_date = 0;
     ui64 languages = 0, version_plat = 0;
     int licenseStatus;
     char vendorPassword[20 + 1];
     char buyHashId[44] = "";
     char purchaseUrl[244] = "";
     unsigned int nVendorPwdLength;

     // Reconfigure entity and product below to match Ecosystem
     const char* entityName = "Software Creator"; //From Immutable Ecosystem
     const char* product = "HomeCooking";
     ui64 entityId = 2; // From Immutable Ecosystem, static per application
     ui64 productId = 1; // From Immutable Ecosystem, static per application

     // Populate the Infura ID with your specific id
     const char* infuraId = INFURA_PROJECT_ID; // From https://infura.io

     // Populate the password by converting the string to bytes
     nVendorPwdLength = lm->AutoLmPwdStringToBytes("ThePassw\\0rd",
                                                   vendorPassword);

     // Initialize AutoLM object with entity, product, mode, password
     // and Infura id needed to verify activation on-chain
     lm->AutoLmInit(entityName, entityId, product, productId, 3,
                    vendorPassword, nVendorPwdLength, NULL, infuraId);

     // Loop to validate license activation (needed if we create an activation)
     for (;;)
     {
       switch (licenseStatus = lm->AutoLmValidateLicense(LICENSE_FILE,
                                 &exp_date, buyHashId, &languages, &version_plat))
       {
         // If valid, display the activation expiration
         case licenseValid:
             Validated = true;
             sprintf(BufValidate,
                     "Activation for %s expires on %s", product,
                     ctime(&exp_date));
           break;

         // If no license file exists, create a local license activation
         case noLicenseFile:
         {
           int created = lm->AutoLmCreateLicense(LICENSE_FILE);
           if (created >= 0)
             continue; // Call Validate again after creating
           else
             break;
         }

         // If no license not valid on-chain/expired, launch Dapp to purchase
         case blockchainExpiredLicense:

           // launch browser to purchase from Immutable
           launchPurchaseDialog(entityId, productId, buyHashId, purchaseUrl);
           break;
         default:
           break;
       }
       break;
     }

     // If license is not valid then exit the application
     if (licenseStatus != licenseValid)
     {
#ifndef _UNIX
       // Not activated, close the application
       DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
       puts(purchaseUrl);
       return FALSE;
#else /* ifndef _UNIX */
       puts("This application requires a valid activation on-chain.");
       if (licenseStatus == blockchainExpiredLicense)
       {
         puts("\nYour browser (Chrome) was opened to the Immutable Ecosystem");
         puts("license activation purchase page for this application, passing");
         puts("your unique activation identifier. Please Purchase the activation");
         puts("from the Ecosystem to unlock this application running on your PC.");
         puts(purchaseUrl);
       }
       return -1;
#endif /* ifndef _UNIX */
     }
     else
     {
         // activated
#ifndef _UNIX
         DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
#else /* ifndef _UNIX */
       puts("This application was purchased on the Immutable Ecosystem.");
       puts(BufValidate);
#endif /* ifndef _UNIX */
     }
   }

   // Return error if 'new' does not work, improper build?
   else
     return FALSE;

   // Application activation verified, unlock and return success
#ifndef _UNIX
   return TRUE;
#else
   puts("\n  TestApplication ready to use");
   return 0;
#endif
}

#ifndef _UNIX
//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        if (Validated == true)
        {
            //OutputDebugString(L"************");
            wchar_t wBuf[100];
            size_t n;
            mbstowcs_s(&n, wBuf, BufValidate, 100);
            SetDlgItemText(hDlg, IDC_STATIC_MSG_VALIDATED, wBuf);
            SetDlgItemText(hDlg, IDC_STATIC_MSG_VALIDATED_2,
              L"TestApplication for this PC was validated on the Immutable Ecosystem.");
        }
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
#endif /* ifndef _UNIX */
