# **********************************************************************
#
# Copyright (c) 2001
# MutableRealms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

top_srcdir	= ..

include $(top_srcdir)/config/Make.rules

SUBDIRS		= IceUtil \
		  Slice \
		  slice2cpp \
		  slice2freeze \
		  slice2docbook \
		  slice2java \
		  slice2xsd \
		  Ice \
		  IceXML \
		  Freeze \
		  IceStorm \
		  IcePack \
		  Glacier

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	done
