# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
