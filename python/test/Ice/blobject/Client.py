#!/usr/bin/env python
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from TestHelper import TestHelper
TestHelper.loadSlice('Test.ice')
import sys
import Ice
import Test
import RouterI


def test(b):
    if not b:
        raise RuntimeError('test assertion failed')


class Client(TestHelper):

    def allTests(self, communicator, sync):
        hello = Test.HelloPrx.checkedCast(communicator.stringToProxy("test:{0}".format(self.getTestEndpoint())))
        hello.sayHello(False)
        hello.sayHello(False, {"_fwd": "o"})
        test(hello.add(10, 20) == 30)
        try:
            hello.raiseUE()
            test(False)
        except Test.UE:
            pass

        try:
            Test.HelloPrx.checkedCast(communicator.stringToProxy("unknown:{0} -t 10000".format(self.getTestEndpoint())))
            test(False)
        except Ice.ObjectNotExistException:
            pass

        # First try an object at a non-existent endpoint.
        try:
            Test.HelloPrx.checkedCast(communicator.stringToProxy("missing:default -p 12000 -t 10000"))
            test(False)
        except Ice.UnknownLocalException as e:
            test(e.unknown.find('ConnectionRefusedException'))
            if sync:
                hello.shutdown()

    def run(self, args):

        properties = self.createTestProperties(args)
        properties.setProperty('Ice.Warn.Dispatch', '0')
        with self.initialize(properties=properties) as communicator:
            router = RouterI.RouterI(communicator, False)
            sys.stdout.write("testing async blobject... ")
            sys.stdout.flush()
            self.allTests(communicator, False)
            print("ok")
            router.destroy()

        with self.initialize(properties=properties) as communicator:
            router = RouterI.RouterI(communicator, True)
            sys.stdout.write("testing sync blobject... ")
            sys.stdout.flush()
            self.allTests(communicator, True)
            print("ok")
            router.destroy()
