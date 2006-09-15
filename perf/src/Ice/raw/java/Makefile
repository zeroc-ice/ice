# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

CLIENT		= Client.class
SERVER		= Server.class
TARGETS		= $(CLIENT) $(SERVER)

all: $(TARGETS)

$(CLIENT): Client.java
	rm -f $@
	javac $<

$(SERVER): Server.java
	rm -f $@
	javac $<

clean:
	rm -f $(TARGETS)
