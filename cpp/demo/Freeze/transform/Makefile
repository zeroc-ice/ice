# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CREATE          = create
READ            = read
READNEW         = readnew
RECREATE        = recreate

TARGETS		= $(CREATE) $(READ) $(READNEW) $(RECREATE)

CREATE_OBJS     = ContactData.o Contacts.o create.o
READ_OBJS       = ContactData.o Contacts.o read.o
READNEW_OBJS    = NewContactData.o NewContacts.o readnew.o
RECREATE_OBJS   = NewContactData.o NewContacts.o recreate.o

OBJS		= $(CREATE_OBJS) $(READ_OBJS) $(READNEW_OBJS) $(RECREATE_OBJS)

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= ContactData.ice NewContactData.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)

$(CREATE): $(CREATE_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(CREATE_OBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS)

$(READ): $(READ_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(READ_OBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS)

$(READNEW): $(READNEW_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(READNEW_OBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS)

$(RECREATE): $(RECREATE_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(RECREATE_OBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS)


# The slice2freeze rules are structured like this to avoid issues with
# parallel make.
Contacts.h: Contacts.cpp
Contacts.cpp: ContactData.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f Contacts.h Contacts.cpp
	$(SLICE2FREEZE) -I. -I$(slicedir) --dict Demo::Contacts,string,Demo::ContactData,sort \
              --dict-index Demo::Contacts,phoneNumber,sort Contacts ContactData.ice

NewContacts.h: NewContacts.cpp
NewContacts.cpp: NewContactData.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f NewContacts.h NewContacts.cpp
	$(SLICE2FREEZE) -I. -I$(slicedir) --dict Demo::NewContacts,string,Demo::ContactData,sort \
              --dict-index Demo::NewContacts,phoneNumber,sort NewContacts NewContactData.ice

cleandb::
	-rm -rf db/* dbnew/*

clean::
	-rm -rf db/* dbnew/*
	-rm -f Contacts.h Contacts.cpp NewContacts.h NewContacts.cpp

include .depend
