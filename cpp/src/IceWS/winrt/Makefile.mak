# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..
LIBNAME		= $(SDK_LIBRARY_PATH)\icews.lib
TARGETS		= $(LIBNAME)
SOURCE_DIR	= ..

TARGETS         = $(LIBNAME)

OBJS		= $(ARCH)\$(CONFIG)\EndpointInfo.obj \
		  $(ARCH)\$(CONFIG)\ConnectionInfo.obj

# OBJS		= $(ARCH)\$(CONFIG)\AcceptorI.obj \
#                   $(ARCH)\$(CONFIG)\ConnectorI.obj \
#                   $(ARCH)\$(CONFIG)\ConnectionInfo.obj \
#                   $(ARCH)\$(CONFIG)\EndpointInfo.obj \
#                   $(ARCH)\$(CONFIG)\EndpointI.obj \
#                   $(ARCH)\$(CONFIG)\Instance.obj \
#                   $(ARCH)\$(CONFIG)\PluginI.obj \
#                   $(ARCH)\$(CONFIG)\TransceiverI.obj \
#                   $(ARCH)\$(CONFIG)\Util.obj

SLICE_SRCS	= $(SDIR)/EndpointInfo.ice \
                  $(SDIR)/ConnectionInfo.ice

SRCS		= $(OBJS:.obj=.cpp)
SRCS		= $(SRCS:x86\=)
SRCS		= $(SRCS:x64\=)
SRCS		= $(SRCS:arm\=)
SRCS		= $(SRCS:Retail\=..\)
SRCS		= $(SRCS:Debug\=..\)

HDIR		= $(headerdir)\IceWS
SDIR		= $(slicedir)\IceWS

PDBNAME		= $(LIBNAME:.lib=.pdb)
CPPFLAGS        = /Fd$(PDBNAME) -I..\.. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
SLICE2CPPFLAGS	= --checksum --ice --include-dir IceWS --dll-export ICE_WS_API

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
	-del /q $(SOURCE_DIR)\EndpointInfo.cpp $(HDIR)\EndpointInfo.h
	-del /q $(SOURCE_DIR)\ConnectionInfo.cpp $(HDIR)\ConnectionInfo.h
	-del /q $(ARCH)\$(CONFIG)\*.obj
	-del /q $(PDBNAME)

install:: all

!include .depend.mak
