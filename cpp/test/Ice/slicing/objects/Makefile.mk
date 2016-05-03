# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_sliceflags		:= -I$(test)
$(test)_client_sources 		= $(test-client-sources) ClientPrivate.ice
$(test)_server_sources 		= $(test-server-sources) ServerPrivate.ice
$(test)_serveramd_sources 	= $(test-serveramd-sources) ServerPrivateAMD.ice

tests += $(test)