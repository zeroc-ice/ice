# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME     	= $(top_srcdir)\lib\icestringconverter$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\icestringconverter$(SOVERSION)$(LIBSUFFIX).dll

TARGETS		= $(LIBNAME) $(DLLNAME)

OBJS		=  Plugin.obj

SRCS		= $(OBJS:.obj=.cpp)


!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

LINKWITH        = $(LIBS)

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
!endif

!if "$(CPP_COMPILER)" == "BCC2007"
RES_FILE        = ,, IceStringConverter.res
!else
RES_FILE        = IceStringConverter.res
!endif

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS) IceStringConverter.res
	$(LINK) $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) $(RES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

IceStringConverter.res: IceStringConverter.rc
	rc.exe $(RCFLAGS) IceStringConverter.rc

clean::
	del /q $(DLLNAME:.dll=.*)
	del /q IceStringConverter.res

install:: all
	copy $(LIBNAME) $(install_libdir)
	copy $(DLLNAME) $(install_bindir)

!if "$(OPTIMIZE)" != "yes"

!if "$(CPP_COMPILER)" == "BCC2007"

install:: all
	copy $(DLLNAME:.dll=.tds) $(install_bindir)

!else

install:: all
	copy $(DLLNAME:.dll=.pdb) $(install_bindir)

!endif

!endif

!include .depend
