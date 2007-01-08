# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME     	= $(top_srcdir)\lib\icessl$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\icessl$(SOVERSION)$(LIBSUFFIX).dll

TARGETS		= $(LIBNAME) $(DLLNAME)

OBJS		= AcceptorI.obj \
		  Certificate.obj \
                  ConnectorI.obj \
                  EndpointI.obj \
                  Instance.obj \
                  PluginI.obj \
                  TransceiverI.obj \
                  Util.obj \
		  RFC2253.obj \
		  TrustManager.obj

SRCS		= $(OBJS:.obj=.cpp)

HDIR		= $(includedir)\IceSSL

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I.. $(CPPFLAGS) -DICE_SSL_API_EXPORTS -DFD_SETSIZE=1024 -DWIN32_LEAN_AND_MEAN

LINKWITH        = $(OPENSSL_LIBS) $(LIBS)
!if "$(BORLAND_HOME)" == ""
LINKWITH	= $(LINKWITH) ws2_32.lib
!endif

!if "$(BORLAND_HOME)" == "" & "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
!endif

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS)
	$(LINK) $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$(DLLNAME) $(PRELIBS)$(LINKWITH)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)

clean::
	del /q $(DLLNAME:.dll=.*)

install:: all
	copy $(LIBNAME) $(install_libdir)
	copy $(DLLNAME) $(install_bindir)

!if "$(OPTIMIZE)" != "yes"

!if "$(BORLAND_HOME)" == ""

install:: all
	copy $(DLLNAME:.dll=.pdb) $(install_bindir)

!else

install:: all
	copy $(DLLNAME:.dll=.tds) $(install_bindir)

!endif

!endif

!include .depend
