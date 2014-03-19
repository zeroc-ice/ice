#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, time, Ice, IceStorm, getopt

Ice.loadSlice('Clock.ice')
import Demo

class Publisher(Ice.Application):
    def usage(self):
        print("Usage: " + self.appName() + " [--datagram|--twoway|--oneway] [topic]")

    def run(self, args):
        try:
            opts, args = getopt.getopt(args[1:], '', ['datagram', 'twoway', 'oneway'])
        except getopt.GetoptError:
            self.usage()
            return 1

        datagram = False
        twoway = False
        optsSet = 0
        topicName = "time"
        for o, a in opts:
            if o == "--datagram":
                datagram = True
                optsSet = optsSet + 1
            elif o == "--twoway":
                twoway = True
                optsSet = optsSet + 1
            elif o == "--oneway":
                optsSet = optsSet + 1

        if optsSet > 1:
            self.usage()
            return 1

        if len(args) > 0:
            topicName = args[0]

        manager = IceStorm.TopicManagerPrx.checkedCast(self.communicator().propertyToProxy('TopicManager.Proxy'))
        if not manager:
            print(args[0] + ": invalid proxy")
            return 1

        #
        # Retrieve the topic.
        #
        try:
            topic = manager.retrieve(topicName)
        except IceStorm.NoSuchTopic:
            try:
                topic = manager.create(topicName)
            except IceStorm.TopicExists:
                print(self.appName() + ": temporary error. try again")
                return 1

        #
        # Get the topic's publisher object, and create a Clock proxy with
        # the mode specified as an argument of this application.
        #
        publisher = topic.getPublisher();
        if datagram:
            publisher = publisher.ice_datagram();
        elif twoway:
            # Do nothing.
            pass
        else: # if(oneway)
            publisher = publisher.ice_oneway();
        clock = Demo.ClockPrx.uncheckedCast(publisher)

        print("publishing tick events. Press ^C to terminate the application.")
        try:
            while 1:
                clock.tick(time.strftime("%m/%d/%Y %H:%M:%S"))
                time.sleep(1)
        except IOError:
            # Ignore
            pass
        except Ice.CommunicatorDestroyedException:
            # Ignore
            pass
                
        return 0

app = Publisher()
sys.exit(app.main(sys.argv, "config.pub"))
