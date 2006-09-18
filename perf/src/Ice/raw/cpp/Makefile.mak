# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

CLIENT		= client.exe
SERVER		= server.exe


TARGETS		= $(CLIENT) $(SERVER)

COBJS		= Client.obj

SOBJS		= Server.obj

#CPPFLAGS	:=
CPPFLAGS	= /GX /O

all: $(CLIENT) $(SERVER)

$(CLIENT): $(COBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(COBJS) wsock32.lib

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SOBJS) wsock32.lib

clean:
	rm -f Client.o Server.o $(TARGETS)
