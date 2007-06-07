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

OBJS		= Client.obj BenchTypes.obj Test.obj

SRCS		= $(OBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(CPP_COMPILER)" != "BCC2006" && "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
!endif

$(CLIENT): $(OBJS)
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) $(SETARGV) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) freeze$(LIBSUFFIX).lib
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

BenchTypes.h BenchTypes.cpp: Test.ice $(SLICE2FREEZE)
	del /q BenchTypes.h BenchTypes.cpp
	$(SLICE2FREEZE) -I$(slicedir) --dict Demo::IntIntMap,int,int --dict Demo::Struct1Struct2Map,Demo::Struct1,Demo::Struct2 \
	--dict Demo::Struct1Class1Map,Demo::Struct1,Demo::Class1 \
	--dict Demo::Struct1ObjectMap,Demo::Struct1,Object \
        --dict Demo::IndexedIntIntMap,int,int --dict-index Demo::IndexedIntIntMap \
        --dict Demo::IndexedStruct1Struct2Map,Demo::Struct1,Demo::Struct2 \
        --dict-index Demo::IndexedStruct1Struct2Map,s,case-insensitive \
        --dict-index Demo::IndexedStruct1Struct2Map,s1 \
        --dict Demo::IndexedStruct1Class1Map,Demo::Struct1,Demo::Class1 \
        --dict-index Demo::IndexedStruct1Class1Map,s,case-sensitive \
        BenchTypes Test.ice

clean::
	del /q BenchTypes.h BenchTypes.cpp

clean::
	del /q Test.cpp Test.h

!include .depend
