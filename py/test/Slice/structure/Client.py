#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "python", "Ice.py")):
        break
else:
    raise RuntimeError("can't find toplevel directory!")

import Ice

Ice.loadSlice('Test.ice')
import Test, copy

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def allTests(communicator):
    sys.stdout.write("testing equals() for Slice structures... ")
    sys.stdout.flush()

    #
    # Define some default values.
    #
    def_s2 = Test.S2(True, 98, 99, 100, 101, 1.0, 2.0, "string", ("one", "two", "three"), {"abc":"def"}, \
                     Test.S1("name"), Test.C(5), communicator.stringToProxy("test"))

    #
    # Compare default-constructed structures.
    #
    test(Test.S2() == Test.S2())

    #
    # Change one primitive member at a time.
    #
    v = copy.copy(def_s2)
    test(v == def_s2)

    v = copy.copy(def_s2)
    v.bo = False
    test(v != def_s2)

    v = copy.copy(def_s2)
    v.by = v.by - 1
    test(v != def_s2)

    v = copy.copy(def_s2)
    v.sh = v.sh - 1
    test(v != def_s2)

    v = copy.copy(def_s2)
    v.i = v.i - 1
    test(v != def_s2)

    v = copy.copy(def_s2)
    v.l = v.l - 1
    test(v != def_s2)

    v = copy.copy(def_s2)
    v.f = v.f - 1
    test(v != def_s2)

    v = copy.copy(def_s2)
    v.d = v.d - 1
    test(v != def_s2)

    v = copy.copy(def_s2)
    v.str = ""
    test(v != def_s2)

    #
    # String member
    #
    v1 = copy.copy(def_s2)
    v1.str = "string"
    test(v1 == def_s2)

    v1 = copy.copy(def_s2)
    v2 = copy.copy(def_s2)
    v1.str = None
    test(v1 != v2)

    v1 = copy.copy(def_s2)
    v2 = copy.copy(def_s2)
    v2.str = None
    test(v1 != v2)

    v1 = copy.copy(def_s2)
    v2 = copy.copy(def_s2)
    v1.str = None
    v2.str = None
    test(v1 == v2)

    #
    # Sequence member
    #
    v1 = copy.copy(def_s2)
    v1.ss = copy.copy(def_s2.ss)
    test(v1 == def_s2)

    v1 = copy.copy(def_s2)
    v1.ss = []
    test(v1 != def_s2)

    v1 = copy.copy(def_s2)
    v1.ss = ("one", "two", "three")
    test(v1 == def_s2)

    v1 = copy.copy(def_s2)
    v2 = copy.copy(def_s2)
    v1.ss = None
    test(v1 != v2)

    v1 = copy.copy(def_s2)
    v2 = copy.copy(def_s2)
    v2.ss = None
    test(v1 != v2)

    #
    # Dictionary member
    #
    v1 = copy.copy(def_s2)
    v1.sd = {"abc":"def"}
    test(v1 == def_s2)

    v1 = copy.copy(def_s2)
    v1.sd = {}
    test(v1 != def_s2)

    v1 = copy.copy(def_s2)
    v2 = copy.copy(def_s2)
    v1.sd = None
    test(v1 != v2)

    v1 = copy.copy(def_s2)
    v2 = copy.copy(def_s2)
    v2.sd = None
    test(v1 != v2)

    #
    # Struct member
    #
    v1 = copy.copy(def_s2)
    v1.s = copy.copy(def_s2.s)
    test(v1 == def_s2)

    v1 = copy.copy(def_s2)
    v1.s = Test.S1("name")
    test(v1 == def_s2)

    v1 = copy.copy(def_s2)
    v1.s = Test.S1("noname")
    test(v1 != def_s2)

    v1 = copy.copy(def_s2)
    v2 = copy.copy(def_s2)
    v1.s = None
    test(v1 != v2)

    v1 = copy.copy(def_s2)
    v2 = copy.copy(def_s2)
    v2.s = None
    test(v1 != v2)

    #
    # Class member
    #
    v1 = copy.copy(def_s2)
    v1.cls = copy.copy(def_s2.cls)
    test(v1 != def_s2)

    v1 = copy.copy(def_s2)
    v2 = copy.copy(def_s2)
    v1.cls = None
    test(v1 != v2)

    v1 = copy.copy(def_s2)
    v2 = copy.copy(def_s2)
    v2.cls = None
    test(v1 != v2)

    #
    # Proxy member
    #
    v1 = copy.copy(def_s2)
    v1.prx = communicator.stringToProxy("test")
    test(v1 == def_s2)

    v1 = copy.copy(def_s2)
    v1.prx = communicator.stringToProxy("test2")
    test(v1 != def_s2)

    v1 = copy.copy(def_s2)
    v2 = copy.copy(def_s2)
    v1.prx = None
    test(v1 != v2)

    v1 = copy.copy(def_s2)
    v2 = copy.copy(def_s2)
    v2.prx = None
    test(v1 != v2)

    print("ok")

def run(args, communicator):
    allTests(communicator)

    return True

try:
    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(sys.argv)
    communicator = Ice.initialize(sys.argv, initData)
    status = run(sys.argv, communicator)
except:
    traceback.print_exc()
    status = False

if communicator:
    try:
        communicator.destroy()
    except:
        traceback.print_exc()
        status = False

sys.exit(not status)
