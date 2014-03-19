# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice Protobuf is licensed to you under the terms
# described in the ICE_PROTOBUF_LICENSE file included in this
# distribution.
#
# **********************************************************************

#
# If protobuf is not installed in a standard location where the
# compiler can find it, set PROTOBUF_HOME to the protobuf installation
# directory.
#
PROTOBUF_HOME   ?= /usr/local

top_srcdir	= ../..

CLIENT		= client
SERVER		= server

TARGETS		= $(CLIENT) $(SERVER)

OBJS		= Hello.o \
		  Person.pb.o

COBJS		= Client.o

SOBJS		= HelloI.o \
		  Server.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= Hello.ice
PROTO_SRCS	= Person.proto

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= $(ICE_FLAGS) -I$(PROTOBUF_HOME)/include -I. $(CPPFLAGS)
LIBS		:= $(ICE_LIB_DIR) $(LIBS) -L$(PROTOBUF_HOME)/lib -lprotobuf

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(LIBS)

%..pb.h %.pb.cpp: %.proto
	rm -f $(*F).pb.h $(*F).pb.cpp
	$(PROTOBUF_HOME)/bin/protoc --cpp_out=. $(*F).proto
	mv $(*F).pb.cc $(*F).pb.cpp

depend::
	for i in $(PROTO_SRCS); do \
		echo "$${i%.proto}.pb.cpp: $$i" >> .depend; \
	done

clean::
	rm -f $(addsuffix .pb.cpp, $(basename $(notdir $(PROTO_SRCS))))
	rm -f $(addsuffix .pb.h, $(basename $(notdir $(PROTO_SRCS))))

include .depend
