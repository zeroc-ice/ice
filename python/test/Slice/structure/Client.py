#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "python", "Ice", "__init__.py")):
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
    def_s2 = Test.S2(True, 98, 99, 100, 101, "string", (1, 2, 3), Test.S1("name"))

    #
    # Compare default-constructed structures.
    #
    test(Test.S2() == Test.S2())

    #
    # Change one member at a time.
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
    v1.seq = copy.copy(def_s2.seq)
    test(v1 == def_s2)

    v1 = copy.copy(def_s2)
    v1.seq = ()
    test(v1 != def_s2)

    v1 = copy.copy(def_s2)
    v1.seq = (1, 2, 3)
    test(v1 == def_s2)

    v1 = copy.copy(def_s2)
    v2 = copy.copy(def_s2)
    v1.seq = None
    test(v1 != v2)

    v1 = copy.copy(def_s2)
    v2 = copy.copy(def_s2)
    v2.seq = None
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
    # Define some default values.
    #
    def_s3 = Test.S3(Test.C("name"), {"1":"2"}, communicator.stringToProxy("test"))

    #
    # Compare default-constructed structures.
    #
    test(Test.S3() == Test.S3())

    #
    # Change one member at a time.
    #
    v1 = copy.copy(def_s3)
    test(v1 == def_s3)

    v1.obj = None
    test(v1 != def_s3)

    v1.obj = Test.C("name")
    test(v1 != def_s3)

    v1 = copy.copy(def_s3)
    v1.sd = copy.copy(def_s3.sd)
    test(v1 == def_s3)

    v1.sd = None
    test(v1 != def_s3)

    v1.sd = {"1":"3"}
    test(v1 != def_s3)

    v1 = copy.copy(def_s3)
    v1.prx = None
    test(v1 != def_s3)

    v1.prx = communicator.stringToProxy("test")
    test(v1 == def_s3)

    v1.prx = communicator.stringToProxy("test2")
    test(v1 != def_s3)

    print("ok")

def run(args, communicator):
    allTests(communicator)

    return True

try:
    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(sys.argv)
    with Ice.initialize(sys.argv, initData) as communicator:
        status = run(sys.argv, communicator)
except:
    traceback.print_exc()
    status = False

sys.exit(not status)
