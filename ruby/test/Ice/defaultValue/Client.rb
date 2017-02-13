#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

require 'pathname'
require 'Ice'

Ice::loadSlice('Test.ice')
require './AllTests'

def test(b)
    if !b
        raise RuntimeError, 'test assertion failed'
    end
end

def run(args)
    allTests()
    return true
end

begin
    status = run(ARGV)
rescue => ex
    puts $!
    print ex.backtrace.join("\n")
    status = false
end

exit(status ? 0 : 1)
