# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_programs 	= publisher subscriber
$(test)_dependencies 	= IceStorm Ice TestCommon

$(test)_publisher_sources 	= Publisher.cpp Event.ice
$(test)_subscriber_sources 	= Subscriber.cpp Event.ice

tests += $(test)