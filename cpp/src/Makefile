# **********************************************************************
#
# Copyright (c) 2001
# ZeroC, Inc.
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
		  slice2freezej \
		  slice2docbook \
		  slice2java \
		  slice2xsd \
		  slice2wsdl \
		  Ice \
		  IceSSL \
		  IceXML \
		  Freeze \
		  IceBox \
		  IceStorm \
		  Yellow \
		  IcePack \
		  Glacier \
		  IcePatch \
		  XMLTransform

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	done
