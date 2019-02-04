#!/usr/bin/env ruby
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

require 'Ice'
Ice::loadSlice('Test.ice')

class Client < ::TestHelper
    def run(args)
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
    end
end
