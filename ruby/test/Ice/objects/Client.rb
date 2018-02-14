#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

require 'pathname'
require 'Ice'
Ice::loadSlice('Test.ice')
Ice::loadSlice('ClientPrivate.ice')
require './AllTests'

def run(args, communicator)
    initial = allTests(communicator)
    initial.shutdown()
    return true
end

begin
    communicator = Ice.initialize(ARGV)
    status = run(ARGV, communicator)
rescue => ex
    puts $!
    print ex.backtrace.join("\n")
    status = false
end

if communicator
    communicator.destroy()
end

exit(status ? 0 : 1)
