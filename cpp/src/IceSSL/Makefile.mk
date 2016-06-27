# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries	:= IceSSL

IceSSL_targetdir	:= $(libdir)
IceSSL_dependencies	:= Ice
IceSSL_cppflags  	:= -DICE_SSL_API_EXPORTS
IceSSL_sliceflags	:= --include-dir IceSSL --dll-export ICE_SSL_API

projects += $(project)
