#!/usr/bin/env ruby
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

require "Ice"
Ice::loadSlice("'-I#{Ice.getSliceDir}' --checksum Test.ice CTypes.ice")
require './AllTests'

class Client < ::TestHelper
    def run(args)
        self.init(args:args) do |communicator|
            checksum = allTests(self, communicator)
            checksum.shutdown()
        end
    end
end
