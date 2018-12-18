# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_client_sources  = $(test-client-sources) \
                          Twoways.m \
                          TwowaysAMI.m \
                          Oneways.m \
                          OnewaysAMI.m \
                          BatchOneways.m \
                          BatchOnewaysAMI.m

tests += $(test)
