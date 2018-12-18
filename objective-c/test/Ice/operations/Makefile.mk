# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
