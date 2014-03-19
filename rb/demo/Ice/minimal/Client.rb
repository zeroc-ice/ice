#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

require 'Ice'

Ice::loadSlice('Hello.ice')

begin
    communicator = Ice::initialize(ARGV)
    hello = Demo::HelloPrx::checkedCast(communicator.stringToProxy("hello:tcp -h localhost -p 10000"))
    hello.sayHello()
    communicator.destroy()
rescue => ex
    puts $!
    puts ex.backtrace.join("\n")
    exit(1)
end
