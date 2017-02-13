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
Ice::loadSlice('--underscore Key.ice')

def test(b)
    if !b
        raise RuntimeError, 'test assertion failed'
    end
end

class DisplayI  < BEGIN_::Display
    def _do(_dup, current=nil)
    end
end

def run(args, communicator)
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
    d = BEGIN_::DisplayPrx::uncheckedCast(communicator.stringToProxy("test:tcp"))
    test(d.method(:_do))
    d1 = DisplayI.new
    e = BEGIN_::ElsifPrx::uncheckedCast(communicator.stringToProxy("test:tcp"))
    test(e.method(:_do))
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

    return true
end

begin
    initData = Ice::InitializationData.new
    initData.properties = Ice.createProperties(ARGV)
    communicator = Ice.initialize(ARGV, initData)
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
