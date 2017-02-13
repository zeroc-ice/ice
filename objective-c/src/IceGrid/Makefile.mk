# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries		= IceGridObjC

IceGridObjC_targetdir		:= $(libdir)
IceGridObjC_sliceflags		:= --include-dir objc/IceGrid
IceGridObjC_dependencies 	:= IceObjC Glacier2ObjC
IceGridObjC_slicedir		:= $(slicedir)/IceGrid
IceGridObjC_includedir		:= $(includedir)/objc/IceGrid

IceGridObjC_install:: $(install_includedir)/objc/IceGrid.h

projects += $(project)
