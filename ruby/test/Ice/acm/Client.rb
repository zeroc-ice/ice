#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

require "Ice"
Ice::loadSlice("Test.ice")
require './AllTests'

class Client < ::TestHelper
    def run(args)
        properties = self.createTestProperties(args:args)
        properties.setProperty("Ice.Warn.Connections", "0")
        self.init(properties:properties) do |communicator|
            allTests(self, communicator)
        end
    end
end
