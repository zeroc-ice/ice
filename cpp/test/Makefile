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
		  Ice \
                  IceSSL \
		  IceXML \
		  IceStorm \
		  Yellow \
		  IcePack \
		  Freeze \
		  Glacier \
		  XMLTransform

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	done
