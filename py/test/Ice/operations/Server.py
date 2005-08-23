#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
    raise "can't find toplevel directory!"

sys.path.insert(0, os.path.join(toplevel, "python"))
sys.path.insert(0, os.path.join(toplevel, "lib"))

import Ice

#
# Find Slice directory.
#
slice_dir = os.getenv('ICEPY_HOME', '')
if len(slice_dir) == 0 or not os.path.exists(os.path.join(slice_dir, "slice")):
    slice_dir = os.getenv('ICE_HOME', '')
if len(slice_dir) == 0 or not os.path.exists(os.path.join(slice_dir, "slice")):
    print sys.argv[0] + ': Slice directory not found. Define ICEPY_HOME or ICE_HOME.'
    sys.exit(1)

Ice.loadSlice('-I' + slice_dir + '/slice Test.ice')
import Test

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class MyDerivedClassI(Test.MyDerivedClass):
    def __init__(self, adapter, identity):
        self.adapter = adapter
        self.identity = identity

    def shutdown(self, current=None):
        self.adapter.getCommunicator().shutdown()

    def opVoid(self, current=None):
        pass

    def opByte(self, p1, p2, current=None):
        return (p1, p1 ^ p2)

    def opBool(self, p1, p2, current=None):
        return (p2, p1)

    def opShortIntLong(self, p1, p2, p3, current=None):
        return (p3, p1, p2, p3)

    def opFloatDouble(self, p1, p2, current=None):
        return (p2, p1, p2)

    def opString(self, p1, p2, current=None):
        return (p1 + " " + p2, p2 + " " + p1)

    def opMyEnum(self, p1, current=None):
        return (Test.MyEnum.enum3, p1)

    def opMyClass(self, p1, current=None):
        return (Test.MyClassPrx.uncheckedCast(self.adapter.createProxy(self.identity)), p1,
                Test.MyClassPrx.uncheckedCast(self.adapter.createProxy(Ice.stringToIdentity("noSuchIdentity"))))

    def opStruct(self, p1, p2, current=None):
        p1.s.s = "a new string"
        return (p2, p1)

    def opByteS(self, p1, p2, current=None):
        p3 = p1[0:]
        p3.reverse()
        r = p1[0:]
        r.extend(p2)
        return (r, p3)

    def opBoolS(self, p1, p2, current=None):
        p3 = p1[0:]
        p3.extend(p2)
        r = p1[0:]
        r.reverse();
        return (r, p3)

    def opShortIntLongS(self, p1, p2, p3, current=None):
        p4 = p1[0:]
        p5 = p2[0:]
        p5.reverse()
        p6 = p3[0:]
        p6.extend(p3)
        return (p3, p4, p5, p6)

    def opFloatDoubleS(self, p1, p2, current=None):
        p3 = p1[0:]
        p4 = p2[0:]
        p4.reverse()
        r = p2[0:]
        r.extend(p1)
        return (r, p3, p4)

    def opStringS(self, p1, p2, current=None):
        p3 = p1[0:]
        p3.extend(p2)
        r = p1[0:]
        r.reverse()
        return (r, p3)

    def opByteSS(self, p1, p2, current=None):
        p3 = p1[0:]
        p3.reverse()
        r = p1[0:]
        r.extend(p2)
        return (r, p3)

    def opBoolSS(self, p1, p2, current=None):
        p3 = p1[0:]
        p3.extend(p2)
        r = p1[0:]
        r.reverse()
        return (r, p3)

    def opShortIntLongSS(self, p1, p2, p3, current=None):
        p4 = p1[0:]
        p5 = p2[0:]
        p5.reverse()
        p6 = p3[0:]
        p6.extend(p3)
        return (p3, p4, p5, p6)

    def opFloatDoubleSS(self, p1, p2, current=None):
        p3 = p1[0:]
        p4 = p2[0:]
        p4.reverse()
        r = p2[0:]
        r.extend(p2)
        return (r, p3, p4)

    def opStringSS(self, p1, p2, current=None):
        p3 = p1[0:]
        p3.extend(p2)
        r = p2[0:]
        r.reverse()
        return (r, p3)

    def opStringSSS(self, p1, p2, current=None):
        p3 = p1[0:]
        p3.extend(p2)
        r = p2[0:]
        r.reverse()
        return (r, p3)

    def opByteBoolD(self, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    def opShortIntD(self, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    def opLongFloatD(self, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    def opStringStringD(self, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    def opStringMyEnumD(self, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    def opIntS(self, s, current=None):
        return [-x for x in s]

    def opContext(self, current=None):
        return current.ctx

    def opDerived(self, current=None):
        pass

class TestCheckedCastI(Test.TestCheckedCast):
    def __init__(self):
	self.ctx = None

    def getContext(self, current):
	return self.ctx;

    def ice_isA(self, s, current):
	self.ctx = current.ctx
	return Test.TestCheckedCast.ice_isA(self, s, current)

def run(args, communicator):
    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12345 -t 10000")
    adapter = communicator.createObjectAdapter("TestAdapter")
    id = Ice.stringToIdentity("test")
    adapter.add(MyDerivedClassI(adapter, id), id)
    adapter.add(TestCheckedCastI(), Ice.stringToIdentity("context"))
    adapter.activate()
    communicator.waitForShutdown()
    return True

try:
    communicator = Ice.initialize(sys.argv)
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
