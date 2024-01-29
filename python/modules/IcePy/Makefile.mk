#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_libraries    := IcePy

IcePy_target            := python-module
IcePy_targetname        := IcePy
IcePy_targetdir         := $(lang_srcdir)/python
IcePy_installdir        := $(install_pythondir)
IcePy_cppflags          := -I$(project) $(ice_cpp_cppflags) -I$(top_srcdir)/cpp/src -I$(top_srcdir)/cpp/src/slice2py $(python_cppflags) \
    -DICE_CPP11_MAPPING
IcePy_dependencies      := IceDiscovery++11 IceLocatorDiscovery++11 IceSSL++11 Ice++11
# TODO temporary ++11 dependencies are not linked
IcePy_system_libs      := $(python_ldflags) \
    -L$(top_srcdir)/cpp/lib/x86_64-linux-gnu \
    -L$(top_srcdir)/cpp/lib \
    -lIce++11 -lIceSSL++11 -lIceDiscovery++11 -lIceLocatorDiscovery++11
IcePy_libs              := mcpp
IcePy_extra_sources     := $(wildcard $(top_srcdir)/cpp/src/Slice/*.cpp) \
                           $(top_srcdir)/cpp/src/slice2py/PythonUtil.cpp \
                           $(top_srcdir)/cpp/src/slice2py/Python.cpp     \

#
# On the default platform, always write the module in the python directory.
#
$(foreach p,$(supported-platforms),$(eval $$p_targetdir[IcePy] := /$$p))
$(firstword $(supported-platforms))_targetdir[IcePy] :=
$(firstword $(supported-platforms))_installdir[IcePy] :=

projects += $(project)
srcs:: $(project)
