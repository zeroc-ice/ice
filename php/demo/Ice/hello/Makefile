# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

SLICE_SRCS	= Hello.ice

include $(top_srcdir)/config/Make.rules.php

SRCS	= Hello.php

all:: $(SRCS)

%.php: %.ice
	$(SLICE2PHP) $(SLICE2PHPFLAGS) $<

clean::
	rm -f $(SRCS)


include .depend
