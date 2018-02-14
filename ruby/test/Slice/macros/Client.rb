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

require 'Ice'
Ice::loadSlice('Test.ice')

def test(b)
    if !b
        raise RuntimeError, 'test assertion failed'
    end
end

status = true

begin
    print "testing Slice predefined macros... "
    STDOUT.flush
    d = Test::Default.new
    test(d.x == 10)
    test(d.y == 10)

    nd = Test::NoDefault.new
    test(nd.x != 10)
    test(nd.y != 10)

    c = Test::RubyOnly.new
    test(c.lang.eql? "ruby")
    test(c.version == Ice::intVersion())
    puts "ok"
rescue => ex
    puts $!
    print ex.backtrace.join("\n")
    status = false
end

exit(status ? 0 : 1)
