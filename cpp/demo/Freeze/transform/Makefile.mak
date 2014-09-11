# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

CREATE          = create.exe
READ            = read.exe
READNEW         = readnew.exe
RECREATE        = recreate.exe

TARGETS		= $(CREATE) $(READ) $(READNEW) $(RECREATE)

CREATE_OBJS     = ContactData.obj Contacts.obj create.obj
READ_OBJS       = ContactData.obj Contacts.obj read.obj
READNEW_OBJS    = NewContactData.obj NewContacts.obj readnew.obj
RECREATE_OBJS   = NewContactData.obj NewContacts.obj recreate.obj

SRCS		= $(OBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
CREATE_PDBFLAGS        = /pdb:$(CREATE:.exe=.pdb)
READ_PDBFLAGS          = /pdb:$(READ:.exe=.pdb)
READNEW_PDBFLAGS       = /pdb:$(READNEW:.exe=.pdb)
RECREATE_PDBFLAGS      = /pdb:$(RECREATE:.exe=.pdb)
!endif


$(CREATE): $(CREATE_OBJS)
	$(LINK) $(LD_EXEFLAGS) $(CREATE_PDBFLAGS) $(SETARGV) $(CREATE_OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) freeze$(LIBSUFFIX).lib
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(READ): $(READ_OBJS)
	$(LINK) $(LD_EXEFLAGS) $(READ_PDBFLAGS) $(SETARGV) $(READ_OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) freeze$(LIBSUFFIX).lib
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(READNEW): $(READNEW_OBJS)
	$(LINK) $(LD_EXEFLAGS) $(READNEW_PDBFLAGS) $(SETARGV) $(READNEW_OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) freeze$(LIBSUFFIX).lib
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(RECREATE): $(RECREATE_OBJS)
	$(LINK) $(LD_EXEFLAGS) $(RECREATE_PDBFLAGS) $(SETARGV) $(RECREATE_OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) freeze$(LIBSUFFIX).lib
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

Contacts.h Contacts.cpp: ContactData.ice "$(SLICE2FREEZE)" "$(SLICEPARSERLIB)"
	del /q Contacts.h Contacts.cpp
	"$(SLICE2FREEZE)" -I"$(slicedir)" --dict Demo::Contacts,string,Demo::ContactData,sort \
              --dict-index Demo::Contacts,phoneNumber,sort Contacts ContactData.ice

NewContacts.h NewContacts.cpp: NewContactData.ice "$(SLICE2FREEZE)" "$(SLICEPARSERLIB)"
	del /q NewContacts.h NewContacts.cpp
	"$(SLICE2FREEZE)" -I"$(slicedir)" --dict Demo::NewContacts,string,Demo::ContactData,sort \
              --dict-index Demo::NewContacts,phoneNumber,sort NewContacts NewContactData.ice

cleandb::
	-if exist db\__Freeze rmdir /q /s db\__Freeze
        -for %f in (db\*) do if not %f == db\.gitignore del /q %f
	-if exist dbnew\__Freeze rmdir /q /s dbnew\__Freeze
        -for %f in (dbnew\*) do if not %f == dbnew\.gitignore del /q %f

clean:: cleandb
	-del /q ContactData.h ContactData.cpp
        -del /q NewContactData.h NewContactData.cpp
        -del /q Contacts.h Contacts.cpp
        -del /q NewContacts.h NewContacts.cpp

!include .depend.mak
