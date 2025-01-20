#!/usr/bin/env ruby
# Copyright (c) ZeroC, Inc.

require './AllTests'

class Client < ::TestHelper
    def run(args)
        self.init(args:args) do |communicator|
            t = allTests(self, communicator)
            t.shutdown()
        end
    end
end
