# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

include $(top_srcdir)/config/Make.rules

all::

install::
	$(call installdata,templates.xml,$(DESTDIR)$(install_configdir))
	$(call installdata,upgradeicegrid36.py,$(DESTDIR)$(install_configdir))
	$(call installdata,icegrid-slice.3.5.ice.gz,$(DESTDIR)$(install_configdir))
	$(call installdata,icegrid-slice.3.6.ice.gz,$(DESTDIR)$(install_configdir))

