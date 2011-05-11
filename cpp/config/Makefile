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

all::
	echo ""

install::
	$(call installdata,templates.xml,$(install_configdir))
	$(call installdata,convertssl.py,$(install_configdir))
	$(call installdata,upgradeicegrid.py,$(install_configdir))
	$(call installdata,icegrid-slice.3.1.ice.gz,$(install_configdir))
	$(call installdata,icegrid-slice.3.2.ice.gz,$(install_configdir))
	$(call installdata,icegrid-slice.3.3.ice.gz,$(install_configdir))

