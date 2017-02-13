# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries		= IceStormObjC

IceStormObjC_targetdir		:= $(libdir)
IceStormObjC_sliceflags		:= --include-dir objc/IceStorm
IceStormObjC_dependencies 	:= IceObjC
IceStormObjC_slicedir		:= $(slicedir)/IceStorm
IceStormObjC_includedir		:= $(includedir)/objc/IceStorm

IceStormObjC_install:: $(install_includedir)/objc/IceStorm.h

projects += $(project)
