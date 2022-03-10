
<img src="./images/Immutable_BlueOnWhite_Logo.png" align="right" width="100" height="50"/>

# <img src="./images/AutoLm.png" alt="autolm" width="100" height="100"/> Building AutoLM and dependencies

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

AutoLM depends on curl as a submodule. If your OS does not support
libcurl development perform the commands below to clone (download) the
latest curl code into your build tree.

```
git submodule init
git submodule update
```

### Linux/MSYS2/MACOS

To configure, make and install the default libcurl installation,
use these commands in the curl directory (installed with
git submodule update).


```
cd curl
./configure --with-ssl
make install
```

<b>Alternatively</b>, to configure, make and install <b>the minimum</b>
libcurl installation that works with AutoLM, use these commands.

```
cd curl
./configure --with-ssl --disable-ftp --disable-file --disable-ldap --disable-ldaps --disable-rtsp --disable-proxy --disable-dict --disable-telnet --disable-tftp  --disable-pop3 --disable-imap --disable-smb --disable-smtp --disable-gopher --disable-manual --without-brotli --without-zlib --disable-progress-meter  --disable-dnsshuffle
make
make install
```

This completes building and installing curl and libcurl for linking with
AutoLM for Linux/MSYS2/MACOS applications. Unless also building libraries
for Visual Studio applications you can skip the next section.

## Visual Studio 2019

To build libcurl for VS and both x86 and x64 platforms, please
complete all sections below. If you only need support for
one platform then only that platform needs to be completed.
If you do not complete all sections below, some build
configurations for the AutoLM VS solution may not link
correctly.

### Configure the cloned curl repository

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

### x86

From the Start (bottom left Windows button) search field,
open the application "Developer Command Prompt for VS 2019"
and then move to the curl/winbuild folder. Build curl with
with the following nmake command.

```
cd winbuild
nmake /f Makefile.vc mode=static VC=19 ENABLE_WINSSL=yes
```

This will create the static libcurl_a.lib in the folder
curl\builds\libcurl-vc19-x86-release-static-ipv6-sspi-winssl\lib

### x64

From the Start (bottom left Windows button) search field,
open the application "x64 Native Tools Command Prompt for VS 2019"
and then move to the winbuild folder and building with nmake.

```
cd winbuild
nmake /f Makefile.vc mode=static VC=19 ENABLE_WINSSL=yes
```

This will create the static libcurl_a.lib in the folder
curl\builds\libcurl-vc19-x64-release-static-ipv6-sspi-winssl\lib

# Building AutoLM

Now that libcurl is built and its library is available,
it is time to build AutoLM.

## Building with Visual Studio

Open the solution file AutoLM.sln, select the build type (x86
or x64) and configuration (Debug or Release) and rebuild the solution.

## Building for Unix

For Unix systems the Makefile.msys2 and Makefile.linux and Makefile.macos
can be used based on your OS. If the example does not link with a
particular OS, please be sure to check the libraries of the OS match
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

# Using AutoLM

See the [README.md](./README.md) file for using AutoLM.

<img src="./images/Immutable_BlueOnWhite_Logo.png" align="right" width="100" height="50"/>
