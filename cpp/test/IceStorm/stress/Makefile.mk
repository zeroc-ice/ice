#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(test)_programs        = publisher subscriber
$(test)_dependencies    = IceStorm Ice TestCommon

$(test)_publisher_sources       = Publisher.cpp Event.ice
$(test)_subscriber_sources      = Subscriber.cpp Event.ice

tests += $(test)
