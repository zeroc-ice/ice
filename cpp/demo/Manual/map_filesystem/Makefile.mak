# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

MAXWARN         = yes

CLIENT		= client.exe
SERVER		= server.exe

TARGETS		= $(CLIENT) $(SERVER)

SLICE_OBJS	= .\Filesystem.obj \
		  .\FilesystemDB.obj

BISON_FLEX_OBJS = .\Grammar.obj \
                  .\Scanner.obj

COBJS		= .\Filesystem.obj \
		  .\Client.obj \
		  .\Parser.obj \
		  $(BISON_FLEX_OBJS)

SOBJS		= $(SLICE_OBJS) \
		  .\FilesystemI.obj \
		  .\Server.obj \
		  .\IdentityDirectoryEntryMap.obj \
		  .\IdentityFileEntryMap.obj

OBJS		= $(COBJS) \
		  $(SOBJS)

all:: IdentityDirectoryEntryMap.cpp IdentityDirectoryEntryMap.h IdentityFileEntryMap.cpp IdentityFileEntryMap.h

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
SLICE2CPPFLAGS	= -I. $(SLICE2CPPFLAGS)

!if "$(GENERATE_PDB)" == "yes"
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
!endif

$(CLIENT): $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(SETARGV) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVER): $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) 
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

IdentityFileEntryMap.h: IdentityFileEntryMap.cpp
IdentityFileEntryMap.cpp: FilesystemDB.ice Filesystem.ice "$(SLICE2FREEZE)" "$(SLICEPARSERLIB)"
	del /q IdentityFileEntryMap.h IdentityFileEntryMap.cpp
	"$(SLICE2FREEZE)" -I"$(slicedir)" -I. --ice \
                --dict FilesystemDB::IdentityFileEntryMap,Ice::Identity,FilesystemDB::FileEntry \
		IdentityFileEntryMap FilesystemDB.ice "$(slicedir)/Ice/Identity.ice"

IdentityDirectoryEntryMap.h: IdentityDirectoryEntryMap.cpp
IdentityDirectoryEntryMap.cpp: FilesystemDB.ice Filesystem.ice "$(SLICE2FREEZE)" "$(SLICEPARSERLIB)"
	del /q IdentityDirectoryEntryMap.h IdentityDirectoryEntryMap.cpp
	"$(SLICE2FREEZE)" -I"$(slicedir)" -I. --ice \
                --dict FilesystemDB::IdentityDirectoryEntryMap,Ice::Identity,FilesystemDB::DirectoryEntry \
		IdentityDirectoryEntryMap FilesystemDB.ice "$(slicedir)/Ice/Identity.ice"

clean::
	-del /q Filesystem.cpp Filesystem.h
	-del /q FilesystemDB.cpp FilesystemDB.h
	-del /q IdentityFileEntryMap.cpp IdentityFileEntryMap.h
	-del /q IdentityDirectoryEntryMap.cpp IdentityDirectoryEntryMap.h

clean::
	-if exist db\__Freeze rmdir /q /s db\__Freeze
	-for %f in (db\*) do if not %f == db\.gitignore del /q %f
