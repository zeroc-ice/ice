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
Ice::loadSlice('Test.ice')

def test(b)
    if !b
        raise RuntimeError, 'test assertion failed'
    end
end

def allTests(communicator)
    print "testing equals() for Slice structures... "
    STDOUT.flush

    #
    # Define some default values.
    #
    def_s2 = Test::S2.new(true, 98, 99, 100, 101, 1.0, 2.0, "string", ["one", "two", "three"], {"abc"=>"def"}, \
                          Test::S1.new("name"), Test::C.new(5), communicator.stringToProxy("test"))

    #
    # Compare default-constructed structures.
    #
    test(Test::S2.new.eql?(Test::S2.new))

    #
    # Change one primitive member at a time.
    #
    v = def_s2.clone
    test(v.eql?(def_s2))

    v = def_s2.clone
    v.bo = false
    test(!v.eql?(def_s2))

    v = def_s2.clone
    v.by = v.by - 1
    test(!v.eql?(def_s2))

    v = def_s2.clone
    v.sh = v.sh - 1
    test(!v.eql?(def_s2))

    v = def_s2.clone
    v.i = v.i - 1
    test(!v.eql?(def_s2))

    v = def_s2.clone
    v.l = v.l - 1
    test(!v.eql?(def_s2))

    v = def_s2.clone
    v.f = v.f - 1
    test(!v.eql?(def_s2))

    v = def_s2.clone
    v.d = v.d - 1
    test(!v.eql?(def_s2))

    v = def_s2.clone
    v.str = ""
    test(!v.eql?(def_s2))

    #
    # String member
    #
    v1 = def_s2.clone
    v1.str = "string"
    test(v1.eql?(def_s2))

    v1 = def_s2.clone
    v2 = def_s2.clone
    v1.str = nil
    test(!v1.eql?(v2))

    v1 = def_s2.clone
    v2 = def_s2.clone
    v2.str = nil
    test(!v1.eql?(v2))

    v1 = def_s2.clone
    v2 = def_s2.clone
    v1.str = nil
    v2.str = nil
    test(v1.eql?(v2))

    #
    # Sequence member
    #
    v1 = def_s2.clone
    v1.ss = def_s2.ss.clone
    test(v1.eql?(def_s2))

    v1 = def_s2.clone
    v1.ss = []
    test(!v1.eql?(def_s2))

    v1 = def_s2.clone
    v1.ss = ["one", "two", "three"]
    test(v1.eql?(def_s2))

    v1 = def_s2.clone
    v2 = def_s2.clone
    v1.ss = nil
    test(!v1.eql?(v2))

    v1 = def_s2.clone
    v2 = def_s2.clone
    v2.ss = nil
    test(!v1.eql?(v2))

    #
    # Dictionary member
    #
    v1 = def_s2.clone
    v1.sd = {"abc"=>"def"}
    test(v1.eql?(def_s2))

    v1 = def_s2.clone
    v1.sd = {}
    test(!v1.eql?(def_s2))

    v1 = def_s2.clone
    v2 = def_s2.clone
    v1.sd = nil
    test(!v1.eql?(v2))

    v1 = def_s2.clone
    v2 = def_s2.clone
    v2.sd = nil
    test(!v1.eql?(v2))

    #
    # Struct member
    #
    v1 = def_s2.clone
    v1.s = def_s2.s.clone
    test(v1.eql?(def_s2))

    v1 = def_s2.clone
    v1.s = Test::S1.new("name")
    test(v1.eql?(def_s2))

    v1 = def_s2.clone
    v1.s = Test::S1.new("noname")
    test(!v1.eql?(def_s2))

    #
    # Class member
    #
    v1 = def_s2.clone
    v1.cls = def_s2.cls.clone
    test(!v1.eql?(def_s2))

    #
    # Proxy member
    #
    v1 = def_s2.clone
    v1.prx = communicator.stringToProxy("test")
    test(v1.eql?(def_s2))

    v1 = def_s2.clone
    v1.prx = communicator.stringToProxy("test2")
    test(!v1.eql?(def_s2))

    v1 = def_s2.clone
    v2 = def_s2.clone
    v1.prx = nil
    test(!v1.eql?(v2))

    v1 = def_s2.clone
    v2 = def_s2.clone
    v2.prx = nil
    test(!v1.eql?(v2))

    puts "ok"
end

def run(args, communicator)
    allTests(communicator)

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
