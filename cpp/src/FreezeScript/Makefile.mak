# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

TRANSFORMDB	= $(top_srcdir)\bin\transformdb.exe
DUMPDB		= $(top_srcdir)\bin\dumpdb.exe

TARGETS		= $(TRANSFORMDB) $(DUMPDB)

BISON_FLEX_OBJS = .\Grammar.obj \
                  .\Scanner.obj

COMMON_OBJS	= .\AssignVisitor.obj \
		  .\Data.obj \
		  .\Error.obj \
		  .\Functions.obj \
		  .\Exception.obj \
		  .\Parser.obj \
		  .\Print.obj \
		  .\Util.obj \
                  $(BISON_FLEX_OBJS)

TRANSFORM_OBJS =  .\TransformAnalyzer.obj \
		  .\TransformVisitor.obj \
		  .\Transformer.obj \
		  .\transformdb.obj

DUMP_OBJS	= .\DumpDescriptors.obj \
		  .\DumpDB.obj

OBJS		= $(COMMON_OBJS) $(TRANSFORM_OBJS) $(DUMP_OBJS)

HDIR		= $(headerdir)\FreezeScript

!include $(top_srcdir)/config/Make.rules.mak

$(OBJS)		: $(DB_NUPKG)

CPPFLAGS	= -I.. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN $(DB_CPPFLAGS)
BISONFLAGS	= --name-prefix "freeze_script_" $(BISONFLAGS)

LINKWITH	= $(LIBS) $(DB_LIBS)

!if "$(GENERATE_PDB)" == "yes"
TPDBFLAGS        = /pdb:$(TRANSFORMDB:.exe=.pdb)
DPDBFLAGS        = /pdb:$(DUMPDB:.exe=.pdb)
!endif

TRES_FILE        = TransformDB.res
DRES_FILE        = DumpDB.res

$(TRANSFORMDB): $(TRANSFORM_OBJS) $(COMMON_OBJS) TransformDB.res
	$(LINK) $(LD_EXEFLAGS) $(DB_LDFLAGS) $(TPDBFLAGS) $(TRANSFORM_OBJS) $(COMMON_OBJS) $(SETARGV) $(PREOUT)$@ \
		$(PRELIBS)$(LINKWITH) $(TRES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest
    @if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

$(DUMPDB): $(DUMP_OBJS) $(COMMON_OBJS) DumpDB.res
	$(LINK) $(LD_EXEFLAGS) $(DB_LDFLAGS) $(DPDBFLAGS) $(DUMP_OBJS) $(COMMON_OBJS) $(SETARGV) $(PREOUT)$@ \
		$(PRELIBS)$(LINKWITH) $(DRES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest
    @if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

clean::
	-del /q $(TRANSFORMDB:.exe=.*)
	-del /q $(DUMPDB:.exe=.*)
	-del /q TransformDB.res DumpDB.res

install:: all
	copy $(TRANSFORMDB) "$(install_bindir)"
	copy $(DUMPDB) "$(install_bindir)"

!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(TRANSFORMDB:.exe=.pdb) "$(install_bindir)"
	copy $(DUMPDB:.exe=.pdb) "$(install_bindir)"

!endif
