# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

ifneq ($(filter debian ubuntu yocto,$(linux_id)),)

#
# Only build the IceBT plugin if pkg-config, BlueZ and D-Bus requirements are
# installed.
#
ifeq ($(shell pkg-config --exists bluez dbus-1 2> /dev/null && echo yes),yes)
$(project)_libraries    := IceBT

IceBT_targetdir         := $(libdir)
IceBT_dependencies      := Ice
IceBT_cppflags          := -DICEBT_API_EXPORTS $(shell pkg-config --cflags dbus-1)
IceBT_sliceflags        := --include-dir IceBT
IceBT_system_libs       = $(IceSSL_system_libs) $(shell pkg-config --libs dbus-1)

projects += $(project)
endif

endif
