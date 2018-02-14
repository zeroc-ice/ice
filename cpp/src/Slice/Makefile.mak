# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\slice$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\slice$(SOVERSION)$(LIBSUFFIX)$(COMPSUFFIX).dll

TARGETS		= $(LIBNAME) $(DLLNAME)

BISON_FLEX_OBJS = .\Grammar.obj \
                  .\Scanner.obj

OBJS		= .\Checksum.obj \
		  .\CPlusPlusUtil.obj \
		  .\CsUtil.obj \
		  .\DotNetNames.obj \
		  .\FileTracker.obj \
		  .\JavaUtil.obj \
		  .\MD5.obj \
		  .\MD5I.obj \
		  .\Parser.obj \
		  .\PHPUtil.obj \
		  .\Preprocessor.obj \
		  .\PythonUtil.obj \
		  .\Python.obj \
		  .\RubyUtil.obj \
		  .\Util.obj \
		  .\Ruby.obj \
		  $(BISON_FLEX_OBJS)

!include $(top_srcdir)/config/Make.rules.mak

$(OBJS)		: $(MCPP_NUPKG)

CPPFLAGS	= -I.. $(CPPFLAGS) -DSLICE_API_EXPORTS  -DWIN32_LEAN_AND_MEAN
BISONFLAGS	= --name-prefix "slice_" $(BISONFLAGS)

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
!endif

RES_FILE        = Slice.res

!if "$(STATICLIBS)" == "yes"

$(DLLNAME):

$(LIBNAME): $(OBJS)
	$(AR) $(ARFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@

!else

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS) Slice.res
	$(LINK) $(BASE):0x21000000 $(LD_DLLFLAGS) $(MCPP_LDFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(BASELIBS) $(MCPP_LIBS) \
		$(RES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
		$(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

!endif

clean::
	-del /q Slice.res

install:: all
	copy $(LIBNAME) "$(install_libdir)"
	copy $(DLLNAME) "$(install_bindir)"


!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(DLLNAME:.dll=.pdb) "$(install_bindir)"

!endif
