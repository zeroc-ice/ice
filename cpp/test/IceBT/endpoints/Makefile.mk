# Copyright (c) ZeroC, Inc.

#
# The IceBT plug-in is only built when the Bluetooth system libraries (BlueZ/DBus) are available
# (see config/Make.rules.Linux). Only build this test when the plug-in is available, so it can link
# against IceBT and use IceBT::btPluginFactory.
#
ifneq ($(IceBT_system_libs),)

$(project)_dependencies := TestCommon Ice IceBT

tests += $(project)

endif
