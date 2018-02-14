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
if slice_dir.empty?
    fail "Slice directory not found"
end

Ice::loadSlice("'-I" + slice_dir + "' Test.ice")
require './AllTests'

def test(b)
    if !b
        raise RuntimeError, 'test assertion failed'
    end
end

def run(args, communicator)
    myClass = allTests(communicator)

    myClass.shutdown()
    return true
end

begin
    initData = Ice::InitializationData.new
    initData.properties = Ice.createProperties(ARGV)
    communicator = Ice.initialize(ARGV, initData)
    status = run(ARGV, communicator)
rescue => ex
    puts $!
    print ex.backtrace.join("\n")
    status = false
end

if communicator
    begin
        communicator.destroy()
    rescue => ex
        puts $!
        print ex.backtrace.join("\n")
        status = false
    end
end

exit(status ? 0 : 1)
