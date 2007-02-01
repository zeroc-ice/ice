#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

require 'pathname'

rubyDir = nil
for toplevel in [".", "..", "../..", "../../..", "../../../.."]
    path = Pathname.new(toplevel).join("ruby", "Ice.rb")
    if path.file?
        rubyDir = Pathname.new(toplevel).join("ruby")
        break
    end
end
if not rubyDir
    fail "can't find toplevel directory!"
end
$:.unshift(rubyDir)

require 'Ice'
Ice::loadSlice('Test.ice')
require 'AllTests'

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
    begin
        communicator.destroy()
    rescue => ex
        puts $!
        print ex.backtrace.join("\n")
        status = false
    end
end

exit(status ? 0 : 1)
