# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries 	= IceUtil

IceUtil_targetdir	:= $(libdir)
IceUtil_system_libs     := $(ICEUTIL_OS_LIBS)

# Always enable the static configuration for the IceUtil library
IceUtil_always_enable_configs := static

ifeq ($(libbacktrace),yes)
    IceUtil_cppflags += -DICE_LIBBACKTRACE
endif

projects += $(project)
