# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_client_sources = Client.cpp Test.ice TestI.cpp MyObjectI.cpp InterceptorI.cpp AMDInterceptorI.cpp

tests += $(test)