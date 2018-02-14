# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

!include $(top_srcdir)/config/Make.rules.mak

install::
	copy templates.xml "$(install_configdir)"
	copy upgradeicegrid36.py "$(install_configdir)"
	copy icegridregistry.cfg "$(install_configdir)"
	copy icegridnode.cfg "$(install_configdir)"
	copy glacier2router.cfg "$(install_configdir)"
	copy icegrid-slice.3.5.ice.gz "$(install_configdir)"
	copy icegrid-slice.3.6.ice.gz "$(install_configdir)"
	copy Ice.props "$(install_configdir)"
	copy Ice.Cpp.props "$(install_configdir)"
