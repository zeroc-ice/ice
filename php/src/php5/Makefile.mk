# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries := IcePHP

IcePHP_target		:= php-module
IcePHP_targetname	:= IcePHP
IcePHP_targetdir	:= $(lang_srcdir)/lib
IcePHP_installdir	:= $(install_phplibdir)
IcePHP_cppflags  	:= -I$(project) -I$(top_srcdir)/cpp/include -I$(top_srcdir)/cpp/include/generated $(php_cppflags)
IcePHP_ldflags		:= $(php_ldflags)
IcePHP_dependencies	:= IceDiscovery IceLocatorDiscovery IceSSL Ice Slice IceUtil

projects += $(project)
srcs:: $(project)