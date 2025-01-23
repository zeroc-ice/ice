# Copyright (c) ZeroC, Inc.

$(project)_programs        = publisher subscriber control
$(project)_dependencies    = IceStorm Ice TestCommon

$(project)_publisher_sources       = Publisher.cpp Single.ice Controller.ice
$(project)_subscriber_sources      = Subscriber.cpp Single.ice Controller.ice
$(project)_control_sources         = Control.cpp Controller.ice

$(project)_cleanfiles = 0.db/* 1.db/* 2.db/*

tests += $(project)
