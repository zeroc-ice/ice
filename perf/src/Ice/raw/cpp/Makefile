# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

CLIENT		= client
SERVER		= server


TARGETS		= $(CLIENT) $(SERVER)

COBJS		= Client.o

SOBJS		= Server.o

SRCS		= $(SOBJS:.o=.cpp) $(COBJS:.o=.cpp)

CPPFLAGS	:=

all: $(CLIENT) $(SERVER)

$(CLIENT): $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(COBJS)

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(SOBJS)

clean:
	rm -f Client.o Server.o $(TARGETS)
