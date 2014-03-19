# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
	javac -source 1.4 $<

$(SERVER): Server.java
	rm -f $@
	javac -source 1.4 $<

clean:
	rm -f $(TARGETS)
