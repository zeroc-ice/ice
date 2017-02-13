# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME     	= $(top_srcdir)\lib\icediscovery$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\icediscovery$(SOVERSION)$(LIBSUFFIX)$(COMPSUFFIX).dll

TARGETS		= $(LIBNAME) $(DLLNAME)

SDIR 		= $(slicedir)\IceDiscovery
SLICE_OBJS	= .\IceDiscovery.obj 

OBJS		= .\LocatorI.obj \
		  .\LookupI.obj \
		  .\PluginI.obj \
		  $(SLICE_OBJS)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I.. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN -DICE_DISCOVERY_API_EXPORTS
SLICE2CPPFLAGS	= --ice --include-dir IceDiscovery $(SLICE2CPPFLAGS)

LINKWITH        = $(LIBS)

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
!endif

RES_FILE        = IceDiscovery.res

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS) IceDiscovery.res
	$(LINK) $(BASE):0x28000000 $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) $(RES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

IceDiscovery.h IceDiscovery.cpp: $(slicedir)\IceDiscovery\IceDiscovery.ice "$(SLICE2CPP)" "$(SLICEPARSERLIB)"
	del /q $(*F).h $(*F).cpp
	"$(SLICE2CPP)" $(SLICE2CPPFLAGS) $(slicedir)\IceDiscovery\IceDiscovery.ice

clean::
	-del /q IceDiscovery.cpp IceDiscovery.h
	-del /q IceDiscovery.res

install:: all
	copy $(LIBNAME) "$(install_libdir)"
	copy $(DLLNAME) "$(install_bindir)"


!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(DLLNAME:.dll=.pdb) "$(install_bindir)"

!endif
