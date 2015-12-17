# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

include $(top_srcdir)/config/Make.rules
ifeq ($(CPP11_MAPPING),yes)
SUBDIRS		= IceUtil \
		  Slice \
		  Ice
else


SUBDIRS		= Common \
		  IceUtil \
		  Slice \
		  Ice \
		  IceSSL \
		  IceDiscovery

ifeq ($(findstring MINGW,$(UNAME)),)
SUBDIRS		:= $(SUBDIRS) \
		   IceBox \
		   IceStorm \
		   Glacier2 \
		   IceGrid
endif
endif

.PHONY: $(EVERYTHING) $(SUBDIRS)

all:: $(SUBDIRS)

$(SUBDIRS):
	@echo "making all in $@"
	@$(MAKE) all --directory=$@

$(EVERYTHING_EXCEPT_ALL)::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	done
