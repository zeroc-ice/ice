# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries    := IceBox
$(project)_programs     := icebox iceboxadmin
$(project)_dependencies := Ice
$(project)_sliceflags   := --include-dir IceBox

IceBox_targetdir        := $(libdir)
IceBox_sources          := $(slicedir)/IceBox/IceBox.ice $(currentdir)/Exception.cpp

icebox_targetdir        := $(bindir)
icebox_sources          := $(currentdir)/Service.cpp $(currentdir)/ServiceManagerI.cpp
icebox_dependencies     := IceBox

iceboxadmin_targetdir   := $(bindir)
iceboxadmin_sources     := $(currentdir)/Admin.cpp
iceboxadmin_dependencies:= IceBox

projects += $(project)
