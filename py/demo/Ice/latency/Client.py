#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, time, traceback, Ice

Ice.loadSlice('Latency.ice')
import Demo

class Client(Ice.Application):
    def run(self, args):
        if len(args) > 1:
            print(self.appName() + ": too many arguments")
            return 1

        ping = Demo.PingPrx.checkedCast(self.communicator().propertyToProxy('Ping.Proxy'))
        if not ping:
            print("invalid proxy")
            return 1

        # Initial ping to setup the connection.
        ping.ice_ping();

        repetitions = 100000
        print("pinging server " + str(repetitions) + " times (this may take a while)")

        tsec = time.time()

        i = repetitions
        while(i >= 0):
            ping.ice_ping()
            i = i - 1

        tsec = time.time() - tsec
        tmsec = tsec * 1000.0

        print("time for %d pings: %.3fms" % (repetitions, tmsec))
        print("time per ping: %.3fms" % (tmsec / repetitions))

        return 0

app = Client()
sys.exit(app.main(sys.argv, "config.client"))
