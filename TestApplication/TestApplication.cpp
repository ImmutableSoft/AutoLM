// TestApplication.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "TestApplication.h"
#include "autolm.h"
#include <string.h>
#include <stdio.h>

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

int launchBuyDialog(char* vendorIdStr, char* productIdStr, char* buyUniqueId)
{
  char* binaryPathLinux = (char*)"/usr/bin/google-chrome";
  char* binaryPathWin = (char*)"chrome.exe";
  char bufLink[200];
  char bufLaunch[250];
  sprintf_s(bufLink, 200, "--app=https://ecosystem.immutablesoft.org/?func=activation&entity=%s&product=%s&identifier=%s&promo=1",
    vendorIdStr, productIdStr, buyUniqueId);
  printf("bufLink - '%s'\n", bufLink);

  sprintf_s(bufLaunch, 250, "start chrome.exe \"%s\"", bufLink);
  printf("bufLaunch - '%s'\n", bufLaunch);
  int n = 0;
#ifndef _WIN32
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

   AutoLm *lm = new AutoLm();
   if (lm)
   {
     const char* vendorName = "CreatorAuto1"; //From Immutable Ecosystem
     ui64 vendorId = 3; // From Immutable Ecosystem, static per application
     char vendorIdStr[21];
     sprintf_s(vendorIdStr, 20, "%llu", vendorId);
     const char* product = "GameProduct";
     ui64 productId = 1; // From Immutable Ecosystem, static per application
     char productIdStr[21];
     sprintf_s(productIdStr, 20, "%llu", productId);

     char vendorPassword[20 + 1];
     unsigned int nVendorPwdLength;
     nVendorPwdLength = lm->AutoLmPwdStringToBytes("MyPassw\\0rd",
       vendorPassword);

     char buyHashId[44] = "";
     const char* infuraId = INFURA_PROJECT_ID; // From https://infura.io
     time_t exp_date = 0;
     ui64 resultingValue;
     int licenseStatus;

     lm->AutoLmInit(vendorName, vendorId, product, productId, 3,
       vendorPassword, nVendorPwdLength, NULL, infuraId);
     for (;;)
     {
       switch (licenseStatus = lm->AutoLmValidateLicense(LICENSE_FILE,
         &exp_date, buyHashId, &resultingValue))
       {
       case licenseValid:
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
         launchBuyDialog(vendorIdStr, productIdStr, buyHashId);
         break;
       default:
         break;
       }
       break;
     }

     // If license is not valid then exit the application
     if (licenseStatus != licenseValid)
     {
       // Not activated, close the application
       DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
       return FALSE;
     }
   }

   return TRUE;
}

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
