#!/usr/bin/env ruby
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

require 'Ice'
Ice::loadSlice("'-I#{Ice.getSliceDir()}' Test.ice")
require './AllTests'

class Client < ::TestHelper
    def run(args)
        #
        # In this test, we need at least two threads in the
        # client side thread pool for nested AMI.
        #
        properties = self.createTestProperties(args:args)

        #
        # For this test, we want to disable retries.
        #
        properties.setProperty('Ice.RetryIntervals', '-1')

        #
        # This test kills connections, so we don't want warnings.
        #
        properties.setProperty('Ice.Warn.Connections', '0')

        #
        # Limit the send buffer size, this test relies on the socket
        # send() blocking after sending a given amount of data.
        #
        properties.setProperty("Ice.TCP.SndSize", "50000");

        self.init(properties:properties) do |communicator|
            allTests(self, communicator)
        end
    end
end
