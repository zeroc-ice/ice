#!/usr/bin/env ruby
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

require "Ice"
Ice::loadSlice("'-I#{Ice.getSliceDir()}' Test.ice")
require './AllTests'

class Client < ::TestHelper
    def run(args)
        properties = self.createTestProperties(args:args)
        properties.setProperty("Ice.Default.Locator", "locator:#{self.getTestEndpoint(properties:properties)}")
        self.init(properties:properties) do |communicator|
            allTests(self, communicator)
        end
    end
end
