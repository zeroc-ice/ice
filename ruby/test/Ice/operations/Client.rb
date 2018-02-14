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

def test(b)
    if !b
        raise RuntimeError, 'test assertion failed'
    end
end

def run(args, communicator)
    myClass = allTests(communicator)

    print "testing server shutdown... "
    STDOUT.flush
    myClass.shutdown()
    begin
        myClass.ice_timeout(100).ice_ping(); # Use timeout to speed up testing on Windows
        test(false)
    rescue Ice::LocalException
        puts "ok"
    end

    return true
end

begin
    initData = Ice::InitializationData.new
    initData.properties = Ice.createProperties(ARGV)
    #
    # This is not necessary since we don't have AMI support (yet).
    #
    #initData.properties.setProperty('Ice.ThreadPool.Client.Size', '2')
    #initData.properties.setProperty('Ice.ThreadPool.Client.SizeWarn', '0')

    initData.properties.setProperty("Ice.BatchAutoFlushSize", "100")

    communicator = Ice.initialize(ARGV, initData)
    status = run(ARGV, communicator)

    # Test multiple destroy calls
    communicator.destroy()
    communicator.destroy()

rescue => ex
    puts $!
    print ex.backtrace.join("\n")
    status = false
end

if communicator
    communicator.destroy()
end

exit(status ? 0 : 1)
