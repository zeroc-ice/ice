# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

TARGETS = Chat.js ChatSession.js

ifeq ($(OPTIMIZE),yes)
TARGETS	:= $(TARGETS) Client.min.js Client.min.js.gz
endif

include $(top_srcdir)/config/Make.rules.js

SLICE2JSFLAGS	:= $(SLICE2JSFLAGS) -I$(slicedir) -I.


ifeq ($(OPTIMIZE),yes)

CLOSUREFLAGS := $(CLOSUREFLAGS) --warning_level QUIET

Client.min.js Client.min.js.gz: $(libdir)/Ice.min.js $(libdir)/Glacier2.min.js Client.js Chat.js ChatSession.js
	@rm -f browser/Client.min.js browser/Client.min.js.gz
	java -jar $(CLOSURE_PATH)/compiler.jar $(CLOSUREFLAGS) --js $(libdir)/Ice.min.js $(libdir)/Glacier2.min.js \
	Chat.js ChatSession.js Client.js --js_output_file Client.min.js
	gzip -c9 Client.min.js > Client.min.js.gz
endif
