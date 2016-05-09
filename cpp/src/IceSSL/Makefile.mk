# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries	:= IceSSL

IceSSL_targetdir	:= $(libdir)
IceSSL_dependencies	:= Ice IceUtil
IceSSL_cppflags		:= -DICESSL_API_EXPORTS
IceSSL_system_libs	:= $(SSL_OS_LIBS)
IceSSL_sliceflags	:= --include-dir IceSSL --dll-export ICESSL_API

projects += $(project)
