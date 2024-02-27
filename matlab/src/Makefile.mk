#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_libraries    := IceMatlab

IceMatlab_target        := matlab-module
IceMatlab_targetname    := ice
IceMatlab_targetdir     := $(lang_srcdir)/lib
IceMatlab_cppflags      := -I$(project) \
    -I$(top_srcdir)/cpp/include \
    -I$(top_srcdir)/cpp/include/generated \
    -I$(top_srcdir)/cpp/src $(matlab_cppflags)

IceMatlab_system_libs   := $(matlab_ldflags) \
    -L$(top_srcdir)/cpp/lib/x86_64-linux-gnu \
    -Wl,-rpath=\$$ORIGIN \
    -lIce -lIceSSL -lIceDiscovery -lIceLocatorDiscovery

IceMatlab_dependencies  := IceDiscovery IceLocatorDiscovery IceSSL Ice

projects += $(project)
srcs:: $(project)
