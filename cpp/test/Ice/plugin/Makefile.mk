# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_libraries 	:= $(test)_TestPlugin

$(test)_TestPlugin_sources := Plugin.cpp

tests += $(test)
