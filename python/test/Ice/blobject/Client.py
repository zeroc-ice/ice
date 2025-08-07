#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice("Test.ice")

import RouterI
from generated.test.Ice.blobject import Test
from TestHelper import test

import Ice


class Client(TestHelper):
    def allTests(self, communicator: Ice.Communicator, sync: bool):
        hello = Test.HelloPrx(communicator, f"test:{self.getTestEndpoint()}")
        hello.sayHello(False)
        hello.sayHello(False, {"_fwd": "o"})
        test(hello.add(10, 20) == 30)
        try:
            hello.raiseUE()
            test(False)
        except Test.UE:
            pass

        try:
            Test.HelloPrx.checkedCast(Ice.ObjectPrx(communicator, f"unknown:{self.getTestEndpoint()} -t 10000"))
            test(False)
        except Ice.ObjectNotExistException:
            pass

        # First try an object at a non-existent endpoint.
        try:
            Test.HelloPrx.checkedCast(Ice.ObjectPrx(communicator, "missing:default -p 12000 -t 10000"))
            test(False)
        except Ice.UnknownLocalException as e:
            test(str(e).find("ConnectionRefusedException"))
            if sync:
                hello.shutdown()

    def run(self, args: list[str]):
        properties = self.createTestProperties(args)
        properties.setProperty("Ice.Warn.Dispatch", "0")
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
