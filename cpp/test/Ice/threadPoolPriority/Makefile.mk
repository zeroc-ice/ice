# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_programs := client server servercustom

$(test)_servercustom_sources 	:= ServerCustomThreadPool.cpp Test.ice TestI.cpp

tests += $(test)