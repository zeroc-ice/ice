# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries	= IceIAP

IceIAP_configs		:= xcodesdk cpp11-xcodesdk
IceIAP_platforms	:= iphoneos iphonesimulator

IceIAP_targetdir	:= $(libdir)
IceIAP_dependencies	:= Ice
IceIAP_sliceflags	:= --include-dir IceIAP
IceIAP_cppflags  	:= -DICEIAP_API_EXPORTS

projects += $(project)
