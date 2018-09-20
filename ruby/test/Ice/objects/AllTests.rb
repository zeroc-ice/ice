# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

require './TestI.rb'

class II < ::Ice::InterfaceByValue
    def initialize()
        super("::Test::I")
    end
end

class JI < ::Ice::InterfaceByValue
    def initialize()
        super("::Test::J")
    end
end

#
# Ice for Ruby behaves differently than Ice for C++, because
# collocated invocations are still sent "over the wire". Therefore
# we always need to install the factories, even for the collocated
# case.
#
class MyValueFactory
    def create(type)
        if type == '::Test::B'
            return BI.new
        elsif type == '::Test::C'
            return CI.new
        #
        # We do not specialize D, instead we just re-open it to define
        # its methods.
        #
        #elsif type == '::Test::D'
        #      return DI.new
        elsif type == '::Test::E'
            return EI.new
        elsif type == '::Test::F'
            return FI.new
        elsif type == '::Test::I'
            return II.new
        elsif type == '::Test::J'
            return JI.new
        end
        fail "unknown type"
    end
end

class MyObjectFactory
    def create(type)
        return nil
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

def allTests(helper, communicator)

    factory = MyValueFactory.new
    communicator.getValueFactoryManager().add(factory, '::Test::B')
    communicator.getValueFactoryManager().add(factory, '::Test::C')
    #communicator.getValueFactoryManager().add(factory, '::Test::D')
    communicator.getValueFactoryManager().add(factory, '::Test::E')
    communicator.getValueFactoryManager().add(factory, '::Test::F')
    communicator.getValueFactoryManager().add(factory, '::Test::I')
    communicator.getValueFactoryManager().add(factory, '::Test::J')

    communicator.addObjectFactory(MyObjectFactory.new, 'TestOF')

    print "testing stringToProxy... "
    STDOUT.flush
    ref = "initial:#{helper.getTestEndpoint()}"
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
    test(b1.postUnmarshalInvoked)
    test(b1.theA.preMarshalInvoked)
    test(b1.theA.postUnmarshalInvoked)
    test(b1.theA.theC.preMarshalInvoked)
    test(b1.theA.theC.postUnmarshalInvoked)
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
    test(d.postUnmarshalInvoked)
    test(d.theA.preMarshalInvoked)
    test(d.theA.postUnmarshalInvoked)
    test(d.theB.preMarshalInvoked)
    test(d.theB.postUnmarshalInvoked)
    test(d.theB.theC.preMarshalInvoked)
    test(d.theB.theC.postUnmarshalInvoked)
    puts "ok"

    print "testing protected members... "
    STDOUT.flush
    e = initial.getE()
    test(e.checkValues())
    begin
        e.i # Test that i is not accessible
        test(false)
    rescue NoMethodError
        # Expected
    end
    begin
        e.s # Test that s is not accessible
        test(false)
    rescue NoMethodError
        # Expected
    end
    f = initial.getF()
    test(f.checkValues())
    test(f.e2.checkValues())
    begin
        f.e1 # Test that e1 is not accessible
        test(false)
    rescue NoMethodError
        # Expected
    end
    puts "ok"

    print "getting I, J, H... "
    STDOUT.flush
    i = initial.getI()
    test(i)
    j = initial.getJ()
    test(i)
    h = initial.getH()
    test(i)
    puts "ok"

    print "getting K... "
    STDOUT.flush
    k = initial.getK()
    test(k.value.data == "l")
    puts "ok"

    print "testing Value as parameter... "
    STDOUT.flush
    v1 = Test::L.new("l")
    v2, v3 = initial.opValue(v1)
    test(v2.data == "l")
    test(v3.data == "l")

    v1 = [Test::L.new("l")]
    v2, v3 = initial.opValueSeq(v1)
    test(v2[0].data == "l")
    test(v3[0].data == "l")

    v1 = {}
    v1["l"] = Test::L.new("l")
    v2, v3 = initial.opValueMap(v1)
    test(v2["l"].data == "l")
    test(v3["l"].data == "l")
    puts "ok"

    print "getting D1... "
    STDOUT.flush
    d1 = initial.getD1(Test::D1.new(Test::A1.new("a1"), Test::A1.new("a2"), Test::A1.new("a3"), Test::A1.new("a4")))
    test(d1.a1.name == "a1")
    test(d1.a2.name == "a2")
    test(d1.a3.name == "a3")
    test(d1.a4.name == "a4")
    puts "ok"

    print "throw EDerived... "
    STDOUT.flush
    begin
        initial.throwEDerived()
        test(false)
    rescue Test::EDerived => e
        test(e.a1.name == "a1")
        test(e.a2.name == "a2")
        test(e.a3.name == "a3")
        test(e.a4.name == "a4")
    end
    puts "ok"

    print "setting G... "
    STDOUT.flush
    begin
        initial.setG(Test::G.new(Test::S.new("hello"), "g"))
    rescue Ice::OperationNotExistException
    end
    puts "ok"

    print "setting I... "
    STDOUT.flush
    initial.setI(i)
    initial.setI(j)
    initial.setI(h)
    puts "ok"

    print "testing sequences... "
    STDOUT.flush
    initial.opBaseSeq([])

    retS, outS = initial.opBaseSeq([Test::Base.new])
    test(retS.length == 1 && outS.length == 1)
    puts "ok"

    print "testing recursive type... "
    STDOUT.flush
    top = Test::Recursive.new
    p = top;
    depth = 0;
    begin
        while depth <= 700
            p.v = Test::Recursive.new
            p = p.v;
            if (depth < 10 && (depth % 10) == 0) || \
               (depth < 1000 && (depth % 100) == 0) || \
               (depth < 10000 && (depth % 1000) == 0) || \
               (depth % 10000) == 0
                initial.setRecursive(top)
            end
            depth += 1
        end
        test(!initial.supportsClassGraphDepthMax())
    rescue Ice::UnknownLocalException
        # Expected marshal exception from the server (max class graph depth reached)
    rescue Ice::UnknownException
        # Expected stack overflow from the server (Java only)
    end
    initial.setRecursive(Test::Recursive.new)
    puts "ok"

    print "testing compact ID... "
    STDOUT.flush
    begin
        r = initial.getCompact()
        test(r != nil)
    rescue Ice::OperationNotExistException
    end
    puts "ok"

    print "testing marshaled results..."
    STDOUT.flush
    b1 = initial.getMB()
    test(b1 != nil && b1.theB == b1);
    b1 = initial.getAMDMB()
    test(b1 != nil && b1.theB == b1);
    puts "ok"

    print "testing UnexpectedObjectException... "
    STDOUT.flush
    ref = "uoet:#{helper.getTestEndpoint()}"
    base = communicator.stringToProxy(ref)
    test(base)
    uoet = Test::UnexpectedObjectExceptionTestPrx::uncheckedCast(base)
    test(uoet)
    begin
        uoet.op()
        test(false)
    rescue Ice::UnexpectedObjectException => ex
        test(ex.type == "::Test::AlsoEmpty")
        test(ex.expectedType == "::Test::Empty")
    rescue Ice::UnmarshalOutOfBoundsException => ex
        # This test raises Ice::UnmarshalOutOfBoundsException on Windows when the
        # server is compiled with VC6.
        test(RUBY_PLATFORM =~ /(win|w)32$/)
    rescue Ice::Exception => ex
        puts $!
        print ex.backtrace.join("\n")
        test(false)
    rescue => ex
        puts $!
        print ex.backtrace.join("\n")
        test(false)
    end
    puts "ok"

    print "testing getting ObjectFactory... "
    STDOUT.flush
    test(communicator.findObjectFactory('TestOF') != nil)
    puts "ok"

    print "testing getting ObjectFactory as ValueFactory... "
    STDOUT.flush
    test(communicator.getValueFactoryManager().find('TestOF') != nil)
    puts "ok"

    return initial
end
