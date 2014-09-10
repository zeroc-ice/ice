# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir		= ..\..\..
LIBNAME		= $(SDK_LIBRARY_PATH)\icegrid.lib
TARGETS		= $(LIBNAME)
SOURCE_DIR	= ..

TARGETS		= $(LIBNAME)

SLICE_SRCS	= ..\Admin.cpp \
		  ..\Descriptor.cpp \
		  ..\Exception.cpp \
		  ..\FileParser.cpp \
		  ..\Locator.cpp \
		  ..\Observer.cpp \
		  ..\Query.cpp \
		  ..\Registry.cpp \
		  ..\Session.cpp \
		  ..\UserAccountMapper.cpp

OBJS		= $(ARCH)\$(CONFIG)\Admin.obj \
		  $(ARCH)\$(CONFIG)\Descriptor.obj \
		  $(ARCH)\$(CONFIG)\Exception.obj \
		  $(ARCH)\$(CONFIG)\FileParser.obj \
		  $(ARCH)\$(CONFIG)\Locator.obj \
		  $(ARCH)\$(CONFIG)\Observer.obj \
		  $(ARCH)\$(CONFIG)\Query.obj \
		  $(ARCH)\$(CONFIG)\Registry.obj \
		  $(ARCH)\$(CONFIG)\Session.obj \
		  $(ARCH)\$(CONFIG)\UserAccountMapper.obj

SRCS		= $(OBJS:.obj=.cpp)
SRCS		= $(SRCS:x86\=)
SRCS		= $(SRCS:x64\=)
SRCS		= $(SRCS:arm\=)
SRCS		= $(SRCS:Retail\=..\)
SRCS		= $(SRCS:Debug\=..\)

HDIR		= $(headerdir)\IceGrid
SDIR		= $(slicedir)\IceGrid

PDBNAME		= $(LIBNAME:.lib=.pdb)
CPPFLAGS        = /Fd$(PDBNAME) -I..\.. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
SLICE2CPPFLAGS	= --checksum --ice --include-dir IceGrid --dll-export ICE_GRID_API

!include $(top_srcdir)\config\Make.rules.mak

$(LIBNAME): $(OBJS) sdks
	$(AR) $(ARFLAGS) $(OBJS) /out:$(LIBNAME)

clean::
	-del /q $(SOURCE_DIR)\Admin.cpp $(HDIR)\Admin.h
	-del /q $(SOURCE_DIR)\Descriptor.cpp $(HDIR)\Descriptor.h
	-del /q $(SOURCE_DIR)\Exception.cpp $(HDIR)\Exception.h
	-del /q $(SOURCE_DIR)\FileParser.cpp $(HDIR)\FileParser.h
	-del /q $(SOURCE_DIR)\Locator.cpp $(HDIR)\Locator.h
	-del /q $(SOURCE_DIR)\Observer.cpp $(HDIR)\Observer.h
	-del /q $(SOURCE_DIR)\Query.cpp $(HDIR)\Query.h
	-del /q $(SOURCE_DIR)\Registry.cpp $(HDIR)\Registry.h
	-del /q $(SOURCE_DIR)\Session.cpp $(HDIR)\Session.h
	-del /q $(SOURCE_DIR)\UserAccountMapper.cpp $(HDIR)\UserAccountMapper.h
	-del /q $(ARCH)\$(CONFIG)\*.obj
	-del /q $(PDBNAME)

install:: all
