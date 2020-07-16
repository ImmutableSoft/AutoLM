
# Building AutoLM dependencies

Automatic License Manager build and installation instructions.

AutoLM depends on curl as a submodule. Perform the commands below
to clone the latest curl into your build tree.

```
git submodule init
git submodule update
```

Now we need to build curl, but depending on your application
and what you need to link the AutoLM library with, choose
path 1 or 2 below. Path 1 is for Unix applications such as
Linux, MSYS2, MacOS and others. Path 2 is for applications
that are built with and must link the AutoLM library with
their Visual Studio creation.

## To build curl for Unix (Linux/MSYS2/MacOS)

Distribution dependent, but install libcurl development.

Debian

```
sudo apt-get install libcurl-dev
```

MSYS2

```
pacman -S libcurl-devel
```

## To build curl for VS 2019

Open Developer Command Prompt for VS 2019
and move into the winbuild directory and build with nmake.

```
cd curl/winbuild
nmake /f Makefile.vc mode=static VC=19 ENABLE_WINSSL=yes
```

# Building AutoLM

Now that libcurl is built and available it is time to build
AutoLM. If using Visual Studio simply open the solution file
AutoLM.sln and rebuild the solution. For Unix systems the
Makefile.msys2 and Makefile.linux and Makefile.macos
can be used based on your OS.

```
make -f Makefile.msys2
```
