#!/usr/bin/env ruby
# Copyright (c) ZeroC, Inc.

require 'Ice'
Ice::loadSlice(["Test.ice", "Forward.ice", "ClientPrivate.ice"])
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

        # The block form of Ice::initialize destroys the communicator through the C++ API when the block returns,
        # without going through the Ruby-level destroy. The process must still exit cleanly when a custom slice
        # loader is set.
        initData = Ice::InitializationData.new
        initData.properties = self.createTestProperties(args:args)
        initData.sliceLoader = CustomSliceLoader.new
        Ice::initialize(initData) do |communicator|
        end
    end
end
