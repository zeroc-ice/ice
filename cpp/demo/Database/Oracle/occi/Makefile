# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../../..

CLIENT		= client
SERVER		= server

TARGETS		= $(CLIENT) $(SERVER)

SLICE_SRCS	= HR.ice

OBJS		= HR.o

COBJS		= Client.o

SOBJS		= DbTypes.o \
                  DbTypesMap.o \
                  DeptFactoryI.o \
                  DeptI.o  \
                  EmpI.o  \
                  OCCIServantLocator.o \
                  Server.o \
                  Util.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I$(ORACLE_HOME)/rdbms/public $(CPPFLAGS)

ORACLE_LIBS     = -L$(ORACLE_HOME)/lib -locci -lclntsh

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(ORACLE_LIBS) $(LIBS)


DbTypes.h DbTypes.cpp DbTypesMap.h DbTypesMap.cpp DbTypesOut.typ: DbTypes.typ
	ott userid=scott/tiger@orcl code=cpp hfile=DbTypes.h cppfile=DbTypes.cpp \
            mapfile=DbTypesMap.cpp intype=DbTypes.typ outtype=DbTypesOut.typ \
            attraccess=private

clean::
	-rm -f DbTypes.h DbTypes.cpp DbTypesMap.h DbTypesMap.cpp DbTypesOut.typ

include .depend


