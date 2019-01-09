#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

require 'Ice'
Ice::loadSlice("--all -I. ClientPrivate.ice")
require './AllTests'

class Client < ::TestHelper
    def run(args)
        self.init(args:args) do |communicator|
            initial = allTests(self, communicator)
            initial.shutdown()
        end
    end
end
