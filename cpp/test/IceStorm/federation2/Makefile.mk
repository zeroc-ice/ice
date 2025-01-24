# Copyright (c) ZeroC, Inc.

$(project)_programs        = publisher subscriber
$(project)_dependencies    = IceStorm Ice TestCommon

$(project)_publisher_sources       = Publisher.cpp Event.ice
$(project)_subscriber_sources      = Subscriber.cpp Event.ice

$(project)_cleanfiles = db/* 0.db/* 1.db/* 2.db/* db2/* 0.db2/* 1.db2/* 2.db2/*

tests += $(project)
