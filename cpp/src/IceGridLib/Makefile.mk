# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries	:= IceGrid

IceGrid_targetdir	:= $(libdir)
IceGrid_dependencies	:= Glacier2 Ice
IceGrid_sliceflags	:= --include-dir IceGrid --dll-export ICE_GRID_API

projects += $(project)
