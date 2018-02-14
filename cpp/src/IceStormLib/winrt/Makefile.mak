# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir		= ..\..\..
LIBNAME		= $(SDK_LIBRARY_PATH)\icestorm.lib
TARGETS		= $(LIBNAME)
SOURCE_DIR	= ..

TARGETS         = $(LIBNAME)

OBJS		= $(ARCH)\$(CONFIG)\IceStorm.obj \
		  $(ARCH)\$(CONFIG)\Metrics.obj

SLICE_SRCS	= IceStorm\IceStorm.ice \
		  IceStorm\Metrics.ice

SRCS		= $(OBJS:.obj=.cpp)
SRCS		= $(SRCS:x86\=)
SRCS		= $(SRCS:x64\=)
SRCS		= $(SRCS:arm\=)
SRCS		= $(SRCS:Retail\=..\)
SRCS		= $(SRCS:Debug\=..\)

HDIR		= $(headerdir)\IceStorm
SDIR		= $(slicedir)\IceStorm

PDBNAME		= $(LIBNAME:.lib=.pdb)
CPPFLAGS	= /Fd$(PDBNAME) -I..\.. -DICE_BUILDING_ICESTORM -DWIN32_LEAN_AND_MEAN
ICECPPFLAGS	= -I..
SLICE2CPPFLAGS	= --ice --include-dir IceStorm --checksum

!include $(top_srcdir)\config\Make.rules.mak

$(LIBNAME): $(OBJS) sdks
	$(AR) $(ARFLAGS) $(OBJS) /out:$(LIBNAME)

clean::
	-del /q $(SOURCE_DIR)\IceStorm.cpp $(HDIR)\IceStorm.h
	-del /q $(SOURCE_DIR)\Metrics.cpp $(HDIR)\Metrics.h
	-del /q $(ARCH)\$(CONFIG)\*.obj
	-del /q $(PDBNAME)

install:: all
