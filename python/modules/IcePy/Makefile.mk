# Copyright (c) ZeroC, Inc.

$(project)_libraries    := IcePy

IcePy_target            := python-module
IcePy_targetname        := IcePy
IcePy_targetdir         := $(lang_srcdir)/python
IcePy_installdir        := $(install_pythondir)
IcePy_cppflags          := $(ice_cpp_cppflags) -I$(top_srcdir)/cpp/src $(python_cppflags)
IcePy_dependencies      := IceDiscovery IceLocatorDiscovery Ice
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
