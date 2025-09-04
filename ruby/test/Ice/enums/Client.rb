#!/usr/bin/env ruby
# Copyright (c) ZeroC, Inc.

require "Ice"
Ice::loadSlice(["Test.ice"])
require './AllTests'

class Client < ::TestHelper
    def run(args)
        self.init(args:args) do |communicator|
            proxy = allTests(self, communicator)
            proxy.shutdown()
        end
    end
end
