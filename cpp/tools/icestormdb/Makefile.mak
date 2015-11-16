# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

ICESTORMDB	= $(top_srcdir)\bin\icestormdb35.exe

TARGETS         = $(ICESTORMDB)

SLICE_OBJS	= DBTypes.obj

OBJS		= IceStormDB.obj \
		  LLUMap.obj \
		  SubscriberMap.obj \
		  $(SLICE_OBJS)

SRCS            = $(OBJS:.obj=.cpp)

SDIR		= $(slicedir)\IceStorm

SLICE2FREEZECMD = "$(SLICE2FREEZE)" --ice $(ICECPPFLAGS)

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I. -I..\..\src $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
SLICE2CPPFLAGS	= --ice -I. $(SLICE2CPPFLAGS)
LINKWITH 	= $(LIBS) icestorm$(LIBSUFFIX).lib freeze$(LIBSUFFIX).lib icepatch2$(LIBSUFFIX).lib

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS       	= /pdb:$(ICESTORMDB:.exe=.pdb)
!endif

RES_FILE       	= IceStormDB.res

$(ICESTORMDB): $(OBJS) IceStormDB.res
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) $(OBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) $(RES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) &&\
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

IceStormDB.obj: SubscriberRecord.h LLURecord.h

SubscriberRecord.ice: ..\..\src\IceStorm\SubscriberRecord.ice
	copy ..\..\src\IceStorm\SubscriberRecord.ice .

SubscriberRecord.h: SubscriberRecord.ice
	"$(SLICE2CPP)" $(SLICE2CPPFLAGS) SubscriberRecord.ice

LLURecord.ice: ..\..\src\IceStorm\LLURecord.ice
	copy ..\..\src\IceStorm\LLURecord.ice .

LLURecord.h: LLURecord.ice
	"$(SLICE2CPP)" $(SLICE2CPPFLAGS) LLURecord.ice

LLUMap.h LLUMap.cpp: LLURecord.ice "$(SLICE2FREEZE)" "$(SLICEPARSERLIB)"
	del /q LLUMap.h LLUMap.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::LLUMap,string,IceStormElection::LogUpdate \
	LLUMap LLURecord.ice

SubscriberMap.h SubscriberMap.cpp: SubscriberRecord.ice "$(SLICE2FREEZE)" "$(SLICEPARSERLIB)"
	del /q SubscriberMap.h SubscriberMap.cpp
	$(SLICE2FREEZECMD) \
	--dict IceStorm::SubscriberMap,IceStorm::SubscriberRecordKey,IceStorm::SubscriberRecord,sort \
	SubscriberMap SubscriberRecord.ice

Clean::
	-del /q $(ICESTORMDB:.exe=.*)
	-del /q IceStormDB.res
	-del /q DBTypes.h DBTypes.cpp
	-del /q LLUMap.h LLUMap.cpp
	-del /q SubscriberMap.h SubscriberMap.cpp
	-del /q LLURecord.h LLURecord.cpp
	-del /q SubscriberRecord.h SubscriberRecord.cpp
	-del /q LLURecord.ice SubscriberRecord.ice

Install:: all
	copy $(ICESTORMDB) "$(install_bindir)"


!if "$(GENERATE_PDB)" == "yes"

Install:: all
	copy $(ICESTORMDB:.exe=.pdb) "$(install_bindir)"

!endif
