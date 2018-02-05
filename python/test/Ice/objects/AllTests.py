# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, TestI, sys

def MyValueFactory(type):
    if type == '::Test::B':
        return TestI.BI()
    elif type == '::Test::C':
        return TestI.CI()
    elif type == '::Test::D':
        return TestI.DI()
    elif type == '::Test::E':
        return TestI.EI()
    elif type == '::Test::F':
        return TestI.FI()
    elif type == '::Test::I':
        return TestI.II()
    elif type == '::Test::J':
        return TestI.JI()
    elif type == '::Test::H':
        return TestI.HI()
    assert(False) # Should never be reached

class MyObjectFactory(Ice.ObjectFactory):
    def create(self, type):
        return None

    def destroy():
        pass

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def allTests(communicator):
    communicator.getValueFactoryManager().add(MyValueFactory, '::Test::B')
    communicator.getValueFactoryManager().add(MyValueFactory, '::Test::C')
    communicator.getValueFactoryManager().add(MyValueFactory, '::Test::D')
    communicator.getValueFactoryManager().add(MyValueFactory, '::Test::E')
    communicator.getValueFactoryManager().add(MyValueFactory, '::Test::F')
    communicator.getValueFactoryManager().add(MyValueFactory, '::Test::I')
    communicator.getValueFactoryManager().add(MyValueFactory, '::Test::J')
    communicator.getValueFactoryManager().add(MyValueFactory, '::Test::H')

    communicator.addObjectFactory(MyObjectFactory(), "TestOF")

    sys.stdout.write("testing stringToProxy... ")
    sys.stdout.flush()
    ref = "initial:default -p 12010"
    base = communicator.stringToProxy(ref)
    test(base)
    print("ok")

    sys.stdout.write("testing checked cast... ")
    sys.stdout.flush()
    initial = Test.InitialPrx.checkedCast(base)
    test(initial)
    test(initial == base)
    print("ok")

    sys.stdout.write("getting B1... ")
    sys.stdout.flush()
    b1 = initial.getB1()
    test(b1)
    print("ok")

    sys.stdout.write("getting B2... ")
    sys.stdout.flush()
    b2 = initial.getB2()
    test(b2)
    print("ok")

    sys.stdout.write("getting C... ")
    sys.stdout.flush()
    c = initial.getC()
    test(c)
    print("ok")

    sys.stdout.write("getting D... ")
    sys.stdout.flush()
    d = initial.getD()
    test(d)
    print("ok")

    sys.stdout.write("testing protected members... ")
    sys.stdout.flush()
    e = initial.getE()
    test(e.checkValues())
    test(e._i == 1)
    test(e._s == "hello")
    f = initial.getF()
    test(f.checkValues())
    test(f.e2.checkValues())
    test(f._e1.checkValues())
    print("ok")

    sys.stdout.write("getting I, J, H... ")
    sys.stdout.flush()
    i = initial.getI()
    test(i)
    j = initial.getJ()
    test(isinstance(j, TestI.JI))
    h = initial.getH()
    test(isinstance(h, Test.H))
    print("ok")

    sys.stdout.write("getting D1... ")
    sys.stdout.flush()
    d1 = initial.getD1(Test.D1(Test.A1("a1"), Test.A1("a2"), Test.A1("a3"), Test.A1("a4")))
    test(d1.a1.name == "a1")
    test(d1.a2.name == "a2")
    test(d1.a3.name == "a3")
    test(d1.a4.name == "a4")
    print("ok")

    sys.stdout.write("throw EDerived... ")
    sys.stdout.flush()
    try:
        initial.throwEDerived()
        test(false)
    except Test.EDerived as e:
        test(e.a1.name == "a1")
        test(e.a2.name == "a2")
        test(e.a3.name == "a3")
        test(e.a4.name == "a4")
    print("ok")

    sys.stdout.write("setting I... ")
    sys.stdout.flush()
    initial.setI(TestI.II())
    initial.setI(TestI.JI())
    initial.setI(TestI.HI())
    print("ok")

    sys.stdout.write("checking consistency... ")
    sys.stdout.flush()
    test(b1 != b2)
    test(b1 != c)
    test(b1 != d)
    test(b2 != c)
    test(b2 != d)
    test(c != d)
    test(b1.theB == b1)
    test(b1.theC == None)
    test(isinstance(b1.theA, Test.B))
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
    test(d.theC == None)
    print("ok")

    sys.stdout.write("getting B1, B2, C, and D all at once... ")
    sys.stdout.flush()
    b1, b2, c, d = initial.getAll()
    test(b1)
    test(b2)
    test(c)
    test(d)
    print("ok")

    sys.stdout.write("checking consistency... ")
    sys.stdout.flush()
    test(b1 != b2)
    test(b1 != c)
    test(b1 != d)
    test(b2 != c)
    test(b2 != d)
    test(c != d)
    test(b1.theA == b2)
    test(b1.theB == b1)
    test(b1.theC == None)
    test(b2.theA == b2)
    test(b2.theB == b1)
    test(b2.theC == c)
    test(c.theB == b2)
    test(d.theA == b1)
    test(d.theB == b2)
    test(d.theC == None)
    test(d.preMarshalInvoked)
    test(d.postUnmarshalInvoked)
    test(d.theA.preMarshalInvoked)
    test(d.theA.postUnmarshalInvoked)
    test(d.theB.preMarshalInvoked)
    test(d.theB.postUnmarshalInvoked)
    test(d.theB.theC.preMarshalInvoked)
    test(d.theB.theC.postUnmarshalInvoked)
    print("ok")

    sys.stdout.write("testing sequences... ")
    try:
        sys.stdout.flush()
        initial.opBaseSeq([])

        retS, outS = initial.opBaseSeq([Test.Base()])
        test(len(retS) == 1 and len(outS) == 1)
    except Ice.OperationNotExistException:
        pass
    print("ok")

    sys.stdout.write("testing recursive type... ")
    sys.stdout.flush()
    top = Test.Recursive()
    p = top;
    depth = 0;
    try:
        while depth <= 700:
            p.v = Test.Recursive()
            p = p.v;
            if (depth < 10 and (depth % 10) == 0) or \
               (depth < 1000 and (depth % 100) == 0) or \
               (depth < 10000 and (depth % 1000) == 0) or \
               (depth % 10000) == 0:
                initial.setRecursive(top)
            depth += 1
        test(not initial.supportsClassGraphDepthMax())
    except Ice.UnknownLocalException:
        # Expected marshal exception from the server (max class graph depth reached)
        pass
    except Ice.UnknownException:
        # Expected stack overflow from the server (Java only)
        pass
    initial.setRecursive(Test.Recursive())
    print("ok")

    sys.stdout.write("testing compact ID... ")
    sys.stdout.flush()
    try:
        r = initial.getCompact()
        test(r)
    except Ice.OperationNotExistException:
        pass
    print("ok")

    sys.stdout.write("testing marshaled results...")
    sys.stdout.flush()
    b1 = initial.getMB()
    test(b1 != None and b1.theB == b1)
    b1 = initial.getAMDMBAsync().result()
    test(b1 != None and b1.theB == b1)
    print("ok")

    # Don't run this test with collocation, this should work with collocation
    # but the test isn't written to support it (we'd need support for the
    # streaming interface)
    if initial.ice_getConnection():
        sys.stdout.write("testing UnexpectedObjectException... ")
        sys.stdout.flush()
        ref = "uoet:default -p 12010"
        base = communicator.stringToProxy(ref)
        test(base)
        uoet = Test.UnexpectedObjectExceptionTestPrx.uncheckedCast(base)
        test(uoet)
        try:
            uoet.op()
            test(False)
        except Ice.UnexpectedObjectException as ex:
            test(ex.type == "::Test::AlsoEmpty")
            test(ex.expectedType == "::Test::Empty")
        except Ice.Exception as ex:
            print(ex)
            test(False)
        except:
            print(sys.exc_info())
            test(False)
        print("ok")

    sys.stdout.write("testing getting ObjectFactory... ")
    sys.stdout.flush()
    test(communicator.findObjectFactory("TestOF") != None)
    print("ok")

    sys.stdout.write("testing getting ObjectFactory as ValueFactory... ")
    sys.stdout.flush()
    test(communicator.getValueFactoryManager().find("TestOF") != None)
    print("ok")

    return initial
