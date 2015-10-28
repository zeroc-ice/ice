# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

ICEGRIDDB	= $(top_srcdir)\bin\icegriddb35.exe

TARGETS         = $(ICEGRIDDB)

OBJS		= IceGridDB.obj \
		  DBTypes.obj \
		  StringApplicationInfoDict.obj \
		  IdentityObjectInfoDict.obj \
		  StringAdapterInfoDict.obj \
		  SerialsDict.obj

SRCS            = $(OBJS:.obj=.cpp)

SDIR		= $(slicedir)\IceGrid

SLICE2FREEZECMD = "$(SLICE2FREEZE)" --ice $(ICECPPFLAGS)

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
SLICE2CPPFLAGS	= --ice $(SLICE2CPPFLAGS)
LINKWITH 	= $(LIBS) icegrid$(LIBSUFFIX).lib freeze$(LIBSUFFIX).lib glacier2$(LIBSUFFIX).lib icepatch2$(LIBSUFFIX).lib

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS       	= /pdb:$(ICEGRIDDB:.exe=.pdb)
!endif

RES_FILE       	= IceGridDB.res

$(ICEGRIDDB): $(OBJS) IceGridDB.res
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) $(OBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) $(RES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) &&\
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

StringApplicationInfoDict.h StringApplicationInfoDict.cpp: "$(SDIR)\Admin.ice" "$(SLICE2FREEZE)" "$(SLICEPARSERLIB)"
	del /q StringApplicationInfoDict.h StringApplicationInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringApplicationInfoDict,string,IceGrid::ApplicationInfo \
	StringApplicationInfoDict "$(SDIR)\Admin.ice"

IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp: "$(slicedir)\Ice\Identity.ice" "$(SDIR)\Admin.ice" "$(SLICE2FREEZE)" "$(SLICEPARSERLIB)"
	del /q IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::IdentityObjectInfoDict,Ice::Identity,IceGrid::ObjectInfo \
	--dict-index IceGrid::IdentityObjectInfoDict,type \
	IdentityObjectInfoDict "$(slicedir)\Ice\Identity.ice" "$(SDIR)\Admin.ice"

StringAdapterInfoDict.h StringAdapterInfoDict.cpp: "$(SDIR)\Admin.ice" "$(SLICE2FREEZE)" "$(SLICEPARSERLIB)"
	del /q StringAdapterInfoDict.h StringAdapterInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringAdapterInfoDict,string,IceGrid::AdapterInfo \
	--dict-index IceGrid::StringAdapterInfoDict,replicaGroupId StringAdapterInfoDict "$(SDIR)\Admin.ice"

SerialsDict.h SerialsDict.cpp: "$(SLICE2FREEZE)" "$(SLICEPARSERLIB)"
	del /q SerialsDict.h SerialsDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::SerialsDict,string,long SerialsDict

clean::
	-del /q $(ICEGRIDDB:.exe=.*)
	-del /q IceGridDB.res
	-del /q DBTypes.h DBTypes.cpp
	-del /q StringApplicationInfoDict.h StringApplicationInfoDict.cpp
	-del /q StringAdapterInfoDict.h StringAdapterInfoDict.cpp
	-del /q IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp
	-del /q SerialsDict.h SerialsDict.cpp

install:: all
	copy $(ICEGRIDDB) "$(install_bindir)"


!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(ICEGRIDDB:.exe=.pdb) "$(install_bindir)"

!endif

!include .depend.mak
