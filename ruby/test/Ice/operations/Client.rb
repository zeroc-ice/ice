#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

require "Ice"
Ice::loadSlice("'-I#{Ice.getSliceDir()}' Test.ice")
require './AllTests'

class Client < ::TestHelper
    def run(args)
        properties = self.createTestProperties(args:args)
        #
        # This is not necessary since we don't have AMI support (yet).
        #
        #properties.setProperty('Ice.ThreadPool.Client.Size', '2')
        #properties.setProperty('Ice.ThreadPool.Client.SizeWarn', '0')
        properties.setProperty("Ice.BatchAutoFlushSize", "100")
        self.init(properties:properties) do |communicator|
            myClass = allTests(self, communicator)
            print "testing server shutdown... "
            STDOUT.flush
            myClass.shutdown()
            begin
                myClass.ice_timeout(100).ice_ping(); # Use timeout to speed up testing on Windows
                test(false)
            rescue Ice::LocalException
                puts "ok"
            end
            # Test multiple destroy calls
            communicator.destroy()
            communicator.destroy()
        end
    end
end
