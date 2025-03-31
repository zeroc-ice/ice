#!/usr/bin/env ruby
# Copyright (c) ZeroC, Inc.

require 'Ice'
Ice::loadSlice('Key.ice')
Ice::loadSlice('Clash.ice')

class Client < ::TestHelper
    def run(args)
        self.init(args:args) do |communicator|
            print "testing type names... "
            STDOUT.flush
            a = EscapedBEGIN::END_::Alias
            b = EscapedBEGIN::And.new
            b._begin = 0;
            c = EscapedBEGIN::BreakPrx::new(communicator, "test:tcp")
            test(c.method(:_case))
            test(c.method(:my_operation))
            d1 = EscapedBEGIN::Display.new
            d1._when._begin = 0
            d1._dup = communicator.stringToProxy("test:tcp")
            d1._else = 0
            e = EscapedBEGIN::ElsifPrx::new(communicator, "test:tcp")
            test(e.method(:_case))
            f = EscapedBEGIN::Next.new
            f._new = 0
            g = EscapedBEGIN::Nil.new
            g._new = 0
            g._not = 0
            g._or._begin = 0
            i = EscapedBEGIN::Redo
            puts "ok"
        end
    end
end
