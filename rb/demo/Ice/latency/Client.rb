#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

require 'Ice'

Ice::loadSlice('Latency.ice')

class Client < Ice::Application
    def run(args)
        ping = Demo::PingPrx::checkedCast(Ice::Application::communicator().propertyToProxy('Latency.Ping'))
        if not ping
            puts "invalid proxy"
            return 1
        end

        # Initial ping to setup the connection.
        ping.ice_ping()

        repetitions = 100000
        puts "pinging server " + repetitions.to_s + " times (this may take a while)"

        tsec = Time.now

        i = repetitions
        while i >= 0
            ping.ice_ping()
            i = i - 1
        end

        tsec = Time.now - tsec
        tmsec = tsec * 1000.0

        printf "time for %d pings: %.3fms\n", repetitions, tmsec
        printf "time per ping: %.3fms\n", tmsec / repetitions

        return 0
    end
end

app = Client.new
exit(app.main(ARGV, "config.client"))
