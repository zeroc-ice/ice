# **********************************************************************
#
# Copyright (c) 2002
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
#
# **********************************************************************

top_srcdir	= ..

include $(top_srcdir)/config/Make.rules

SUBDIRS		= IceUtil \
		  icecpp \
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
