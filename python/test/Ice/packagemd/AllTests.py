# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

import Ice, Test, Test1, testpkg, modpkg, sys, threading, time, traceback

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def allTests(helper, communicator):

    sys.stdout.write("testing stringToProxy... ")
    sys.stdout.flush()
    ref = "initial:{0}".format(helper.getTestEndpoint())
    base = communicator.stringToProxy(ref)
    test(base)
    print("ok")

    sys.stdout.write("testing checked cast... ")
    sys.stdout.flush()
    initial = Test.InitialPrx.checkedCast(base)
    test(initial)
    test(initial == base)
    print("ok")

    sys.stdout.write("testing types without package... ")
    sys.stdout.flush()
    c1 = initial.getTest1C2AsC1()
    test(c1)
    test(isinstance(c1, Test1.C2))
    c2 = initial.getTest1C2AsC2()
    test(c2)
    try:
        initial.throwTest1E2AsE1()
        test(False)
    except Test1.E1 as ex:
        test(isinstance(ex, Test1.E2))
    try:
        initial.throwTest1E2AsE2()
        test(False)
    except Test1.E2 as ex:
        # Expected
        pass
    try:
        initial.throwTest1Def()
        test(False)
    except Test1._def as ex:
        # Expected
        pass
    print("ok")

    sys.stdout.write("testing types with package... ")
    sys.stdout.flush()

    c1 = initial.getTest2C2AsC1()
    test(c1)
    test(isinstance(c1, testpkg.Test2.C2))
    c2 = initial.getTest2C2AsC2()
    test(c2)
    try:
        initial.throwTest2E2AsE1()
        test(False)
    except testpkg.Test2.E1 as ex:
        test(isinstance(ex, testpkg.Test2.E2))
    try:
        initial.throwTest2E2AsE2()
        test(False)
    except testpkg.Test2.E2 as ex:
        # Expected
        pass

    c1 = initial.getTest3C2AsC1()
    test(c1)
    test(isinstance(c1, modpkg.Test3.C2))
    c2 = initial.getTest3C2AsC2()
    test(c2)
    try:
        initial.throwTest3E2AsE1()
        test(False)
    except modpkg.Test3.E1 as ex:
        test(isinstance(ex, modpkg.Test3.E2))
    try:
        initial.throwTest3E2AsE2()
        test(False)
    except modpkg.Test3.E2 as ex:
        # Expected
        pass

    print("ok")

    return initial
