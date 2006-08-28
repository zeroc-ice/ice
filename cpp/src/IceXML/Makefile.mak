# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..
 
LIBNAME		= $(top_srcdir)\lib\icexml$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\icexml$(SOVERSION)$(LIBSUFFIX).dll

TARGETS		= $(LIBNAME) $(DLLNAME)

OBJS		= Parser.obj

SRCS		= $(OBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= $(CPPFLAGS) -DICE_XML_API_EXPORTS

LINKWITH        = $(EXPAT_LIBS) $(BASELIBS)

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS)
	del /q $@
	$(LINK) $(LD_DLLFLAGS) $(OBJS), $(DLLNAME),, $(LINKWITH)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)

install:: all
	copy $(LIBNAME) $(install_libdir)
	copy $(DLLNAME) $(install_bindir)

!include .depend
