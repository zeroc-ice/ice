# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

SLICE_SRCS	= Test.ice \
		  CTypes.ice

include $(top_srcdir)/config/Make.rules.php

SRCS		= Test.php CTypes.php

all:: $(SRCS)

%.php: %.ice
	$(SLICE2PHP) $(SLICE2PHPFLAGS) --checksum $<

clean::
	rm -f $(SRCS)

include .depend
