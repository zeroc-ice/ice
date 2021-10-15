#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_libraries    := IceMatlab

IceMatlab_target	:= matlab-module
IceMatlab_targetname	:= ice
IceMatlab_targetdir	:= $(lang_srcdir)/lib
IceMatlab_cppflags	:= -I$(project) -I$(top_srcdir)/cpp/src $(matlab_cppflags)
IceMatlab_system_libs 	:= $(matlab_ldflags) -lIce++11 -lIceSSL++11 -lIceDiscovery++11 -lIceLocatorDiscovery++11
IceMatlab_dependencies  := IceDiscovery++11 IceLocatorDiscovery++11 IceSSL++11 Ice++11

projects += $(project)
srcs:: $(project)
