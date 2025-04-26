#!/usr/bin/env ruby
# Copyright (c) ZeroC, Inc.

require 'Ice'
Ice::loadSlice('Test.ice')
Ice::loadSlice('Forward.ice')
Ice::loadSlice('ClientPrivate.ice')
require './AllTests'
require './TestI.rb'

class Client < ::TestHelper
    def run(args)
        initData = Ice::InitializationData.new
        initData.properties = self.createTestProperties(args:args)
        initData.sliceLoader = CustomSliceLoader.new
        self.init(initData:initData) do |communicator|
            initial = allTests(self, communicator)
            initial.shutdown()
        end
    end
end
