#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

require 'TestI.rb'

#
# Ice for Ruby behaves differently than Ice for C++, because
# collocated invocations are still sent "over the wire". Therefore
# we always need to install the factories, even for the collocated
# case.
#
class MyObjectFactory
    def create(type)
	if type == '::Test::B'
	    return BI.new
	elsif type == '::Test::C'
	    return CI.new
	elsif type == '::Test::D'
	    return DI.new
	end
	fail "unknown type"
    end

    def destroy
	# Nothing to do
    end
end

def test(b)
    if !b
	raise RuntimeError, 'test assertion failed'
    end
end

def allTests(communicator)
    factory = MyObjectFactory.new
    communicator.addObjectFactory(factory, '::Test::B')
    communicator.addObjectFactory(factory, '::Test::C')
    communicator.addObjectFactory(factory, '::Test::D')

    print "testing stringToProxy... "
    STDOUT.flush
    ref = "initial:default -p 12010 -t 10000"
    base = communicator.stringToProxy(ref)
    test(base)
    puts "ok"

    print "testing checked cast... "
    STDOUT.flush
    initial = Test::InitialPrx::checkedCast(base)
    test(initial)
    test(initial == base)
    puts "ok"

    print "getting B1... "
    STDOUT.flush
    b1 = initial.getB1()
    test(b1)
    puts "ok"
    
    print "getting B2... "
    STDOUT.flush
    b2 = initial.getB2()
    test(b2)
    puts "ok"
    
    print "getting C... "
    STDOUT.flush
    c = initial.getC()
    test(c)
    puts "ok"
    
    print "getting D... "
    STDOUT.flush
    d = initial.getD()
    test(d)
    puts "ok"
    
    print "checking consistency... "
    STDOUT.flush
    test(b1 != b2)
    test(b1 != c)
    test(b1 != d)
    test(b2 != c)
    test(b2 != d)
    test(c != d)
    test(b1.theB == b1)
    test(b1.theC == nil)
    test(b1.theA.is_a?(Test::B))
    test(b1.theA.theA == b1.theA)
    test(b1.theA.theB == b1)
    test(b1.theA.theC)
    test(b1.theA.theC.theB == b1.theA)
    test(b1.preMarshalInvoked)
    test(b1.postUnmarshalInvoked())
    test(b1.theA.preMarshalInvoked)
    test(b1.theA.postUnmarshalInvoked())
    test(b1.theA.theC.preMarshalInvoked)
    test(b1.theA.theC.postUnmarshalInvoked())
    # More tests possible for b2 and d, but I think this is already sufficient.
    test(b2.theA == b2)
    test(d.theC == nil)
    puts "ok"

    print "getting B1, B2, C, and D all at once... "
    STDOUT.flush
    b1, b2, c, d = initial.getAll()
    test(b1)
    test(b2)
    test(c)
    test(d)
    puts "ok"
    
    print "checking consistency... "
    STDOUT.flush
    test(b1 != b2)
    test(b1 != c)
    test(b1 != d)
    test(b2 != c)
    test(b2 != d)
    test(c != d)
    test(b1.theA == b2)
    test(b1.theB == b1)
    test(b1.theC == nil)
    test(b2.theA == b2)
    test(b2.theB == b1)
    test(b2.theC == c)
    test(c.theB == b2)
    test(d.theA == b1)
    test(d.theB == b2)
    test(d.theC == nil)
    test(d.preMarshalInvoked)
    test(d.postUnmarshalInvoked())
    test(d.theA.preMarshalInvoked)
    test(d.theA.postUnmarshalInvoked())
    test(d.theB.preMarshalInvoked)
    test(d.theB.postUnmarshalInvoked())
    test(d.theB.theC.preMarshalInvoked)
    test(d.theB.theC.postUnmarshalInvoked())
    puts "ok"

    return initial
end
