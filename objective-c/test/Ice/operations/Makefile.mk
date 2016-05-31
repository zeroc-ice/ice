# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_client_sources 	= $(test-client-sources) \
			  Twoways.m \
			  TwowaysNewAMI.m \
			  Oneways.m \
			  OnewaysNewAMI.m \
			  BatchOneways.m \
			  BatchOnewaysAMI.m

tests += $(test)
