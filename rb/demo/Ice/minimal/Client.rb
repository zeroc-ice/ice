#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

require 'Ice'

Ice::loadSlice('Hello.ice')

status = 0
communicator = nil
begin
    communicator = Ice::initialize(ARGV)
    hello = Demo::HelloPrx::checkedCast(communicator.stringToProxy("hello:tcp -p 10000"))
    if not hello
        puts $0 + ": invalid proxy"
        status = 1
    else
        hello.sayHello()
    end
rescue => ex
    puts $!
    puts ex.backtrace.join("\n")
    status = 1
end

if communicator
    begin
        communicator.destroy()
    rescue => ex
        puts $!
        puts ex.backtrace.join("\n")
        status = 1
    end
end

exit(status)
