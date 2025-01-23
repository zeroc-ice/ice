# Copyright (c) ZeroC, Inc.

$(project)_programs        = publisher subscriber sub
$(project)_dependencies    = IceStorm Ice TestCommon

$(project)_publisher_sources       = Publisher.cpp Single.ice
$(project)_subscriber_sources      = Subscriber.cpp Single.ice
$(project)_sub_sources             = Sub.cpp Single.ice

$(project)_cleanfiles = 0.db/* 1.db/* 2.db/*

tests += $(project)
