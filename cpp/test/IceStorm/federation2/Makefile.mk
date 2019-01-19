#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(test)_programs        = publisher subscriber
$(test)_dependencies    = IceStorm Ice TestCommon

$(test)_publisher_sources       = Publisher.cpp Event.ice
$(test)_subscriber_sources      = Subscriber.cpp Event.ice

$(test)_cleanfiles = db/* 0.db/* 1.db/* 2.db/* db2/* 0.db2/* 1.db2/* 2.db2/*

tests += $(test)
