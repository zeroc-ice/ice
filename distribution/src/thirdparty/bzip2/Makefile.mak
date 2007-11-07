# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

all: debuglib releaselib bzip2

bzip2: releaselib
	$(CC) $(TOOL_CFLAGS) -o bzip2 bzip2.c libbz2.lib setargv.obj
	$(CC) $(TOOL_CFLAGS) -o bzip2recover bzip2recover.c

debuglib: $(OBJS:DIR=.\Debug)
	link  /dll /implib:libbz2d.lib /out:bzip2d.dll $**

releaselib: $(OBJS:DIR=.\Release)
	link  /dll /release /implib:libbz2.lib /out:bzip2.dll $**

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
	del libbz2*.lib 
	del bzip2.exe
	del bzip2recover.exe
