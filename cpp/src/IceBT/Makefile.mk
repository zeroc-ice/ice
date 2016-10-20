# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

ifneq ($(filter debian ubuntu,$(linux_id)),)

$(project)_libraries	:= IceBT

IceBT_targetdir		:= $(libdir)
IceBT_dependencies	:= Ice
IceBT_cppflags  	:= -DICE_BT_API_EXPORTS $(shell pkg-config --cflags dbus-1)
IceBT_system_libs	= $(IceSSL_system_libs) $(shell pkg-config --libs dbus-1)
IceBT_sliceflags	:= --include-dir IceBT --dll-export ICE_BT_API

projects += $(project)

endif
