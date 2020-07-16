#
# Makefile --
#
#       AutoLM Makefile for MSYS/Linux environment.
#
#

##
## Installation paths: 
##
##
INSTROOT = /usr/lib

##
## Commands:
##
CC	= gcc
CPP	= gcc
CP	= cp
RM	= rm
AR	= ar
LINK = /bin/sh /usr/bin/libtool --mode=link c++

##
## Definitions:
##
# need -DCURL_STATICLIB for static build
EXTRAS = -ggdb -O0 -DWIN32 -D_MINGW -D_WINDOWS
#EXTRAS = -O3 -fexpensive-optimizations -DWIN32 

CPPFLAGS = -Wall $(EXTRAS)
CFLAGS = -Wall $(EXTRAS)
INCLUDES = -I"." -I"./base" \
           -I$(CURLCPPDIR)/include

## 
## Global Makefile settings
##

LIBAUTO = \
	base/sha1.o \
	base/md5.o \
	compid.o \
	autolm.o

# Replace -lcurl below with custom build
CURLLIB = -lcurl

# The name of library output file (static libautolm.a)
#AUTOLIB = ./libautolm.a
AUTOLIB = ./libautolm.so

# Static build
#LDFLAGS = -fPIC -static
LDFLAGS = 
LIBS = -lstdc++
# These may be needed for static build, depending on curl install
#LIBS = -lcurl -lssl -lcrypto -lbrotlidec -lbrotlicommon -lnghttp2 \
#       -lpsl -lidn2 -liconv -lstdc++ -lIphlpapi -lz -lunistring

##
## Implicit Targets
##
.c.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

.cpp.o:
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) -o $@ $<

##
## Targets
##

##debug:
##CFLAGS = $(CFLAGS) -ggdb
##		CPPFLAGS = $(CPPFLAGS) -ggdb

all:		libauto

# To create a static library change this below
#	$(CPP) $(CPPFLAGS) -static \

libauto: $(LIBAUTO)
	$(CPP) $(CPPFLAGS) -shared \
                -o $(AUTOLIB) \
				$(LIBAUTO) $(CURLLIB) \
				$(LDFLAGS) $(LIBS)

# For libcurl static, remaning shared
#				-Wl,--whole-archive \
#				$(LIBAUTO) $(CURLLIB) \
#				-Wl,--no-whole-archive \

clean:
	$(RM) *.o
	$(RM) ./base/*.o
	$(RM) libautolm.so

##
