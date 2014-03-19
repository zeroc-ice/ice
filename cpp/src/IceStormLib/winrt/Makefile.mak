# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

SLICE_SRCS	= $(SDIR)/IceStorm.ice \
		  $(SDIR)/Metrics.ice

SRCS		= $(OBJS:.obj=.cpp)
SRCS		= $(SRCS:x86\=)
SRCS		= $(SRCS:x64\=)
SRCS		= $(SRCS:arm\=)
SRCS		= $(SRCS:Retail\=..\)
SRCS		= $(SRCS:Debug\=..\)

HDIR		= $(headerdir)\IceStorm
SDIR		= $(slicedir)\IceStorm

PDBNAME		= $(LIBNAME:.lib=.pdb)
CPPFLAGS	= /Fd$(PDBNAME) -I..\.. -DWIN32_LEAN_AND_MEAN 
ICECPPFLAGS	= -I..
SLICE2CPPFLAGS	= --ice --include-dir IceStorm --checksum --dll-export ICE_STORM_LIB_API

!include $(top_srcdir)\config\Make.rules.mak

$(LIBNAME): $(OBJS) sdks
	$(AR) $(ARFLAGS) $(OBJS) /out:$(LIBNAME)

depend::
	del /q .depend.mak

.cpp.depend:
	$(CXX) /Fo$(ARCH)\$(CONFIG)\ /Fd$(ARCH)\$(CONFIG)\ /Zs /showIncludes $(CXXFLAGS) $(CPPFLAGS) $< 2>&1 | python.exe $(ice_dir)/config/makedepend-winrt.py  $<

depend:: $(ARCH)\$(CONFIG) $(SLICE_SRCS) $(SRCS) $(SRCS_DEPEND)
	@if not "$(SLICE_SRCS)" == "" \
		$(SLICE2CPP) --depend $(SLICE2CPPFLAGS) $(SLICE_SRCS) | python.exe $(ice_dir)\config\makedepend-winrt.py

clean::
	-del /q $(SOURCE_DIR)\IceStorm.cpp $(HDIR)\IceStorm.h
	-del /q $(SOURCE_DIR)\Metrics.cpp $(HDIR)\Metrics.h
	-del /q $(ARCH)\$(CONFIG)\*.obj
	-del /q $(PDBNAME)

install:: all

!include .depend.mak
