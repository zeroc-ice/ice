#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

require 'pathname'

require 'Ice'
slice_dir = Ice.getSliceDir
if slice_dir.empty?
    fail "Slice directory not found"
end

Ice::loadSlice("'-I" + slice_dir + "' Test.ice")
require './AllTests'

def run(args, communicator)
    allTests(communicator, "ServerManager:default -p 12010")
    return true
end

begin
    data = Ice::InitializationData.new
    data.properties = Ice::createProperties(ARGV)
    data.properties.setProperty("Ice.Default.Locator", "locator:default -p 12010")
    communicator = Ice::initialize(ARGV, data)
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
