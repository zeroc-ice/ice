#!/usr/bin/env ruby
# Copyright (c) ZeroC, Inc.

require "Ice"
Ice::loadSlice(["-I#{Ice.getSliceDir()}", "Test.ice"])
require './AllTests'

class Client < ::TestHelper
    def run(args)
        properties = self.createTestProperties(args:args)
        properties.setProperty("Ice.MessageSizeMax", "10")
        properties.setProperty("Ice.Warn.Connections", "0")
        self.init(properties:properties) do |communicator|
            thrower = allTests(self, communicator)
            thrower.shutdown()
        end
    end
end
