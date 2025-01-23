# Copyright (c) ZeroC, Inc.

$(project)_libraries    := IceBox
$(project)_programs     := icebox iceboxadmin
$(project)_dependencies := Ice
$(project)_sliceflags   := --include-dir IceBox
$(project)_cppflags     := $(api_exports_cppflags)

IceBox_targetdir        := $(libdir)
IceBox_sources          := $(slicedir)/IceBox/ServiceManager.ice $(currentdir)/Service.cpp

icebox_targetdir        := $(bindir)
icebox_sources          := $(currentdir)/IceBoxService.cpp $(currentdir)/ServiceManagerI.cpp
icebox_dependencies     := IceBox

iceboxadmin_targetdir   := $(bindir)
iceboxadmin_sources     := $(currentdir)/Admin.cpp
iceboxadmin_dependencies:= IceBox

projects += $(project)
