# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

CLIENT		= client.exe

TARGETS		= $(CLIENT)

OBJS		= Client.obj \
                  ByteIntMap.obj \
                  IntIdentityMap.obj \
                  IntIdentityMapWithIndex.obj \
                  SortedMap.obj \
		  WstringWstringMap.obj

SRCS		= $(OBJS:.obj=.cpp)

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I. -I..\..\include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(CPP_COMPILER)" != "BCC2006" && "$(OPTIMIZE)" != "yes"
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
!endif

$(CLIENT): $(OBJS)
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) $(SETARGV) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) freeze$(LIBSUFFIX).lib
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

ByteIntMap.h ByteIntMap.cpp: $(SLICE2FREEZE)
	del /q ByteIntMap.h ByteIntMap.cpp
	$(SLICE2FREEZE) --dict Test::ByteIntMap,byte,int --dict-index Test::ByteIntMap ByteIntMap

IntIdentityMap.h IntIdentityMap.cpp: $(SLICE2FREEZE)
	del /q IntIdentityMap.h IntIdentityMap.cpp
	$(SLICE2FREEZE) --ice $(SLICE2CPPFLAGS)  --dict Test::IntIdentityMap,int,Ice::Identity IntIdentityMap $(slicedir)\Ice\Identity.ice

IntIdentityMapWithIndex.h IntIdentityMapWithIndex.cpp: $(SLICE2FREEZE)
	del /q IntIdentityMapWithIndex.h IntIdentityMapWithIndex.cpp
	$(SLICE2FREEZE) --ice $(SLICE2CPPFLAGS) --dict Test::IntIdentityMapWithIndex,int,Ice::Identity --dict-index Test::IntIdentityMapWithIndex,category IntIdentityMapWithIndex $(slicedir)\Ice\Identity.ice

SortedMap.h SortedMap.cpp: $(SLICE2FREEZE)
	del /q SortedMap.h SortedMap.cpp
	$(SLICE2FREEZE)  --ice $(SLICE2CPPFLAGS) --dict Test::SortedMap,int,Ice::Identity,sort SortedMap --dict-index "Test::SortedMap,category,sort,std::greater<std::string>" $(slicedir)\Ice\Identity.ice

WstringWstringMap.h WstringWstringMap.cpp: $(SLICE2FREEZE)
	del /q WstringWstringMap.h WstringWstringMap.cpp
	$(SLICE2FREEZE) --dict Test::WstringWstringMap,[\"cpp:type:wstring\"]string,[\"cpp:type:wstring\"]string --dict-index Test::WstringWstringMap WstringWstringMap

clean::
	del /q ByteIntMap.h ByteIntMap.cpp IntIdentityMap.h IntIdentityMap.cpp IntIdentityMapWithIndex.h IntIdentityMapWithIndex.cpp SortedMap.h SortedMap.cpp WstringWstringMap.h WstringWstringMap.cpp

clean::
	del /q db\binary db\binary.* db\intIdentity db\intIdentity.* db\__catalog db\log.*

!include .depend
