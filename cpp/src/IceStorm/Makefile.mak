# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\icestormservice$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\icestormservice$(SOVERSION)$(LIBSUFFIX)$(COMPSUFFIX).dll

ADMIN		= $(top_srcdir)\bin\icestormadmin.exe
DB		= $(top_srcdir)\bin\icestormdb.exe

TARGETS		= $(LIBNAME) $(DLLNAME) $(ADMIN) $(DB)

LSLICE_OBJS      = .\Election.obj \
		  .\IceStormInternal.obj \
		  .\Instrumentation.obj \
		  .\LinkRecord.obj \
		  .\LLURecord.obj \
		  .\SubscriberRecord.obj

BISON_FLEX_OBJS = .\Grammar.obj \
                  .\Scanner.obj

LIB_OBJS	= .\Instance.obj \
		  .\InstrumentationI.obj \
		  .\NodeI.obj \
		  .\Observers.obj \
		  .\Service.obj \
		  .\Subscriber.obj \
		  .\TopicI.obj \
		  .\TopicManagerI.obj \
		  .\TraceLevels.obj \
		  .\TransientTopicI.obj \
		  .\TransientTopicManagerI.obj \
		  .\Util.obj \
                  $(LSLICE_OBJS)

AOBJS		= .\Admin.obj \
		  .\Parser.obj \
                  $(LSLICE_OBJS) \
                  $(BISON_FLEX_OBJS)

DSLICE_OBJS	= .\DBTypes.obj

DOBJS		= .\IceStormDB.obj \
		  $(DSLICE_OBJS)

OBJS		= $(LIB_OBJS) \
		  $(AOBJS) \
		  $(DOBJS)

SLICE_OBJS	= $(LSLICE_OBJS) \
		  $(DSLICE_OBJS)

HDIR		= $(headerdir)\IceStorm
SDIR		= $(slicedir)\IceStorm

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I.. $(LMDB_CPPFLAGS) $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
ICECPPFLAGS	= $(ICECPPFLAGS) -I..
SLICE2CPPFLAGS	= --ice --include-dir IceStorm $(SLICE2CPPFLAGS)
LINKWITH 	= $(LIBS)
ALINKWITH 	= $(LIBS) 
DLINKWITH 	= $(LIBS) 

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
APDBFLAGS       = /pdb:$(ADMIN:.exe=.pdb)
DPDBFLAGS       = /pdb:$(DB:.exe=.pdb)
!endif

RES_FILE        = IceStormService.res
ARES_FILE       = IceStormAdmin.res
DRES_FILE       = IceStormDB.res

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(LIB_OBJS) $(RES_FILE)
	$(LINK) $(BASE):0x2C000000 $(LD_DLLFLAGS) $(PDBFLAGS) $(LIB_OBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) $(RES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

$(ADMIN): $(AOBJS) $(ARES_FILE)
	$(LINK) $(LD_EXEFLAGS) $(APDBFLAGS) $(AOBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(ALINKWITH) $(ARES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

$(DB): $(DOBJS) $(DRES_FILE)
	$(LINK) $(LD_EXEFLAGS) $(DPDBFLAGS) $(DOBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(DLINKWITH) $(DRES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

# Implicit rule to build the private IceStorm .ice files.
{..\IceStorm\}.ice{..\IceStorm\}.h:
	del /q $(*F).h $(*F).cpp
	"$(SLICE2CPP)" $(SLICE2CPPFLAGS) $(*F).ice

clean::
	-del /q Election.cpp Election.h
	-del /q IceStormInternal.cpp IceStormInternal.h
	-del /q Instrumentation.cpp Instrumentation.h
	-del /q LinkRecord.cpp LinkRecord.h
	-del /q LLURecord.cpp LLURecord.h
	-del /q SubscriberRecord.cpp SubscriberRecord.h
	-del /q $(ADMIN:.exe=.*)
	-del /q $(DB:.exe=.*)
	-del /q IceStormAdmin.res IceStormService.res

install:: all
	copy $(LIBNAME) "$(install_libdir)"
	copy $(DLLNAME) "$(install_bindir)"
	copy $(ADMIN) "$(install_bindir)"
	copy $(DB) "$(install_bindir)"

!if "$(GENERATE_PDB)" == "yes"

install:: all
        copy $(ADMIN:.exe=.pdb) "$(install_bindir)"
        copy $(DB:.exe=.pdb) "$(install_bindir)"
        copy $(DLLNAME:.dll=.pdb) "$(install_bindir)"

!endif
