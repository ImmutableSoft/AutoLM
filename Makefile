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
#CURLCPPDIR = ../easylm/curlcpp
CURLCPPDIR = ../curlpp

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
EXTRAS = -ggdb -O0 -D_MINGW -DWIN32 \
         -DCURL_STATICLIB -DNGHTTP2_STATICLIB -D_OPENSSL
#EXTRAS = -O3 -fexpensive-optimizations -D_MINGW -DWIN32 \
         -DCURL_STATICLIB -DNGHTTP2_STATICLIB -D_OPENSSL
CPPFLAGS = -Wall -fPIC $(EXTRAS)
CFLAGS = -Wall -fPIC $(EXTRAS)
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

CURLLIB = $(LIBCURLDIR)/lib/.libs/libcurl.a
CURLCPPLIB = $(CURLCPPDIR)/build/libcurlpp.a
AUTOLIB = ./libautolm.so

LDFLAGS = -fPIC -static
LIBS = -lcurl -lssl -lcrypto -lbrotlidec -lbrotlicommon -lnghttp2 \
       -lpsl -lidn2 -liconv -lstdc++ -lz -lunistring


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

libauto: $(LIBAUTO)
	$(CPP) $(CPPFLAGS) -shared \
                -o $(AUTOLIB) \
				-Wl,--whole-archive \
				$(LIBAUTO) \
				$(CURLCPPLIB) \
				-Wl,--no-whole-archive \
				-fPIC -shared \
				$(LIBS)

clean:
	$(RM) *.o
	$(RM) ./base/*.o
	$(RM) libautolm.so

##
