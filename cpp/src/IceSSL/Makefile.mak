# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME     	= $(top_srcdir)\lib\icessl$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\icessl$(SOVERSION)$(LIBSUFFIX).dll

TARGETS		= $(LIBNAME) $(DLLNAME)

OBJS		= AcceptorI.o \
		  Certificate.o \
                  ConnectorI.o \
                  EndpointI.o \
                  Instance.o \
                  PluginI.o \
                  TransceiverI.o \
                  Util.o \
		  RFC2253.o \
		  TrustManager.o

SRCS		= $(OBJS:.o=.cpp)

HDIR		= $(includedir)\IceSSL

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I.. $(CPPFLAGS) -DICE_SSL_API_EXPORTS

LINKWITH        = $(OPENSSL_LIBS) $(LIBS)

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS)
	del /q $@
	$(LINK) $(LD_DLLFLAGS) $(OBJS), $(DLLNAME),, $(LINKWITH)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)

install:: all
	copy $(LIBNAME) $(install_libdir)
	copy $(DLLNAME) $(install_bindir)

!include .depend
