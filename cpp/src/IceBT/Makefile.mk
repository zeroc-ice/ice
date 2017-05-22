# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

ifneq ($(filter debian ubuntu yocto,$(linux_id)),)

$(project)_libraries    := IceBT

IceBT_targetdir         := $(libdir)
IceBT_dependencies      := Ice
IceBT_cppflags          := -DICEBT_API_EXPORTS $(shell pkg-config --cflags dbus-1)
IceBT_sliceflags        := --include-dir IceBT

projects += $(project)

endif
