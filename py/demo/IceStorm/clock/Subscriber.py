#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
        print(date)

class Subscriber(Ice.Application):
    def usage(self):
        print("Usage: " + self.appName() + \
            " [--batch] [--datagram|--twoway|--ordered|--oneway] [--retryCount count] [--id id] [topic]")

    def run(self, args):
        try:
            opts, args = getopt.getopt(args[1:], '', ['datagram', 'twoway', 'oneway', 'ordered', 'batch',
                    'retryCount=', 'id='])
        except getopt.GetoptError:
            self.usage()
            return 1

        batch = False
        option = "None"
        topicName = "time"
        id = ""
        retryCount = ""

        for o, a in opts:
            oldoption = option
            if o == "--datagram":
                option = "Datagram"
            elif o =="--twoway":
                option = "Twoway"
            elif o =="--ordered":
                option = "Ordered"
            elif o =="--oneway":
                    option = "Oneway"
            elif o =="--batch":
                batch = True
            elif o == "--id":
                id = a
            elif o == "--retryCount":
                retryCount = a
            if oldoption != option and oldoption != "None":
                self.usage()
                return 1

        if len(args) > 1:
                self.usage()
                return 1

        if len(args) > 0:
            topicName = args[0]

        if len(retryCount) > 0:
            if option == "None":
                option = "Twoway"
            elif option != "Twoway" and option != "Ordered":
                print(self.appName() + ": retryCount requires a twoway proxy")
                return 1

        if batch and (option in ("Twoway", "Ordered")):
            print(self.appName() + ": batch can only be set with oneway or datagram")
            return 1

        manager = IceStorm.TopicManagerPrx.checkedCast(self.communicator().propertyToProxy('TopicManager.Proxy'))
        if not manager:
            print(args[0] + ": invalid proxy")
            return 1

        #
        # Retrieve the topic.
        #
        try:
            topic = manager.retrieve(topicName)
        except IceStorm.NoSuchTopic as e:
            try:
                topic = manager.create(topicName)
            except IceStorm.TopicExists as ex:
                print(self.appName() + ": temporary error. try again")
                return 1

        adapter = self.communicator().createObjectAdapter("Clock.Subscriber")

        #
        # Add a servant for the Ice object. If --id is used the identity
        # comes from the command line, otherwise a UUID is used.
        #
        # id is not directly altered since it is used below to detect
        # whether subscribeAndGetPublisher can raise AlreadySubscribed.
        #

        subId = Ice.Identity()
        subId.name = id
        if len(subId.name) == 0:
            subId.name = Ice.generateUUID()
        subscriber = adapter.add(ClockI(), subId)

        #
        # Activate the object adapter before subscribing.
        #
        adapter.activate()

        qos = {}
        if len(retryCount) > 0:
            qos["retryCount"] = retryCount

        #
        # Set up the proxy.
        #
        if option == "Datagram":
            if batch:
                subscriber = subscriber.ice_batchDatagram()
            else:
                subscriber = subscriber.ice_datagram()
        elif option == "Twoway":
            # Do nothing to the subscriber proxy. Its already twoway.
             pass
        elif option == "Ordered":
            # Do nothing to the subscriber proxy. Its already twoway.
            qos["reliability"] = "ordered"
        elif option == "Oneway" or option == "None":
            if batch:
                subscriber = subscriber.ice_batchOneway()
            else:
                subscriber = subscriber.ice_oneway()

        try:
            topic.subscribeAndGetPublisher(qos, subscriber)
        except IceStorm.AlreadySubscribed as ex:
            # If we're manually setting the subscriber id ignore.
            if len(id) == 0:
                raise
            print("reactivating persistent subscriber")

        self.shutdownOnInterrupt()
        self.communicator().waitForShutdown()

        #
        # Unsubscribe all subscribed objects.
        #
        topic.unsubscribe(subscriber)

        return 0

app = Subscriber()
sys.exit(app.main(sys.argv, "config.sub"))
