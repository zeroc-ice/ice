#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, Ice, IceStorm, getopt

Ice.loadSlice('Clock.ice')
import Demo

class ClockI(Demo.Clock):
    def tick(self, date, current=None):
        print date

class Subscriber(Ice.Application):
    def usage(self):
        print "Usage: " + self.appName() + " [--batch] [--datagram|--twoway|--ordered|--oneway] [topic]"

    def run(self, args):
        try:
            opts, args = getopt.getopt(args[1:], '', ['datagram', 'twoway', 'oneway', 'ordered', 'batch'])
        except getopt.GetoptError:
            self.usage()
            return 1

        topicName = "time"
        datagram = False
        twoway = False
        ordered = False
        batch = False
        optsSet = 0
        for o, a in opts:
            if o == "--datagram":
                datagram = True
                optsSet = optsSet + 1
            elif o =="--twoway":
                twoway = True
                optsSet = optsSet + 1
            elif o =="--ordered":
                ordered = True
                optsSet = optsSet + 1
            elif o =="--oneway":
                optsSet = optsSet + 1
            elif o =="--batch":
                batch = True

        if batch and (twoway or ordered):
            print self.appName() + ": batch can only be set with oneway or datagram"
            return 1

        if optsSet > 1:
            self.usage()
            sys.exit(1)

        if len(args) > 0:
            topicName = args[0]

        manager = IceStorm.TopicManagerPrx.checkedCast(\
            self.communicator().propertyToProxy('IceStorm.TopicManager.Proxy'))
        if not manager:
            print args[0] + ": invalid proxy"
            return 1

        #
        # Retrieve the topic.
        #
        try:
            topic = manager.retrieve(topicName)
        except IceStorm.NoSuchTopic, e:
            try:
                topic = manager.create(topicName)
            except IceStorm.TopicExists, ex:
                print self.appName() + ": temporary error. try again"
                return 1

        adapter = self.communicator().createObjectAdapter("Clock.Subscriber")

        #
        # Add a Servant for the Ice Object.
        #
        qos = {}
        subscriber = adapter.addWithUUID(ClockI())

        #
        # Set up the proxy.
        #
        if datagram:
            subscriber = subscriber.ice_datagram()
        elif twoway:
             pass
            # Do nothing to the subscriber proxy. Its already twoway.
        elif ordered:
            # Do nothing to the subscriber proxy. Its already twoway.
            qos["reliability"] = "ordered"
        else: # if(oneway)
            subscriber = subscriber.ice_oneway()
        if batch:
            if datagram:
                subscriber = subscriber.ice_batchDatagram()
            else:
                subscriber = subscriber.ice_batchOneway()

        topic.subscribeAndGetPublisher(qos, subscriber)
        adapter.activate()

        self.shutdownOnInterrupt()
        self.communicator().waitForShutdown()

        #
        # Unsubscribe all subscribed objects.
        #
        topic.unsubscribe(subscriber)
            
        return 0

app = Subscriber()
sys.exit(app.main(sys.argv, "config.sub"))
