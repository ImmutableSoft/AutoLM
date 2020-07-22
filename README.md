
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

###Debian

Curl requires OpenSSL (or similar) development so be sure the OpenSSL
development environment is installed and available.

```
sudo apt-get install openssl-devel
sudo apt-get install libcurl-dev
```

###MSYS2

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
cd curl
```

### Visual Studio 2019

From the Start (bottom left Windows button) search field,
open the application "Developer Command Prompt for VS 2019".
Move into the curl directory (cd AutoLM/curl)
within this Developer Command Prompt and configure the
repository by running the buildconf.bat command, and then
moving to the winbuild folder and building with nmake.

```
buildconf.bat
cd winbuild
nmake /f Makefile.vc mode=static VC=19 ENABLE_WINSSL=yes
```
### Linux

To configure, make and install the default libcurl installation,
use these commands in the curl directory (installed with
git submodule update).

```
./configure --with-ssl
make install
```
To configure, make and install the minimum libcurl installation
that works with AutoLM, use these commands.

```
./configure --with-ssl --disable-ftp --disable-file --disable-ldap --disable-ldaps --disable-rtsp --disable-proxy --disable-dict --disable-telnet --disable-tftp  --disable-pop3 --disable-imap --disable-smb --disable-smtp --disable-gopher --disable-manual --without-brotli --without-zlib --disable-progress-meter  --disable-dnsshuffle
make
make install
```

# Building AutoLM

Now that libcurl is built and its library is available,
it is time to build AutoLM.

## Building with Visual Studio

Open the solution file AutoLM.sln, select the build type (x86
or x64) and rebuild the solution.

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
  AutoLM autoLM = new AutoLm();
  if(autoLM)
  {
    const char* vendorName = "MyEntity"; //From Immutable Ecosystem
    ui64 vendorId = 3; // From Immutable Ecosystem
    char vendorIdStr[21];
    sprintf(vendorIdStr, "%llu", vendorId);
    const char* product = "MyApplication";
    ui64 productId = 0; // From Immutable Ecosystem
    char productIdStr[21];
    sprintf(productIdStr, "%llu", productId);

    char vendorPassword[20 + 1];
    unsigned int nVendorPwdLength = 10; // init vendor password length
    vendorPassword[0] = 'M';
    vendorPassword[1] = 'y';
    vendorPassword[2] = 'P';
    vendorPassword[3] = 'a';
    vendorPassword[4] = 's';
    vendorPassword[5] = 's';
    vendorPassword[6] = 'w';
    vendorPassword[7] = '\0'; // Example of escape character in password
    vendorPassword[8] = 'r';
    vendorPassword[9] = 'd';
    vendorPassword[10] = '\0';
    char buyHashId[44];
    const char* infuraId = INFURA_PROJECT_ID; // From https://infura.io
    time_t exp_date = 0;
	  ui64 resultingValue;

    autoLM->AutoLmInit(vendorName, vendorId, product, productId,
	                     3, vendorPassword, nVendorPwdLength, NULL,
                       infuraId);
    for (;;)
    {
      switch(easyLMStatus = autoLM->AutoLmValidateLicense(lic_file,
                                &exp_date, buyHashId, &resultingValue))
      {
        case licenseValid:
        ...
			  case noLicenseFile:
			  {
					  int created = autoLM->AutoLmCreateLicense(lic_file);
					  if (created >= 0)
					  {
						  result[0] = '\0';
						  continue; // Call Validate again after creating
					  }
					  else
					    sprintf(result, "%s\nBlockchain license creation failed %d", result, created);
			  }
        case blockchainExpiredLicense:
      			// launch browser to purchse from Immutable
            //launchBuyDialog(vendorIdStr, productIdStr, buyHashId);
      }
    }
    break;

```

The following command line tools are created when AutoLM
is built; 'compid', 'activate' and 'validate'. Only
'activate' and 'validate' are required. The 'compid' is
used by an application for troubleshooting or when a server
call is required to 'activate'. 'activate' creates a local
license (file) using the end users compid and the application
secret. The 'validate' call requires a previously created
local license (file) and uses libcurl to validate the license
on the Ethereum network.

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
with any application defined computer id that returns hex string format.
Use an application specific and unique method or chain together
user or system hardware information with the default to create a
more specific activation identifier. It is perfectly acceptable
that different applications create different identifiers, but
by default they will be identical as the requirement is that
they be unique per PC/OS. After compilation the computer id for
the compiled system can be retrieved with the compid binary.

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

# AutoLM Application Integration Notes

If found to not be valid on the Ecosystem, the application
should consider the installation unlicensed and report the
product identifier to the user and/or redirect the user
to a browser/tab into the Immutable Ecosystem to purchase
the activation. Once the user purchases a new activation
through the Immutable Ecosystem the check will return valid
and the application logic should unlock the application
features. If the user already has an activation they
can navigate to the Immutable Activations page and update
their activation identifier to the new value. Applications
with many features may wish to consider a bulk migration
feature.