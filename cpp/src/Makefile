# **********************************************************************
#
# Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

include $(top_srcdir)/config/Make.rules

SUBDIRS		= IceUtil \
		  Slice \
		  slice2cpp \
		  slice2cs \
		  slice2freeze \
		  slice2freezej \
		  slice2java \
		  slice2php \
		  slice2py \
		  slice2rb \
		  slice2html \
		  Ice \
		  IceXML \
		  IceSSL \
		  Freeze \
		  FreezeScript \
		  IceBox \
		  Glacier2Lib \
		  Glacier2 \
		  IceDB \
		  IcePatch2Lib \
		  IcePatch2 \
		  IceStormLib \
		  IceGridLib \
		  IceStorm \
		  IceGrid \
		  ca

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    if test -d $$subdir ; \
	    then \
		echo "making $@ in $$subdir"; \
		( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	    fi; \
	done
