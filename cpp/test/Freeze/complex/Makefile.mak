# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

CLIENT		= client.exe

TARGETS		= $(CLIENT)

SLICE_OBJS	= .\Complex.obj

BISON_FLEX_OBJS = .\Grammar.obj \
                  .\Scanner.obj


OBJS		= $(SLICE_OBJS) \
		  $(BISON_FLEX_OBJS) \
		  .\Parser.obj \
		  .\Client.obj \
		  .\ComplexDict.obj

all:: ComplexDict.cpp ComplexDict.h

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
!endif

$(CLIENT): $(OBJS)
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) $(SETARGV) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) 
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

ComplexDict.h ComplexDict.cpp: Complex.ice "$(SLICE2FREEZE)" "$(SLICEPARSERLIB)"
	del /q ComplexDict.h ComplexDict.cpp
	"$(SLICE2FREEZE)" -I. -I"$(slicedir)" --dict Complex::ComplexDict,Complex::Key,Complex::Node ComplexDict Complex.ice

clean::
	del /q Complex.cpp Complex.h
	del /q ComplexDict.h ComplexDict.cpp
	-if exist db\__Freeze rmdir /q /s db\__Freeze
	-for %f in (db\*) do if not %f == db\.gitignore del /q %f
