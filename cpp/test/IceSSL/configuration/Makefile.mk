# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_dependencies = TestCommon IceSSL Ice

#
# Disable var tracking assignments for Linux with this test
#
ifneq ($(linux_id),)
    $(test)_cppflags += $(if $(filter yes,$(OPTIMIZE)),-fno-var-tracking-assignments)
endif

# Need to load certificates with functions from src/IceSSL/Util.h
$(test)[iphoneos]_cppflags              := -Isrc
$(test)[iphonesimulator]_cppflags       := -Isrc

tests += $(test)
