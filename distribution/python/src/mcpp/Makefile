# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************
#
all: libmcpp.a

CFLAGS = -O2 -DMCPP_LIB=1 -DHAVE_CONFIG_H
OBJS = directive.o eval.o expand.o main.o mbchar.o support.o system.o

UNAME = $(shell uname)
ifneq ($(findstring MINGW,$(UNAME)),)
	UNAME 	:= MINGW
	CC	:= gcc
endif

ifeq ($(UNAME),Linux)
	CFLAGS += -fPIC
endif

config:
	@if ! test -f config.h; \
	then \
		cp config.h.$(UNAME) config.h ; \
	fi

libmcpp.a: config $(OBJS)
	ar rcs libmcpp.a $(OBJS)
	ranlib libmcpp.a

clean:
	rm -f $(OBJS)
	rm -f libmcpp.a
