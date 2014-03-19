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

TARGETS         = $(LIBNAME)

OBJS		= $(ARCH)\$(CONFIG)\Admin.obj \
		  $(ARCH)\$(CONFIG)\Locator.obj \
	          $(ARCH)\$(CONFIG)\Query.obj \
		  $(ARCH)\$(CONFIG)\Exception.obj \
	   	  $(ARCH)\$(CONFIG)\Descriptor.obj \
		  $(ARCH)\$(CONFIG)\FileParser.obj \
	          $(ARCH)\$(CONFIG)\Observer.obj \
	   	  $(ARCH)\$(CONFIG)\Session.obj \
	   	  $(ARCH)\$(CONFIG)\Registry.obj \
	   	  $(ARCH)\$(CONFIG)\UserAccountMapper.obj

SLICE_SRCS	= $(SDIR)/Admin.ice \
                  $(SDIR)/Exception.ice \
                  $(SDIR)/FileParser.ice \
                  $(SDIR)/Locator.ice \
                  $(SDIR)/Query.ice \
		  $(SDIR)/Registry.ice \
		  $(SDIR)/Session.ice \
		  $(SDIR)/Observer.ice \
		  $(SDIR)/Descriptor.ice \
		  $(SDIR)/UserAccountMapper.ice

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

depend::
	del /q .depend.mak

.cpp.depend:
	$(CXX) /Fo$(ARCH)\$(CONFIG)\ /Fd$(ARCH)\$(CONFIG)\ /Zs /showIncludes $(CXXFLAGS) $(CPPFLAGS) $< 2>&1 | python.exe $(ice_dir)/config/makedepend-winrt.py  $<

depend:: $(ARCH)\$(CONFIG) $(SLICE_SRCS) $(SRCS) $(SRCS_DEPEND)
	@if not "$(SLICE_SRCS)" == "" \
		$(SLICE2CPP) --depend $(SLICE2CPPFLAGS) $(SLICE_SRCS) | python.exe $(ice_dir)\config\makedepend-winrt.py

clean::
	-del /q $(SOURCE_DIR)\FileParser.cpp $(HDIR)\FileParser.h
	-del /q $(SOURCE_DIR)\Admin.cpp $(HDIR)\Admin.h
	-del /q $(SOURCE_DIR)\Descriptor.cpp $(HDIR)\Descriptor.h
	-del /q $(SOURCE_DIR)\Exception.cpp $(HDIR)\Exception.h
	-del /q $(SOURCE_DIR)\Locator.cpp $(HDIR)\Locator.h
	-del /q $(SOURCE_DIR)\Observer.cpp $(HDIR)\Observer.h
	-del /q $(SOURCE_DIR)\Query.cpp $(HDIR)\Query.h
	-del /q $(SOURCE_DIR)\Session.cpp $(HDIR)\Session.h
	-del /q $(SOURCE_DIR)\Registry.cpp $(HDIR)\Registry.h
	-del /q $(SOURCE_DIR)\UserAccountMapper.cpp $(HDIR)\UserAccountMapper.h
	-del /q $(ARCH)\$(CONFIG)\*.obj
	-del /q $(PDBNAME)

install:: all

!include .depend.mak
