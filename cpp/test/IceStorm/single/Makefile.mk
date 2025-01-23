# Copyright (c) ZeroC, Inc.

$(project)_programs        = publisher subscriber
$(project)_dependencies    = IceStorm Ice TestCommon

$(project)_publisher_sources       = Publisher.cpp Single.ice
$(project)_subscriber_sources      = Subscriber.cpp Single.ice

tests += $(project)
