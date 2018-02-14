# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..
LIBNAME		= $(SDK_LIBRARY_PATH)\glacier2.lib
TARGETS		= $(LIBNAME)
SOURCE_DIR	= ..

TARGETS         = $(LIBNAME)

OBJS		= $(ARCH)\$(CONFIG)\Metrics.obj \
		  $(ARCH)\$(CONFIG)\PermissionsVerifier.obj \
		  $(ARCH)\$(CONFIG)\Router.obj \
		  $(ARCH)\$(CONFIG)\Session.obj \
		  $(ARCH)\$(CONFIG)\SessionHelper.obj \
		  $(ARCH)\$(CONFIG)\SSLInfo.obj

SRCS		= $(OBJS:.obj=.cpp)
SRCS		= $(SRCS:x86\=)
SRCS		= $(SRCS:x64\=)
SRCS		= $(SRCS:arm\=)
SRCS		= $(SRCS:Retail\=..\)
SRCS		= $(SRCS:Debug\=..\)

SRCS		= $(SRCS) \
		..\RouterF.cpp

HDIR		= $(headerdir)\Glacier2
SDIR		= $(slicedir)\Glacier2

SLICE_SRCS	= Glacier2\Metrics.ice \
		  Glacier2\PermissionsVerifier.ice \
		  Glacier2\PermissionsVerifierF.ice \
		  Glacier2\Router.ice \
		  Glacier2\RouterF.ice \
		  Glacier2\Session.ice \
		  Glacier2\SSLInfo.ice

PDBNAME		= $(LIBNAME:.lib=.pdb)
CPPFLAGS	= /Fd$(PDBNAME) -I..\.. $(CPPFLAGS) -DICE_BUILDING_GLACIER2 -DWIN32_LEAN_AND_MEAN

!include $(top_srcdir)\config\Make.rules.mak

SLICE2CPPFLAGS	= --ice --include-dir Glacier2 $(SLICE2CPPFLAGS)

$(LIBNAME): $(OBJS) $(HDIR)\PermissionsVerifierF.h $(HDIR)\RouterF.h sdks
	$(AR) $(ARFLAGS) $(OBJS) /out:$(LIBNAME)

clean::
	-del /q $(SOURCE_DIR)\PermissionsVerifierF.cpp $(HDIR)\PermissionsVerifierF.h
	-del /q $(SOURCE_DIR)\PermissionsVerifier.cpp $(HDIR)\PermissionsVerifier.h
	-del /q $(SOURCE_DIR)\RouterF.cpp $(HDIR)\RouterF.h
	-del /q $(SOURCE_DIR)\Router.cpp $(HDIR)\Router.h
	-del /q $(SOURCE_DIR)\Session.cpp $(HDIR)\Session.h
	-del /q $(SOURCE_DIR)\SSLInfo.cpp $(HDIR)\SSLInfo.h
	-del /q $(SOURCE_DIR)\Metrics.cpp $(HDIR)\Metrics.h
	-del /q $(ARCH)\$(CONFIG)\*.obj
	-del /q $(PDBNAME)

install:: all
