<img src="./images/Immutable_BlueOnWhite_Logo.png" align="right" width="100" height="50"/>

# <img src="./images/AutoLm.png" alt="autolm" width="100" height="100"/> AutoLM Introduction

## Welcoming the future of digital distribution assets!

ImmutableSoft's goal is for all software creators to authenticate
their digital product releases and directly offer for sale
product activations for their digital creations. Purchasers of
software are ensured of the authenticity and that their payment
goes directly to the creator of the product they purchase.

The Automated License Manager, or AutoLM, is the embedded side
of our decentralized software sales and distribution. This
small C++ library and/or command line is used to automate
life-cycle management for any software or digital creation.

AutoLM connects your digital product with the smart contracts
of the [Immutable Ecosystem](https://ecosystem.immutablesoft.org).
Used together they secure supply chain and/or sales automation
processes. A complete life-cycle management solution with
automated processes.

# AutoLM Overview

AutoLM is an open source and commercial friendly (MIT license)
example of a secure software distribution and license activation
library that is compatible with the Immutable Ecosystem. The goals
of AutoLM are to be easy to use while following security best
practices.

AutoLM is built (see [INSTALL.md](./INSTALL.md)) as a
C++ library to be linked together with the software or digital
creation to be licensed/sold. For server side or other
programming language support, there is optionally a set of
command line tools available for use.

When integrated together with the digital
product that is to be distributed and/or sold, AutoLM will query
the immutable Ethereum database (Immutable Ecosystem smart contracts)
and verify that the end users digital download and/or purchased product
license activation is in fact valid.

To access the Ethereum database requires an Infura Product Id, available
for free from [Infura.io](https://infura.io/). Each Entity of the Immutable
Ecosystem should use their own Infura product Id to avoid
exceeding the daily limit of uses (currently 100,000 activation
checks per day is supported with a free Infura account).

# Quick Use Guide for Product Release Authentication (Distribution)

Digital product releases have the files' SHA256 checksum written to the
Ethereum blockchain and signed by the registered creators' Ethereum wallet
address key. An end user or application can easily verify a downloaded
digital file release by a reverse lookup using the SHA256 checksum
of the file. After downloading, the SHA256 hash of the file is used to
look up the product release information. If the information matches
the vendor, product and URI then the file is authentic.

It is important that the user, or the installed software itself,
verify that the lookup information is correct for the product
they downloaded and intend to install. The resulting lookup information
will include the product and owner (entity) along with the specific
release id, supported languages, version and download URI.

## Check File Authentication library libautolm

Below is the comment header for the EthereumAuthenticateFile() function
describing its usage. The SHA256 checksum hex string (of the file in
question) and your InfuraId, are passed as inputs to query the blockchain.
The entity, product and release Ids, languages and version flags, and URI
are outputs of this function on success. If the function returns a negative
value then an AutoLmResponse error occurred and the outputs are undefined
and should be ignored.

```cpp
/***********************************************************************/
/* EthereumAuthenticateFile: lookup file authenticity on blockchain    */
/*                                                                     */
/*      Inputs: hashId = the file SHA256 checksum/hash to lookup       */
/*              infuraId = the Infura ProductId to use for access      */
/*     Outputs: entityId = the Entity Id (creator id) of application   */
/*              productId = the product Id of the application          */
/*              releaseId = the product release index of file          */
/*              languages = the 64 bit language flags of file          */
/*              version = the version, 4 x 16 bits (X.X.X.X)           */
/*              uri = the URI string pointing to the release file      */
/*                                                                     */
/*     Returns: the zero on success, negative on error                 */
/*                                                                     */
/***********************************************************************/
int EthereumAuthenticateFile(const char* hashId,
  const char* infuraId, ui64* entityId, ui64* productId,
  ui64* releaseId, ui64* languages, ui64* version, char* uri)
```

See the Authenticate.cpp main() function to for a complete example that
opens a file, reads the contents and performs the SHA256 checksum before
checking the authentication of the file on the blockchain and displaying
the file details to the user. This library and example can also be used
to self authenticate an application such as a Win10 executable. Computing
the SHA256 of the executable after loaded into memory, and verifying
it with the blockchain ensures that the file is authentic before
execution.

# Quick Use Guide for License Activation Tokens (Purchases)

AutoLM License Activation Tokens activate an instance of installed
software on a particular hardware platform. Typically these tokens are
exchanged on the blockchain for ETH or other cryptocurrency of value
(stable coin, etc.) through a produce license offer on the Immutable
Ecosystem but they can also be directly created by the registered
digital creator for distribution to their customers.

The library to check software activations is designed to be automated
into a digital creators end user distribution flow in one of two ways;
standalone or server assisted. Standalone has the installed
software application create the initial local license activation
file. It is the simplest to deploy and is the basis for the
Quick Use Guide next. Server assisted moves the local license
file creation process to a secure server the creator controls,
through any server interface (JSON/REST interface, etc.). This
gives additional security as well as adding creator control to
the local license file distribution process. This server interface
can be tied into an end user registration requirement on the
software creators website, while still maintaining automation of
the process.

## Check Activation License with library libautolm

The security of AutoLM license activations works by utilizing a
globally unique, read only PC/OS identifier and cryptographically
tying it together with the unique entity and product information,
including a secret password from the software creator. This
one way cryptographic algorithm yields a unique activation identifier
that is then used to identify if the installed software is 'valid'
as a current digital activation asset, stored on the immutable
Ethereum database.

The first step to using AutoLM library for a activation license file
check library is to initialize it with the entity and product
information, as represented on the Immutable Ecosystem. If you have
not created an Entity on Immutable you can test with one of the product
examples on the Ropsten testnet (ie. leave code unchanged). With the
entity and product reference from Immutable, call AutoLmInit() with a
private mode and password to initialize the library. Remember, to access
the Ethereum database requires a valid Infura Product Id, available for
free from [Infura.io](https://infura.io/).

```cpp
/***********************************************************************/
/* AutoLmInit: Initialize AutoLM with entity/product credentials       */
/*                                                                     */
/*      Inputs: entity = full entity name (may change)                 */
/*              entityId =  the Immutable Entity Id                    */
/*              product =  full product name (may change)              */
/*              productId = the Immutable Entity specific Product Id   */
/*              mode = cryptographic algorithm to use for hash         */
/*              password =  password seeded into cryptographic hash    */
/*              pwdLength =  password length in bytes                  */
/*              computer_id =  optional function to generate comp id   */
/*              infuraId =  the Infura product Id assigned the creator */
/*                                                                     */
/*     Returns: 0 if success, otherwise an error occurred              */
/*                                                                     */
/***********************************************************************/
```

Once initialized, the AutoLm object can be used to validate
a license file or create a local license activation file.
For a standalone situation, AutoLmValidateLicense() can be
called to start and if it returns a noLicenseFound error,
then the application may create a local activation with
AutoLmCreateLicense() before calling AutoLmValidateLicense()
again.

```cpp
/***********************************************************************/
/* AutoLmCreateLicense: Create blockchain activation license file      */
/*                                                                     */
/*       Input: filename to write the blockchain license               */
/*                                                                     */
/*     Returns: 0 if success, otherwise error code                     */
/*                                                                     */
/***********************************************************************/
```

On the second call to AutoLmValidateLicense() the
Ethereum database will be checked and if a new install the
blockchainExpiredLicense error will be returned, indicating
that this activation is not purchased or has expired.

```cpp
/***********************************************************************/
/* AutoLmValidateLicense: Determine validity of a license file         */
/*                                                                     */
/*       Input: filename = full filename of license file (may change)  */
/*     Outputs: exp_date = the resulting expiration day/time           */
/*              buyHashId = resulting activation hash to purchase      */
/*              resultValue = resulting value of the activation        */
/*                                                                     */
/*     Returns: the immutable value of license, otherwise zero (0)     */
/*                                                                     */
/***********************************************************************/
```

At this point the application can choose to handle the situation
however it pleases. At a minimum the application should display
the activation identifier and a link to the Immutable Ecosystem.
For a 'one-click' purchase experience provide an embedded
link into the Immutable Ecosystem that will open the purchase
activation page for that product with the end users activation
identifier auto populated .

To integrate AutoLM into other payment options besides Immutable
there are two options, upgrade to EasyLM for a full license
management suite (contact ImmutableSoft for more information) or
have your sales process create activations within the Immutable
Ecosystem on behalf of your customers when they purchase.

Below is the standalone example to launch the Immutable Ecosystem
with a link to an auto populated page for a 'one-click' customer
purchase experience. The parameters include the entity and
product id's as well as the end users activation identifier -
from the installed local license file. The Chrome browser is
launched opening the product purchase page and the purchase page
URL is returned in purchaseUrl, which may be useful (by the user
and/or application) if the launch of the Chrome browser to open
the URL fails.

```cpp
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
```

To complete the standalone integration of AutoLM, below is the code
from the TestApplicaiton example included in Git repository. Note that
the AutoLmValidateLicense() function returns the result as well as the
expiration of the activation identifier as stored on the block chain. It
also returns the activation identifier needed to purchase or renew from
Immutable if the error blockchainExpiredLicense is returned. It is possible
to detect a renewal by checking if the returned expiration date is not zero.

Replace the INFURA_PROJECT_ID below in TestApplication.cpp with the one
you have created from Infura.io (see above for more information). For
evaluation purposes you may contact us and request a copy of one of our
Infura product Ids. For security purposes we cannot share this publicly,
and your application should not either!

```cpp
#define INFURA_PROJECT_ID "31dfbdd34717a744d19a29cddd623391" // Change this!
#define LICENSE_FILE      "./license.elm"

...

int main()
{
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
   }

   // Start application as it has a valid license
   ...
}
```

Be sure to link your application with AutoLm (-lautolm) as well
as any dependencies (curl, openssl, etc.). See the TestApplication
for an example for your build environment. More details on AutoLM is
described below when discussing the command line tools. In general,
the Activate command uses AutoLmInit() and AutoLmCreateLicense() within
the AutoLm library, while the Validate command uses AutoLmInit() and
AutoLmValidateLicense().

# Command Tools for Scripting Languages

Due to the insecurity of passing parameters within
a scripting language, and the fact that AutoLm requires
a password as said parameter, scripting language applications of
AutoLM should be limited to servers which the creator
controls. Otherwise a keystore should be deployed (AWS, etc.)
to store the password and this keystore be kept under control
of the software creator. Alternatively, and only if supported
by the scripting language, bytecode may be generated for the
portion of the application that is to perform the license
activation check.

## Overview of Command Line Tools

To aid testing, debugging and integration with scripting
languages (Python, Perl, Tcl, etc.), the following command
line tools are created when AutoLM is built; 'compid',
'authenticate', 'activate' and 'validate'. The 'compid' outputs
the computer id and is used by an application for troubleshooting
or during server assist when a server call is required to 'activate'
and the computer id must be passed from the PC executing the
software to the server creating the license file.

The 'authenticate' command computes a SHA256 checksum of a local
file and looks up the product release with this checksum. The
immutably recorded file details are returned in the response to
this lookup if the file checksum is valid. The file details
include entity (company/individual), product, version,
languages and the official download URI.

The 'activate' command creates a local license (file) using
the detected OS/PC computer id and the application details
(names, ids, mode, secret). The 'validate' call requires a previously
created local license (file) and uses libcurl to validate the
license both locally and on the Ethereum network. 'Validate' returns
a string representing the license activation value. Any number
greater than zero is active, any number greater than one (1)
is application specific (ie. an application feature or item).

# Authenticate - Secure Authentication of Release File

```bash
$ ./authenticate
Invalid number of arguments 1
authenticate <file name> <infura id>

  Authenticate a digital file with creator release.
    Returns version of release on success.

  <file name> The path to a local file to verify on chain
  <infura id> Your infura.io product id

$ ./authenticate ../../Downloads/Mibpeek-2020_2_2.exe d3dddc623391479a2931dfbd17a744d1
  File ../../Downloads/Mibpeek-2020_2_2.exe
  SHA256 checksum: dcde0b4b83b904d7e27ee7f501ae2a1faaecc6a78a7f41e0a756d2f82e002564
  File authentication found on blockchain
    Version 2020.2.2.0
    Release #7 for entity 5 and product 0.
    URI is https://www.asyn.io/download/mibpeek-2020_2_2.exe
```

As shown in the example above, the 'authenticate' command displays
the version, release, entity and product id, as well as the official
download URI link.

```bash
$ ./authenticate ../../Downloads/zadig-2.5.exe 6233914717a744d19a2931dfbdd3dddc
  File ../../Downloads/zadig-2.5.exe
  SHA256 checksum: 78a1a26854fbc848284588a62c7fbec9c652f6a3218ba543783d369265df00d6
error no entity/product
ERROR - Release not found for SHA256 checksum of this file
```

## CompId - Globally Unique and Immutable Identifier

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
is that the ID be unique per PC/OS, not application. The CompId
command line tool takes no parameters and outputs the PC's unique
global identifier.

```bash
$ ./compid
0x313fc746359696cb41a3a4adb663c6fb
```

## Activate - Local Activation Install

The second action of the library is creating local license
activations for a particular user/system and product.
This action requires a secret password and selected algorithm
from the software creator. The simplest approach is to compile
these details (password, mode) into the application. For
additional security the activation step can be hosted
through a REST/JSON interface on the software creators
website. This interface returns a created license in exchange
for the computer id (and possibly other information)
at runtime.

A local license activation is created locally, derived from the
application secret, product and computer id. However, this local
activation still requires registration on the global blockchain
before in can be considered valid. This blockchain activation
step can require payment to the software creator through the
transfer of crypto-currency (ETH), automating the sales process.

An example using the 'activate' command line tool to create a
license activation file is below.

```bash
$ ./activate
Invalid number of arguments 1
activate <entity name> <entity id> <app name> <app id>
         <mode> <password> [comp id] [file name]

  Create a locate product activation license file

  <entity name> is Immutable Ecosystem Entity name
  <entity id> is Immutable Ecosystem Entity Id
  <product name> is Immutable Ecosystem Product name
  <product id> is Immutable Ecosystem Product Id
  <mode> is authenticate mode, 2 is MD5, 3 is SHA1
  <password> string with escape characters ie. 'ThePassw\0rd'
  [comp id] Optional. Computer/Machine Id, or current OS/PC if missing
  [file name] Optional. Default is ./license.elm

$ ./activate Mibtonix 3 Mibpeek 0 3 Passw\\0rd 0x5adb663c6fbb41a3a43fc74319696c63 ./license.elm
```

## Validate - Secure Blockchain Validation of Local Activation

The third action of the library validates a local activation
license by ensuring the computer id matches the executing PC,
and the product and license hash matches when computed with the
secret password. If a local license file is copied to a different
PC then the computer id will not match and the license is considered
invalid. Validate, after ensuring the validity of the
local license file using the current PC computer id and secret password,
then securely queries (HTTPS) the Ethereum database and queries the
activation value.

Only an activation value greater than zero is considered valid and
any value besides one is considered a product feature. <b>The value
one is reserved for digital product activation purposes.</b> Different
application features can be available for purchase from the Immutable
Ecosystem and are distinguished by their Activation Value. At no time
after purchase can this Value be changed.

An example using the 'validate' command line tool to verify a
license activation file with the Ethereum database is below. Note
that the Infura Product Id (d3dddc6...1) is not a valid Infura Id.
Please register you or your organization with [Infura](https://infura.io) to
receive your own unique product identifier that should be used within your
application for product validations.

```bash
$ ./validate
Invalid number of arguments 1
validate <entity name> <entity id> <app name> <app id>
         <mode> <password> <file name>

  Validate a locate product activation license file

  <entity name> is Immutable Ecosystem Entity name
  <entity id> is Immutable Ecosystem Entity Id
  <product name> is Immutable Ecosystem Product name
  <product id> is Immutable Ecosystem Product Id
  <mode> is authenticate mode, 2 is MD5, 3 is SHA1
  <password> password string, supports escape characters ie. 'Passw\0rd'
  <infura id> the product ID from Infura.io
  [file name] Optional. Default is ./license.elm

$ ./validate Mibtonix 3 Mibpeek 0 3 Passw\\0rd d3dddc623391479a2931dfbd17a744d1 ./license2.elm
1
```

Note the value above returns one (1) indicating the license is valid. If
the 'validate' command returns zero (0) then the activation is not
valid on the Ethereum database indicating a purchase (or activation
Move) from the Immutable Ecosystem is required.

# AutoLM Application Integration Notes

If an activation is found to not be valid on the Ecosystem, the
application should consider the installation unlicensed and
report the product identifier to the user and/or redirect the
user to a browser/tab into the Immutable Ecosystem to purchase
the activation (see launchBuyDialog() above). Once the user
purchases a new activation through the Immutable Ecosystem
the check will return valid and the application logic should
unlock the application feature(s). If the user already has an
activation they can navigate to the Immutable Activations page
and update their activation identifier to the new value.
Applications with many features may wish to consider a bulk
migration feature.

Note that an activation has an expiration date stored on the
blockchain and may be renewed/extended before it expires. It may
be desirable for an application to report this expiration date to
the user, and a link to renew, before and/or after the activation
expires.

<img src="./images/Immutable_BlueOnWhite_Logo.png" align="right" width="100" height="50"/>
