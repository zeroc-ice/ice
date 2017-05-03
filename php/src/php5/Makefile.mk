# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries := IcePHP

IcePHP_target           := php-module
IcePHP_targetname       := IcePHP
IcePHP_targetdir        := lib
IcePHP_installdir       := $(install_phplibdir)
IcePHP_cppflags         := -I$(project) -I$(top_srcdir)/cpp/src $(php_cpp_includes) $(php_cppflags)
IcePHP_ldflags          := $(php_ldflags)
IcePHP_dependencies     := IceDiscovery IceLocatorDiscovery IceSSL Ice
IcePHP_extra_sources    := $(top_srcdir)/cpp/src/Slice/PHPUtil.cpp

projects += $(project)
srcs:: $(project)
