# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

ifneq ($(filter debian ubuntu,$(linux_id)),)

$(project)_libraries	:= IceBT

IceBT_targetdir		:= $(libdir)
IceBT_dependencies	:= IceXML Ice IceUtil
IceBT_cppflags  	:= -DICEBT_API_EXPORTS `pkg-config --cflags dbus-1`
IceBT_system_libs	:= $(SSL_OS_LIBS) `pkg-config --libs dbus-1`
IceBT_sliceflags	:= --include-dir IceBT --dll-export ICESSL_API

projects += $(project)

endif
