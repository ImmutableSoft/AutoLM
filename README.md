# AutoLM
Automatic License Manager

To build curl for VS 2019
1. Open Developer Command Prompt for VS 2019

cd curl/winbuild
nmake /f Makefile.vc mode=static VC=19 ENABLE_WINSSL=yes
