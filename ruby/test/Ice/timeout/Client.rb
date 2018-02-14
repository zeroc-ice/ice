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
    allTests(communicator)
    return true
end

begin
    #
    # In this test, we need at least two threads in the
    # client side thread pool for nested AMI.
    #
    initData = Ice::InitializationData.new
    initData.properties = Ice.createProperties(ARGV)

    #
    # For this test, we want to disable retries.
    #
    initData.properties.setProperty('Ice.RetryIntervals', '-1')

    #
    # This test kills connections, so we don't want warnings.
    #
    initData.properties.setProperty('Ice.Warn.Connections', '0')

    #
    # Limit the send buffer size, this test relies on the socket
    # send() blocking after sending a given amount of data.
    #
    initData.properties.setProperty("Ice.TCP.SndSize", "50000");

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
