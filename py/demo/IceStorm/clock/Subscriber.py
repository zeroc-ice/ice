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
    def tick(self, current=None):
        print "tick"

class Subscriber(Ice.Application):
    def run(self, args):
        properties = self.communicator().getProperties()

        proxyProperty = 'IceStorm.TopicManager.Proxy'
        proxy = properties.getProperty(proxyProperty)
        if len(proxy) == 0:
            print self.appName() + ": property `" + proxyProperty + "' not set"
            return False

        base = self.communicator().stringToProxy(proxy)
        manager = IceStorm.TopicManagerPrx.checkedCast(base)
        if not manager:
            print args[0] + ": invalid proxy"
            return False

	#
	# Gather the set of topics to which to subscribe. It is either
	# the set provided on the command line, or the topic "time".
	#
	topics = []
	if len(args) > 1:
	    for i in range(1, len(args)):
	        topics.append(args[i])
	else:
	    topics.append("time")

	#
	# Set the requested quality of service "reliability" =
	# "batch". This tells IceStorm to send events to the subscriber
	# in batches at regular intervals.
	#
	qos = {}
	qos["reliability"] = "batch"

	#
	# Create the servant to receive the events.
	#
	adapter = self.communicator().createObjectAdapter("Clock.Subscriber")
	clock = ClockI()


	#
	# List of all subscribers.
	#
	subscribers = {}

	#
	# Add the servant to the adapter for each topic. A ServantLocator
	# could have been used for the same purpose.
	#
	for i in range(0, len(topics)):
	    object = adapter.addWithUUID(clock)
	    try:
	        topic = manager.retrieve(topics[i])
		topic.subscribe(qos, object)
	    except IceStorm.NoSuchTopic, e:
	        print self.appName() + ": no such topic name: " + e.name
		break

	    subscribers[topics[i]] = object

	if len(subscribers) == len(topics):
	    adapter.activate()
	    self.shutdownOnInterrupt()
	    self.communicator().waitForShutdown()

	for name in subscribers.keys():
	    try:
	        topic = manager.retrieve(name)
		topic.unsubscribe(subscribers[name])
	    except IceStorm.NoSuchTopic, e:
	        print self.appName() + ": no such topic  name: " + e.name
	    
	return True

app = Subscriber()
sys.exit(app.main(sys.argv, "config.sub"))
