# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# This is a replacement for the makefile.msc file that comes with the
# bzip2 source distribution. The key differences are that this
# makefile performs both release and debug builds and makes DLLs
# instead of static libraries.
#

CC=cl

DEBUG_CFLAGS  = /c /DWIN32 /Fo.\Debug\ /MDd /Zi /Od /D_FILE_OFFSET_BITS=64 /DBZ_EXPORT /nologo
REL_CFLAGS    = /c /DWIN32 /Fo.\Release\ /MD /Ox /D_FILE_OFFSET_BITS=64 /DBZ_EXPORT /nologo

TOOL_CFLAGS   = /DWIN32 /MD /Od /D_FILE_OFFSET_BITS=64 /nologo

FILES 	      = DIR\blocksort.c  \
      		DIR\huffman.c    \
      		DIR\crctable.c   \
      		DIR\randtable.c  \
      		DIR\compress.c   \
      		DIR\decompress.c \
      		DIR\bzlib.c

SRCS	= $(FILES:DIR=.)
OBJS	= $(FILES:.c=.obj)

!if "$(CPP_COMPILER)" == "VC90"
DLLSUFFIX	= _vc90
!endif

all: debuglib releaselib bzip2

bzip2: releaselib
	$(CC) $(TOOL_CFLAGS) /Febzip2 bzip2.c libbz2.lib setargv.obj
	@if exist bzip2.exe.manifest echo ^ ^ ^ Embedding manifest using mt.exe && \
	    mt.exe -nologo -manifest bzip2.exe.manifest -outputresource:bzip2.exe;#1 && del /q bzip2.exe.manifest
	$(CC) $(TOOL_CFLAGS) /Febzip2recover bzip2recover.c
	@if exist bzip2recover.exe.manifest echo ^ ^ ^ Embedding manifest using mt.exe && \
	    mt.exe -nologo -manifest bzip2recover.exe.manifest -outputresource:bzip2recover.exe;#1 && \
	    del /q bzip2recover.exe.manifest

debuglib: $(OBJS:DIR=.\Debug)
	link  /dll /implib:libbz2d.lib /out:bzip2$(DLLSUFFIX)d.dll $**
	@if exist bzip2$(DLLSUFFIX)d.dll.manifest echo ^ ^ ^ Embedding manifest using mt.exe && \
	    mt.exe -nologo -manifest bzip2$(DLLSUFFIX)d.dll.manifest -outputresource:bzip2$(DLLSUFFIX)d.dll;#2 && \
	    del /q bzip2$(DLLSUFFIX)d.dll.manifest

releaselib: $(OBJS:DIR=.\Release)
	link  /dll /release /implib:libbz2.lib /out:bzip2$(DLLSUFFIX).dll $**
	@if exist bzip2$(DLLSUFFIX).dll.manifest echo ^ ^ ^ Embedding manifest using mt.exe && \
	    mt.exe -nologo -manifest bzip2$(DLLSUFFIX).dll.manifest -outputresource:bzip2$(DLLSUFFIX).dll;#2 && \
	    del /q bzip2$(DLLSUFFIX).dll.manifest

$(OBJS:DIR=.\Debug): $(SRCS)
	if not exist .\Debug mkdir .\Debug
	$(CC) $(DEBUG_CFLAGS) $?

$(OBJS:DIR=.\Release): $(SRCS)
	if not exist .\Release mkdir .\Release
	$(CC) $(REL_CFLAGS) $?

clean: 
	del *.obj
	del .\Release\*.obj
	del .\Debug\*.obj
	del bzip2*.dll
	del libbz*.lib
	del bzip2.exe
	del bzip2recover.exe
