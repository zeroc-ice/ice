#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, time, Ice, IceStorm

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

	#
	# Get the topic's publisher object, the Clock type, and create a 
	# oneway Clock proxy (for efficiency reasons).
	#
	clock = Demo.ClockPrx.uncheckedCast(topic.getPublisher().ice_oneway())

	print "publishing tick events. Press ^C to terminate the application."
	try:
	    while 1:
	        clock.tick(time.strftime("%m/%d/%Y %H:%M:%S"))
		time.sleep(1)
	except Ice.CommunicatorDestroyedException, e:
	    # Ignore
	    pass
		
	return True

app = Publisher()
sys.exit(app.main(sys.argv, "config.pub"))
