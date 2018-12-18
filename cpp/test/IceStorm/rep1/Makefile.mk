# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_programs        = publisher subscriber sub
$(test)_dependencies    = IceStorm Ice TestCommon

$(test)_publisher_sources       = Publisher.cpp Single.ice
$(test)_subscriber_sources      = Subscriber.cpp Single.ice
$(test)_sub_sources             = Sub.cpp Single.ice

$(test)_cleanfiles = 0.db/* 1.db/* 2.db/*

tests += $(test)
