#!/usr/bin/env ruby
# Copyright (c) ZeroC, Inc.

require 'Ice'
Ice::loadSlice('Test.ice')
Ice::loadSlice('Forward.ice')
Ice::loadSlice('ClientPrivate.ice')
require './AllTests'

class Client < ::TestHelper
    def run(args)
        self.init(args:args) do |communicator|
            initial = allTests(self, communicator)
            initial.shutdown()
        end
    end
end
