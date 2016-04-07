# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT1		= $(call mktestname,client1)
CLIENT2		= $(call mktestname,client2)
CLIENT3		= $(call mktestname,client3)
CLIENT4		= $(call mktestname,client4)
CLIENT5		= $(call mktestname,client5)

TARGETS		= $(CLIENT1) $(CLIENT2) $(CLIENT3) $(CLIENT4) $(CLIENT5)

C1OBJS		= Client1.o
C2OBJS		= Client2.o
C3OBJS		= Client3.o
C4OBJS		= Client4.o
C5OBJS		= Client5.o

OBJS		= $(C1OBJS) \
		  $(C2OBJS) \
		  $(C3OBJS) \
		  $(C4OBJS) \
		  $(C5OBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

LINKWITH	:= $(LIBS) $(ICONV_LIBS)

$(CLIENT1): $(C1OBJS)
	rm -f $@
	$(call mktest,$@,$(C1OBJS),$(LINKWITH))

$(CLIENT2): $(C2OBJS)
	rm -f $@
	$(call mktest,$@,$(C2OBJS),$(LINKWITH))

$(CLIENT3): $(C3OBJS)
	rm -f $@
	$(call mktest,$@,$(C3OBJS),$(LINKWITH))

$(CLIENT4): $(C4OBJS)
	rm -f $@
	$(call mktest,$@,$(C4OBJS),$(LINKWITH))

$(CLIENT5): $(C5OBJS)
	rm -f $@
	$(call mktest,$@,$(C5OBJS),$(LINKWITH))
