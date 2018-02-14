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

OBJS		= .\ByteIntMap.obj \
		  .\IntIdentityMap.obj \
		  .\IntIdentityMapWithIndex.obj \
		  .\SortedMap.obj \
		  .\WstringWstringMap.obj \
		  .\Client.obj

all:: 		  ByteIntMap.cpp ByteIntMap.h \
		  IntIdentityMap.cpp IntIdentityMap.h \
		  IntIdentityMapWithIndex.cpp IntIdentityMapWithIndex.h \
		  SortedMap.cpp SortedMap.h \
		  WstringWstringMap.cpp WstringWstringMap.h

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I. -I..\..\include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
!endif

$(CLIENT): $(OBJS)
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) $(SETARGV) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) 
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

ByteIntMap.h ByteIntMap.cpp: "$(SLICE2FREEZE)" "$(SLICEPARSERLIB)"
	del /q ByteIntMap.h ByteIntMap.cpp
	"$(SLICE2FREEZE)" --dict Test::ByteIntMap,byte,int --dict-index Test::ByteIntMap,sort ByteIntMap

IntIdentityMap.h IntIdentityMap.cpp: "$(slicedir)\Ice\Identity.ice" "$(SLICE2FREEZE)" "$(SLICEPARSERLIB)"
	del /q IntIdentityMap.h IntIdentityMap.cpp
	"$(SLICE2FREEZE)" --ice $(SLICE2CPPFLAGS)  --dict Test::IntIdentityMap,int,Ice::Identity IntIdentityMap "$(slicedir)\Ice\Identity.ice"

IntIdentityMapWithIndex.h IntIdentityMapWithIndex.cpp: "$(slicedir)\Ice\Identity.ice" "$(SLICE2FREEZE)" "$(SLICEPARSERLIB)"
	del /q IntIdentityMapWithIndex.h IntIdentityMapWithIndex.cpp
	"$(SLICE2FREEZE)" --ice $(SLICE2CPPFLAGS) --dict Test::IntIdentityMapWithIndex,int,Ice::Identity --dict-index Test::IntIdentityMapWithIndex,category IntIdentityMapWithIndex "$(slicedir)\Ice\Identity.ice"

SortedMap.h SortedMap.cpp: "$(slicedir)\Ice\Identity.ice" "$(SLICE2FREEZE)" "$(SLICEPARSERLIB)"
	del /q SortedMap.h SortedMap.cpp
	"$(SLICE2FREEZE)"  --ice $(SLICE2CPPFLAGS) --dict Test::SortedMap,int,Ice::Identity,sort SortedMap --dict-index "Test::SortedMap,category,sort,std::greater<std::string>" "$(slicedir)\Ice\Identity.ice"

WstringWstringMap.h WstringWstringMap.cpp: "$(SLICE2FREEZE)" "$(SLICEPARSERLIB)"
	del /q WstringWstringMap.h WstringWstringMap.cpp
	"$(SLICE2FREEZE)" --dict Test::WstringWstringMap,[\"cpp:type:wstring\"]string,[\"cpp:type:wstring\"]string --dict-index Test::WstringWstringMap WstringWstringMap

clean::
	del /q ByteIntMap.h ByteIntMap.cpp
        del /q IntIdentityMap.h IntIdentityMap.cpp
	del /q IntIdentityMapWithIndex.h IntIdentityMapWithIndex.cpp
        del /q SortedMap.h SortedMap.cpp
	del /q WstringWstringMap.h WstringWstringMap.cpp
	-if exist db\__Freeze rmdir /q /s db\__Freeze
	-for %f in (db\*) do if not %f == db\.gitignore del /q %f
