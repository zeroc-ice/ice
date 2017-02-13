# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

ROUTER		= $(top_srcdir)\bin\glacier2router.exe
TARGETS         = $(ROUTER)

SLICE_OBJS      = .\Instrumentation.obj

OBJS		= .\Blobject.obj \
		  .\ClientBlobject.obj \
		  .\FilterI.obj \
		  .\FilterManager.obj \
		  .\Glacier2Router.obj \
		  .\Instance.obj \
		  .\InstrumentationI.obj \
		  .\ProxyVerifier.obj \
		  .\RequestQueue.obj \
		  .\RouterI.obj \
		  .\RoutingTable.obj \
		  .\ServerBlobject.obj \
		  .\SessionRouterI.obj \
		  $(SLICE_OBJS)

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I.. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
SLICE2CPPFLAGS	= --include-dir Glacier2 $(SLICE2CPPFLAGS)

LINKWITH 	= $(LIBS)

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(ROUTER:.exe=.pdb)
!endif

RES_FILE       = Glacier2Router.res

$(ROUTER): $(OBJS) Glacier2Router.res
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) $(OBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) $(RES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

clean::
        -del /q Instrumentation.cpp Instrumentation.h
	-del /q Glacier2Router.res
	-del /q $(ROUTER:.exe=.*)

install:: all
	copy $(ROUTER) "$(install_bindir)"


!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(ROUTER:.exe=.pdb) "$(install_bindir)"

!endif
