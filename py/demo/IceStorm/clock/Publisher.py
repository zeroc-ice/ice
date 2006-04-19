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

class Publisher(Ice.Application):
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
	# Retrieve the topic named "time".
	#
	try:
	    topic = manager.retrieve("time")
	except IceStorm.NoSuchTopic, e:
	    print self.appName() + ": no such topic name: " + e.name
	    return False

	#
	# Get the topic's publisher object, verify that it supports
	# the Clock type, and create a oneway Clock proxy (for efficiency
	# reasons).
	#
	obj = topic.getPublisher()
	if not obj.ice_isDatagram():
	    obj = obj.ice_oneway()
	clock = Demo.ClockPrx.uncheckedCast(obj)

	print "publishing 10 tick events"
	for i in range(0, 10):
	    clock.tick()

	return True

app = Publisher()
sys.exit(app.main(sys.argv, "config.pub"))
