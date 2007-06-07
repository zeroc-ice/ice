# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

!include $(top_srcdir)/config/Make.rules.mak

install::
	@if not exist $(prefix)\config \
	    @echo "Creating $(prefix)\config..." && \
	    mkdir $(prefix)\config
	copy Make.rules.mak $(prefix)\config
	copy Make.rules.bcc $(prefix)\config
	copy templates.xml $(prefix)\config
	copy convertssl.py $(prefix)\config
	copy upgradeicegrid.py $(prefix)\config
	copy upgradeicestorm.py $(prefix)\config
	copy icegridregistry.cfg $(prefix)\config
	copy icegridnode.cfg $(prefix)\config
	copy glacier2router.cfg $(prefix)\config
	copy icegrid-slice.3.1.ice.gz $(prefix)\config
