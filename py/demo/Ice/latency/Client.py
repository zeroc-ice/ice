#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
	properties = self.communicator().getProperties()
	proxy = properties.getProperty('Latency.Ping')
	if len(proxy) == 0:
	    print " property `Latency.Ping' not set"
	    return False

	ping = Demo.PingPrx.checkedCast(self.communicator().stringToProxy(proxy))
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

app = Client()
sys.exit(app.main(sys.argv, "config"))
