# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

CLIENT		= Client.exe
SERVER		= Server.exe

TARGETS		= $(CLIENT) $(SERVER)

CSFLAGS	:= -optimize+ -unsafe

all: $(TARGETS)

$(CLIENT): Client.cs
	rm -f $@
	mcs $(CSFLAGS) $<

$(SERVER): Server.cs
	rm -f $@
	mcs $(CSFLAGS) $<

clean:
	rm -f $(TARGETS)
