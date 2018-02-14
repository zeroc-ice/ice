# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= php_ice$(LIBSUFFIX).lib
DLLNAME		= $(libdir)\php_ice$(LIBSUFFIX).dll

TARGETS		= $(LIBNAME) $(DLLNAME)

OBJS		= .\Communicator.obj \
		  .\Connection.obj \
		  .\Endpoint.obj \
		  .\Init.obj \
		  .\Logger.obj \
		  .\Operation.obj \
		  .\Properties.obj \
		  .\Proxy.obj \
		  .\Types.obj \
		  .\Util.obj

!include $(top_srcdir)\config\Make.rules.mak.php

CPPFLAGS	= -I. -I.. $(CPPFLAGS) $(ICE_CPPFLAGS) $(PHP_CPPFLAGS) -DZEND_WIN32_KEEP_INLINE

!if "$(ARCH)" == "x86"
CPPFLAGS = $(CPPFLAGS) -D_USE_32BIT_TIME_T
!endif

!if "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(LIBNAME:.lib=.pdb)
!endif

LINKWITH        = $(ICE_LIBS) $(PHP_LIBS) $(CXXLIBS) icediscovery$(LIBSUFFIX).lib icelocatordiscovery$(LIBSUFFIX).lib

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS) IcePHP.res
	$(LINK) $(ICE_LDFLAGS) $(PHP_LDFLAGS) $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) \
		$(PREOUT)$(DLLNAME) $(PRELIBS)$(LINKWITH) IcePHP.res
	move $(DLLNAME:.dll=.lib) $(LIBNAME)

clean::
	-del /q IcePHP.res

install:: all
	copy $(DLLNAME) "$(install_libdir)"
