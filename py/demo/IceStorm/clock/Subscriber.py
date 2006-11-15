#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, Ice, IceStorm

Ice.loadSlice('Clock.ice')
import Demo

class ClockI(Demo.Clock):
    def tick(self, date, current=None):
        print date

class Subscriber(Ice.Application):
    def run(self, args):
        properties = self.communicator().getProperties()

        proxyProperty = 'IceStorm.TopicManager.Proxy'
        proxy = properties.getProperty(proxyProperty)
        if len(proxy) == 0:
            print self.appName() + ": property `" + proxyProperty + "' not set"
            return False

        base = self.communicator().stringToProxy(proxy)
        manager = IceStorm.TopicManagerPrx.checkedCast(self.communicator().stringToProxy(proxy))
        if not manager:
            print args[0] + ": invalid proxy"
            return False

        topicName = "time"
        if len(args) != 1:
            topicName = args[1]

        #
        # Retrieve the topic.
        #
        try:
            topic = manager.retrieve(topicName)
        except IceStorm.NoSuchTopic, e:
            try:
                topic = manager.create(topicName)
            except IceStorm.TopicExists, ex:
                print self.appName() + ": temporay error. try again"
                return False

	adapter = self.communicator().createObjectAdapter("Clock.Subscriber")

	#
	# Add a Servant for the Ice Object.
	#
	subscriber = adapter.addWithUUID(ClockI());

	#
	# This demo requires no quality of service, so it will use
	# the defaults.
	#
	qos = {}

        topic.subscribe(qos, subscriber)
        adapter.activate()

        self.shutdownOnInterrupt()
        self.communicator().waitForShutdown()

        #
        # Unsubscribe all subscribed objects.
        #
        topic.unsubscribe(subscriber)
	    
	return True

app = Subscriber()
sys.exit(app.main(sys.argv, "config.sub"))
