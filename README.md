
# Building AutoLM and dependencies

Automatic License Manager build and installation instructions.

Now we need to build curl, but depending on your application
and what you need to link the AutoLM library with, choose
path 1 or 2 below. Path 1 is for Unix applications such as
Linux, MSYS2, MacOS and others. Path 2 is for applications
that are built with and must link the AutoLM library with
their Visual Studio creation.

## To install curl for Unix (Linux/MSYS2/MacOS)

Distribution dependent, but install libcurl development.
This method bypasses the need to install and build
curl so there is no need to run git submodules init and update.
Examples to install libcurl development are below.

Debian

Curl requires OpenSSL (or similar) development so be sure the OpenSSL
development environment is installed and available.

```
sudo apt-get install openssl-devel
sudo apt-get install libcurl-dev
```

MSYS2

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

If you OS does not support a libcurl development you will need to
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

From the Start search field, open the application
"Developer Command Prompt for VS 2019". Move into the
winbuild directory and configure with buildconf.bat
and then build with nmake.

```
buildconf.bat
cd winbuild
nmake /f Makefile.vc mode=static VC=19 ENABLE_WINSSL=yes
```
### Linux

To configure, make and install the default libcurl installation
use these commands in the curl directory (installed with
git submodule update).

```
./configure --with-ssl
make install <needed for curlcpp>
```
To configure, make and install the minimum libcurl installation
that works with AutoLM, use these commands.

```
./configure --with-ssl --disable-ftp --disable-file --disable-ldap --disable-ldaps --disable-rtsp --disable-proxy --disable-dict --disable-telnet --disable-tftp  --disable-pop3 --disable-imap --disable-smb --disable-smtp --disable-gopher --disable-manual --without-brotli --without-zlib --disable-progress-meter  --disable-dnsshuffle
make
make install <needed for curlcpp>
```

# Building AutoLM

Now that libcurl is built and available it is time to build
AutoLM. If using Visual Studio simply open the solution file
AutoLM.sln and rebuild the solution. For Unix systems the
Makefile.msys2 and Makefile.linux and Makefile.macos
can be used based on your OS.

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

# Using AutoLM within Applications

Coming Soon...