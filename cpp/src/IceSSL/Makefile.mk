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
IceSSL_dependencies	:= Ice
IceSSL_cppflags  	:= -DICESSL_API_EXPORTS
IceSSL_sliceflags	:= --include-dir IceSSL --dll-export ICESSL_API

IceSSL[iphoneos]_excludes		:= $(wildcard $(addprefix $(currentdir)/,*.cpp))
IceSSL[iphonesimulator]_excludes	= $(IceSSL[iphoneos]_excludes)

projects += $(project)
