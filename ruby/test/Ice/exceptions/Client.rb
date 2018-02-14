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

slice_dir = Ice.getSliceDir
Ice::loadSlice("'-I" + slice_dir + "' Test.ice")
require './AllTests'

def run(args, communicator)
    thrower = allTests(communicator)
    thrower.shutdown()
    return true
end

begin
    initData = Ice::InitializationData.new
    initData.properties = Ice.createProperties(ARGV)
    initData.properties.setProperty("Ice.MessageSizeMax", "10")
    initData.properties.setProperty("Ice.Warn.Connections", "0")
    communicator = Ice.initialize(ARGV, initData)
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
