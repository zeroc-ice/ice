# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries    := IcePy

IcePy_target            := python-module
IcePy_targetname        := IcePy
IcePy_targetdir         := $(lang_srcdir)/python
IcePy_installdir        := $(install_pythondir)
IcePy_cppflags          := -I$(project) $(ice_cpp_cppflags) -I$(top_srcdir)/cpp/src $(python_cppflags)
IcePy_system_libs       := $(python_ldflags)
IcePy_dependencies      := IceDiscovery IceLocatorDiscovery IceSSL Ice
IcePy_libs              := mcpp
IcePy_extra_sources     := $(filter-out %Util.cpp %Ruby.cpp,\
                           $(wildcard $(top_srcdir)/cpp/src/Slice/*.cpp)) \
                           $(top_srcdir)/cpp/src/Slice/SliceUtil.cpp \
                           $(top_srcdir)/cpp/src/Slice/PythonUtil.cpp \
                           $(top_srcdir)/cpp/src/Slice/StringLiteralUtil.cpp \

#
# On the default platform, always write the module in the python directory.
#
$(foreach p,$(supported-platforms),$(eval $$p_targetdir[IcePy] := /$$p))
$(firstword $(supported-platforms))_targetdir[IcePy] :=
$(firstword $(supported-platforms))_installdir[IcePy] :=

projects += $(project)
srcs:: $(project)
