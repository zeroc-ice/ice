#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, time, traceback, Ice

Ice.loadSlice('Latency.ice')
import Demo

def run(args, communicator):
    properties = communicator.getProperties()
    proxy = properties.getProperty('Latency.Ping')
    if len(proxy) == 0:
        print " property `Latency.Ping' not set"
        return False

    base = communicator.stringToProxy(proxy)
    ping = Demo.PingPrx.checkedCast(base)
    if not ping:
        print "invalid proxy"
        return False

    # Initial ping to setup the connection.
    ping.ice_ping();

    repetitions = 100000
    print "pinging server " + str(repetitions) + " times (this may take a while)"

    tsec = time.time()

    i = repetitions
    while(i >= 0):
        ping.ice_ping()
        i = i - 1

    tsec = time.time() - tsec
    tmsec = tsec * 1000.0

    print "time for %d pings: %.3fms" % (repetitions, tmsec)
    print "time per ping: %.3fms" % (tmsec / repetitions)

    return True

try:
    properties = Ice.createProperties()
    properties.load("config")
    communicator = Ice.initializeWithProperties(sys.argv, properties)
    status = run(sys.argv, communicator)
except:
    traceback.print_exc()
    status = False

if communicator:
    try:
        communicator.destroy()
    except:
        traceback.print_exc()
        status = False

if status:
    sys.exit(0)
else:
    sys.exit(1)
