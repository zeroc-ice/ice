# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries		= Glacier2ObjC

Glacier2ObjC_targetdir		:= $(libdir)
Glacier2ObjC_sliceflags		:= --include-dir objc/Glacier2
Glacier2ObjC_dependencies 	:= IceObjC
Glacier2ObjC_slicedir		:= $(slicedir)/Glacier2
Glacier2ObjC_includedir		:= $(includedir)/objc/Glacier2

Glacier2ObjC_install:: $(install_includedir)/objc/Glacier2.h

projects += $(project)
