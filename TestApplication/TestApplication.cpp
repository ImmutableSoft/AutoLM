// TestApplication.cpp : Defines the entry point for the application.
//
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "autolm.h"
#include "windows.h"
#ifndef _UNIX
#include "framework.h"
#include "TestApplication.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

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
    LoadStringW(hInstance, IDC_TESTAPPLICATION, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TESTAPPLICATION));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TESTAPPLICATION));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TESTAPPLICATION);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}
#endif /* _UNIX */

int launchBuyDialog(char* vendorIdStr, char* productIdStr, char* buyUniqueId)
{
  char bufLink[200];
  char bufLaunch[250];
  sprintf(bufLink, "--app=https://ecosystem.immutablesoft.org/?func=activation&entity=%s&product=%s&identifier=%s&promo=1",
    vendorIdStr, productIdStr, buyUniqueId);
  printf("bufLink - '%s'\n", bufLink);

  sprintf(bufLaunch, "start chrome.exe \"%s\"", bufLink);
  printf("bufLaunch - '%s'\n", bufLaunch);
  int n = 0;
#ifndef _WINDOWS
  sprintf(bufLaunch, "/usr/bin/google-chrome \"%s\"", bufLink);
  printf("lanching '%s'\n", bufLaunch);
  n = system(bufLaunch);
  printf("n = %d\n", n);
#else
  printf("lanching '%s'\n", bufLaunch);
  n = system(bufLaunch);
#endif
  printf("n = %d\n", n);

  return n;
}

#define INFURA_PROJECT_ID "6233914717a744d19a2931dfbdd3dddc"
#define LICENSE_FILE      "./license.elm"

// Global variables
static char s_bufValidate[100];
static bool s_bValidated = false;

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
   AutoLm *lm = new AutoLm();
   if (lm)
   {
     // Reconfigure entity and product below to match Ecosystem
     const char* entityName = "CreatorAuto1"; //From Immutable Ecosystem
     ui64 entityId = 3; // From Immutable Ecosystem, static per application
     char entityIdStr[21];

     sprintf(entityIdStr, "%llu", entityId);
     const char* product = "GameProduct";
     ui64 productId = 1; // From Immutable Ecosystem, static per application
     char productIdStr[21];
     sprintf(productIdStr, "%llu", productId);

     char vendorPassword[20 + 1];
     unsigned int nVendorPwdLength;
     nVendorPwdLength = lm->AutoLmPwdStringToBytes("MyPassw\\0rd",
       vendorPassword);

     char buyHashId[44] = "";
     const char* infuraId = INFURA_PROJECT_ID; // From https://infura.io
     time_t exp_date = 0;
     ui64 resultingValue;
     int licenseStatus;

     lm->AutoLmInit(entityName, entityId, product, productId, 3,
       vendorPassword, nVendorPwdLength, NULL, infuraId);
     for (;;)
     {
       switch (licenseStatus = lm->AutoLmValidateLicense(LICENSE_FILE,
         &exp_date, buyHashId, &resultingValue))
       {
       case licenseValid:
           s_bValidated = true;
           sprintf(s_bufValidate, "Activation for GameProduct expires on %s", ctime(&exp_date));
           printf("%s\n", s_bufValidate);
         break;
       case noLicenseFile:
       {
         int created = lm->AutoLmCreateLicense(LICENSE_FILE);
         if (created >= 0)
           continue; // Call Validate again after creating
         else
           break;
       }
       case blockchainExpiredLicense:
         // launch browser to purchase from Immutable
         launchBuyDialog(entityIdStr, productIdStr, buyHashId);
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
       return FALSE;
#else /* ifndef _UNIX */
       puts("This application requires an activation validated on the Immutable Ecosystem.");
       puts("\nYour browser (Chrome) was opened to the Immutable Ecosystem");
       puts("license activation purchase page for this application, passing");
       puts("your unique activation identifier. Please Purchase the activation");
       puts("from the Ecosystem to unlock this application running on your PC.");
       return -1;
#endif /* ifndef _UNIX */
     }
     else
     {
         // activated
#ifndef _UNIX
         DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
#else /* ifndef _UNIX */
       puts("TestApplication for this PC was validated on the Immutable Ecosystem.");
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
        if (s_bValidated == true)
        {
            //OutputDebugString(L"************");
            wchar_t wBuf[100];
            size_t n;
            mbstowcs_s(&n, wBuf, s_bufValidate, 100);
            SetDlgItemText(hDlg, IDC_STATIC_MSG_VALIDATED, wBuf);
            SetDlgItemText(hDlg, IDC_STATIC_MSG_VALIDATED_2, L"This application activation was validated on the Immutable Ecosystem");
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
