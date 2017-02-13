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

slice_dir = Ice.getSliceDir
if slice_dir.empty?
    fail "Slice directory not found"
end

Ice::loadSlice("'-I" + slice_dir + "' --checksum Test.ice CTypes.ice")
require './AllTests'

def run(args, communicator)
    checksum = allTests(communicator)
    checksum.shutdown()
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
