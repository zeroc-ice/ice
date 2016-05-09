# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries 	= IceUtil

IceUtil_targetdir		:= $(libdir)
IceUtil_cppflags[shared]	:= -DICE_UTIL_API_EXPORTS
IceUtil_system_libs		:= $(ICEUTIL_OS_LIBS)

# Always enable the static configuration for the IceUtil library
IceUtil_always_enable_configs := static

ifeq ($(DEFAULT_MUTEX_PROTOCOL), PrioInherit)
    IceUtil_cppflags += -DICE_PRIO_INHERIT
endif

ifeq ($(libbacktrace),yes)
    IceUtil_cppflags += -DICE_LIBBACKTRACE
endif

projects += $(project)
