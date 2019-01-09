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
        allTests()
    end
end
