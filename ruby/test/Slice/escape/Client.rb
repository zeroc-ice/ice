#!/usr/bin/env ruby
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

require 'Ice'
Ice::loadSlice('Key.ice')
Ice::loadSlice('Clash.ice')

class Client < ::TestHelper
    def run(args)
        self.init(args:args) do |communicator|
            print "testing type names... "
            STDOUT.flush
            a = BEGIN_::END_::Alias
            b = BEGIN_::And.new
            b._begin = 0;
            c = BEGIN_::BreakPrx::uncheckedCast(communicator.stringToProxy("test:tcp"))
            test(c.method(:_case))
            test(c.method(:_to_a))
            test(c.method(:_instance_variable_set))
            test(c.method(:_instance_variables))
            d1 = BEGIN_::Display.new
            d1._when = 0
            d1._do = 0
            d1._dup = communicator.stringToProxy("test:tcp")
            d1._else = 0
            e = BEGIN_::ElsifPrx::uncheckedCast(communicator.stringToProxy("test:tcp"))
            test(e.method(:_case))
            f = BEGIN_::Next.new
            f._new = 0
            g = BEGIN_::Nil.new
            g._new = 0
            g._not = 0
            g._or = 0
            # TODO: Test local interface
            #h = BEGIN_::ExtendPrx::uncheckedCast(communicator.stringToProxy("test:tcp"))
            i = BEGIN_::Redo
            puts "ok"
        end
    end
end
