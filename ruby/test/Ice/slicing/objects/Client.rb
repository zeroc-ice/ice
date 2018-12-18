#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

require './AllTests'

class Client < ::TestHelper
    def run(args)
        self.init(args:args) do |communicator|
            t = allTests(self, communicator)
            t.shutdown()
        end
    end
end
