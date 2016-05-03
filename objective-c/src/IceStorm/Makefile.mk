# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries		= IceStormObjC

IceStormObjC_targetdir		:= $(libdir)
IceStormObjC_sliceflags		:= --include-dir objc/IceStorm --dll-export ICESTORM_API
IceStormObjC_dependencies 	:= IceObjC
IceStormObjC_slicedir		:= $(slicedir)/IceStorm
IceStormObjC_includedir		:= $(includedir)/objc/IceStorm

IceStormObjC_install:: $(install_includedir)/objc/IceStorm.h

projects += $(project)
