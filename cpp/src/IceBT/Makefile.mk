# Copyright (c) ZeroC, Inc.

#
# Only build the IceBT plugin if Bluetooth system libraries are available (Bluez/DBus)
#
ifneq ($(IceBT_system_libs),)
$(project)_libraries    := IceBT

IceBT_targetdir         := $(libdir)
IceBT_dependencies      := Ice
IceBT_cppflags          := -DICEBT_API_EXPORTS $(api_exports_cppflags) $(shell pkg-config --cflags dbus-1)

projects += $(project)
endif
