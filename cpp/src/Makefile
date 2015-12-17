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
    SUBDIRS	= IceUtil \
		  Slice \
		  slice2cpp \
		  Ice \
		  IceSSL \
		  IceDiscovery \
		  Glacier2Lib \
		  IceStormLib \
		  IceGridLib
else

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
		  slice2java \
		  slice2php \
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
		   IceBox \
		   Glacier2Lib \
		   Glacier2CryptPermissionsVerifier \
		   Glacier2 \
		   IcePatch2Lib \
		   IcePatch2 \
		   IceStormLib \
		   IceGridLib \
		   IceDB \
		   IceStorm \
		   IceGrid

ifneq ($(findstring Ubuntu,$(shell uname -a)),)
    SUBDIRS	:= $(SUBDIRS) IceBT
endif

endif
endif

.PHONY: $(EVERYTHING) $(SUBDIRS)

#
# Dependencies for 'all' target when using -jx
#

Slice: IceUtil

slice2cpp slice2cs slice2java slice2js slice2php slice2html slice2objc: Slice

Ice: slice2cpp

IceXML IceSSL IceBox Glacier2Lib IcePatch2Lib IceStormLib IceGridLib IceBT: Ice

Glacier2 IceGridLib Glacier2CryptPermissionsVerifier: Glacier2Lib

IcePatch2: IcePatch2Lib

IceStorm: IceBox IceStormLib IceGridLib Glacier2Lib IceDB

IceGrid: IceGridLib IceStorm IcePatch2Lib IceDB IceSSL IceBox IceXML

IceDiscovery IceLocatorDiscovery: Ice

IceDB: Ice

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
