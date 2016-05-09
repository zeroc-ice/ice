# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries	:= Glacier2

Glacier2_targetdir		:= $(libdir)
Glacier2_dependencies		:= Ice IceUtil
Glacier2_cppflags	 	:= -DGLACIER2_API_EXPORTS
Glacier2_sliceflags		:= --include-dir Glacier2 --dll-export GLACIER2_API

projects += $(project)