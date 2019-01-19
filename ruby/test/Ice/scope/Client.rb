#!/usr/bin/env ruby
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

require 'Ice'
Ice::loadSlice("Test.ice")
require './AllTests'

class Client < ::TestHelper
    def run(args)
        self.init(args:args) do |communicator|
            allTests(self, communicator)
        end
    end
end
