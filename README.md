
# Building AutoLM and dependencies

Automatic License Manager build and installation instructions.

AutoLM requires libcurl. Depending on your OS, application
and what you need to link the AutoLM library with, choose
path 1 or 2 below. Path 1 is for Unix applications such as
Linux, MSYS2, MacOS and others. Path 2 is for applications
that are built with Windows and Visual Studio.

## To install curl for Unix (Linux/MSYS2/MacOS)

Distribution dependent, but install libcurl development.
This method bypasses the need to install and build
curl so there is no need to run git submodules init and update.
Examples to install libcurl development are below.

### Debian

Curl requires OpenSSL (or similar) development so be sure the OpenSSL
development environment is installed and available.

```
sudo apt-get install openssl-devel
sudo apt-get install libcurl-dev
```

### MSYS2

Curl on MSYS2, especially when using SSL, has many dependencies. Note
that if you choose the minimum build below, not all of these are needed.

```
pacman -S openssl
pacman -S openssl-devel
pacman -S brotli-devel
pacman -S zlib-devel
pacman -S ca-certificates
pacman -S libcurl-devel
```

If your OS does not support a libcurl development you will need to
download, build and install it as described in the next section.

## To download and build libcurl

AutoLM depends on curl as a submodule. Perform the commands below
to clone (download) the latest curl code into your build tree.

```
git submodule init
git submodule update
```

### Visual Studio 2019

To build libcurl for both x86 and x64 platforms, please
complete both sections below. If you only need support for
one platform then only that section needs to be completed.
If you do not complete both sections below then not all
build configurations for AutoLM solution will link correctly.

First, configure the build environment after cloning (not
required if installing release zip, only if a fresh clone).
From the Start (bottom left Windows button) search field,
open the application "Developer Command Prompt for VS 2019".
Move into the curl directory (cd AutoLM/curl)
within this Developer Command Prompt and configure the
repository by running the buildconf.bat command.

```
cd curl
buildconf.bat
```

## x86

From the Start (bottom left Windows button) search field,
open the application "Developer Command Prompt for VS 2019"
and then move to the winbuild folder and building with nmake.

```
cd winbuild
nmake /f Makefile.vc mode=static VC=19 ENABLE_WINSSL=yes
```

This will create libcurl_a.lib in the folder
curl\builds\libcurl-vc19-x86-release-static-ipv6-sspi-winssl\lib

## x64

From the Start (bottom left Windows button) search field,
open the application "x64 Native Tools Command Prompt for VS 2019"
and then move to the winbuild folder and building with nmake.

```
cd winbuild
nmake /f Makefile.vc mode=static VC=19 ENABLE_WINSSL=yes
```

This will create libcurl_a.lib in the folder
curl\builds\libcurl-vc19-x64-release-static-ipv6-sspi-winssl\lib

### Linux

To configure, make and install the default libcurl installation,
use these commands in the curl directory (installed with
git submodule update).


```
cd curl
./configure --with-ssl
make install
```

<b>Alternatively</b>, to configure, make and install the minimum
libcurl installation that works with AutoLM, use these commands.

```
cd curl
./configure --with-ssl --disable-ftp --disable-file --disable-ldap --disable-ldaps --disable-rtsp --disable-proxy --disable-dict --disable-telnet --disable-tftp  --disable-pop3 --disable-imap --disable-smb --disable-smtp --disable-gopher --disable-manual --without-brotli --without-zlib --disable-progress-meter  --disable-dnsshuffle
make
make install
```

# Building AutoLM

Now that libcurl is built and its library is available,
it is time to build AutoLM.

## Building with Visual Studio

Open the solution file AutoLM.sln, select the build type (x86
or x64) and configuration (Debug or Release) and rebuild the solution.

## Building for Unix

For Unix systems the Makefile.msys2 and Makefile.linux and Makefile.macos
can be used based on your OS. If the example does not link with a
particular OS, please be sure to check the libraries for your OS match
those in the Makefile. Please submit a PR if desired for new Makefiles.

Windows MSYS2 and MinGW
```
make -f Makefile.msys2 clean
make -f Makefile.msys2
```

Linux and GCC
```
make -f Makefile.linux clean
make -f Makefile.linux
```

MacOS and GCC
```
make -f Makefile.macos clean
make -f Makefile.macos
```

# Using AutoLM Overview

The Automated License Manager, or AutoLM, empowers software
creators with sales distribution and automation. Utilizing
the Immutable Ecosystem (ie. Immutable), AutoLM automates the
sales and license activation and distribution processes.

AutoLM is an open source and commercial friendly (MIT license)
example of a secure license activation library that is
compatible with Immutable. The goals of AutoLM are to
be easy to use while following security best practices.

When built/linked/integrated together with the digital
product that is to be licensed, this library can perform
three actions. These actions are also available as command
line tools for testing and integration with non-compiled software
languages.

# Quick Use Guide

## Using C/C++

First initialize entity name, product, mode and password by
calling AutoLMInit(), then AutoLmValidateLicense(). If
noLicenseFound error, create a local activation with
AutoLmCreateLicense(). Then call AutoLmValidateLicense()
again.

```
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
      "https://ecosystem.immutablesoft.org/?func=activation&entity=%s&product=%s&identifier=%s&promo=0",
      entityIdStr, productIdStr, activationId);
  }

  // Create the URL with -app parameter for passing to the Chrome browser
  sprintf(bufLink,
    "--app=https://ecosystem.immutablesoft.org/?func=activation&entity=%s&product=%s&identifier=%s&promo=0",
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

...
int main()
{
  ...

   // Allocate the Automatic License Manager object
   AutoLm *lm = new AutoLm();
   if (lm)
   {
     time_t exp_date = 0;
     ui64 resultingValue;
     int licenseStatus;
     char vendorPassword[20 + 1];
     char buyHashId[44] = "";
     char purchaseUrl[244] = "";
     unsigned int nVendorPwdLength;

     // Reconfigure entity and product below to match Ecosystem
     const char* entityName = "CreatorAuto1"; //From Immutable Ecosystem
     const char* product = "GameProduct";
     ui64 entityId = 3; // From Immutable Ecosystem, static per application
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
                                 &exp_date, buyHashId, &resultingValue))
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
     }
  ...
}
```

To aid integration with scripting languages (Python, Perl, Tcl,
etc.), the following command line tools are created when AutoLM
is built; 'compid', 'activate' and 'validate'. The 'compid' is
used by an application for troubleshooting or when a server
call is required to 'activate' as the computer id must be
passed from the PC executing the software to the server creating
the license file. The 'activate' command creates a local
license (file) using the detected OS/PC computer id and the
application details (names, ids, secret). The 'validate' call
requires a previously created local license (file) and uses
libcurl to validate the license on the Ethereum network. Validate
returns a string representing the license activation value. Any
number greater than zero is active, any number greater than one (1)
is application specific (ie. an application feature or item).

# AutoLM Features

## Globally Unique and Immutable Identifier

The foundation of AutoLM is the globally unique and immutable
identifier (computer id). From the physical hardware executing
the library, this identifier will never change for a physical
computer without a reinstall of the OS. This identifier also
cannot be easily faked or changed by the user (immutable). This
global unique and immutable OS identifier is used to create
system specific software license activations.

To encourage customization of this step, AutoLM supports integration
with any application defined computer id that returns a hex string
format equal to or less than 35 bytes long (32 byte value, 2 for
hex prefix '0x' and one for terminating character).

Using an application specific and unique computer id algorithm,
or chaining together user or additional system hardware information
with the default to create a more specific activation identifier.
It is perfectly acceptable that different applications create different
identifiers, but by default they will be identical as the requirement
is that the ID be unique per PC/OS, not application. After compilation
the computer id for the OS/PC can be retrieved with the CompId binary.

```
$ ./compid
0x313fc746359696cb41a3a4adb663c6fb
```

## Secure Unique Activation Install 

The second action of the library is creating local license
activations for a particular user/system and product.
This action requires a secret password and selected algorithm
from the software creator. The simplest approach is to compile
these details (password, mode) into the application. For
additional security this second library can be hosted
through a REST/JSON interface on the software creators
website. This interface returns a created license in exchange
for the computer id (and possibly other information)
at runtime.

A local license activation is created locally, derived from the
application secret, product and computer id. However, this local
activation still requires registration on the global blockchain
before in can be considered globally active. This blockchain
activation step can thus require payment to the software creator
through the transfer of crypto-currency (ETH).

An example using the 'activate' command line tool to create a
license activation file is below.

```
./activate Mibtonix 3 Mibpeek 0 3 Passw\\0rd 0x5adb663c6fbb41a3a43fc74319696c63 ./license.elm
```

## Secure Activation Validation

The third action of the library validates a local activation
license (ensures computer id and hash match) and then
securely queries (HTTPS) the Ethereum network and verifies
the activation value. Only an activation value greater
than zero is considered valid and any value besides one
is considered a product feature. <b>The value one is reserved
for digital product activation purposes.</b> Different application
features can be available for purchase from the Immutable
Ecosystem and are distinguished by their Activation
Value. At no time after purchase can this Value be changed.

An example using the 'validate' command line tool to verify a
license activation file with the Ethereum database is below.

```
./validate Mibtonix 3 Mibpeek 0 3 Passw\\0rd d3dddc623391479a2931dfbd17a744d1 ./license2.elm
1
```

Note the value above returns one (1) indicating the license is valid. If
the 'validate' command returns zero (0) then the activation is not
valid on the Ethereum database indicating a purchase (or activation
Move) from the Immutable Ecosystem is required.

# AutoLM Application Integration Notes

If found to not be valid on the Ecosystem, the application
should consider the installation unlicensed and report the
product identifier to the user and/or redirect the user
to a browser/tab into the Immutable Ecosystem to purchase
the activation (see launchBuyDialog() above). Once the user
purchases a new activation through the Immutable Ecosystem
the check will return valid and the application logic should
unlock the application feature(s). If the user already has an
activation they can navigate to the Immutable Activations page
and update their activation identifier to the new value.
Applications with many features may wish to consider a bulk
migration feature.