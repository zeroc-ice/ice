# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

include $(top_srcdir)/config/Make.rules

ifneq ($(findstring MINGW,$(UNAME)),)
    SUBDIRS	= IceUtil \
		  Slice \
		  slice2cpp \
		  Ice \
		  IceSSL \
		  IceDiscovery \
		  IceLocatorDiscovery
else
    SUBDIRS	= IceUtil \
		  Slice \
		  slice2cpp \
		  slice2cs \
		  slice2freeze \
		  slice2freezej \
		  slice2java \
		  slice2php \
		  slice2py \
		  slice2html \
		  slice2js

ifeq ($(UNAME),Darwin)
    SUBDIRS	:= $(SUBDIRS) slice2objc
endif

    SUBDIRS	:= $(SUBDIRS) \
		   Ice \
		   IceXML \
		   IceSSL \
		   IceDiscovery \
		   IceLocatorDiscovery \
		   Freeze \
		   FreezeScript \
		   IceBox \
		   Glacier2Lib \
		   Glacier2CryptPermissionsVerifier \
		   Glacier2 \
		   IcePatch2Lib \
		   IcePatch2 \
		   IceStormLib \
		   IceGridLib \
		   IceStorm \
		   IceGrid
endif

.PHONY: $(EVERYTHING) $(SUBDIRS)

#
# Dependencies for 'all' target when using -jx
#

Slice: IceUtil

slice2cpp slice2cs slice2freeze slice2freezej slice2java slice2js slice2php slice2py slice2html slice2objc: Slice

Ice: slice2cpp

IceXML IceSSL Freeze FreezeScript IceBox Glacier2Lib IcePatch2Lib IceStormLib IceGridLib: Ice

Freeze: slice2freeze

FreezeScript: Freeze

Glacier2 IceGridLib Glacier2CryptPermissionsVerifier: Glacier2Lib

IcePatch2: IcePatch2Lib

IceStorm: IceBox IceStormLib IceGridLib Glacier2Lib Freeze

IceGrid: IceGridLib IceStorm IcePatch2Lib Freeze IceSSL IceBox IceXML

IceDiscovery IceLocatorDiscovery: Ice

all:: $(SUBDIRS)

$(SUBDIRS):
	@echo "making all in $@"
	@$(MAKE) all --directory=$@

$(EVERYTHING_EXCEPT_ALL)::
	@for subdir in $(SUBDIRS); \
        do \
            if test -d $$subdir ; \
            then \
                echo "making $@ in $$subdir"; \
                ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
            fi; \
        done
