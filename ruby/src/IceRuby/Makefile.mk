# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries := IceRuby

IceRuby_target		:= ruby-module
IceRuby_targetname	:= IceRuby
IceRuby_targetdir	:= $(lang_srcdir)/ruby
IceRuby_installdir	:= $(install_rubylibdir)
IceRuby_cppflags  	:= -I$(project) -I$(top_srcdir)/cpp/include -I$(top_srcdir)/cpp/include/generated $(ruby_cppflags)
IceRuby_ldflags		:= $(ruby_ldflags)
IceRuby_dependencies	:= IceSSL Ice Slice IceUtil

projects += $(project)
srcs:: $(project)