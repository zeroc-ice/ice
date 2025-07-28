#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice("Test.ice")

from generated.test.Slice.macros import Test

import Ice


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


class Client(TestHelper):
    def run(self, args):
        sys.stdout.write("testing Slice predefined macros... ")
        sys.stdout.flush()

        d = Test.Default()
        test(d.x == 10)
        test(d.y == 10)

        nd = Test.NoDefault()
        test(nd.x != 10)
        test(nd.y != 10)

        c = Test.PythonOnly()
        test(c.lang == "python")
        test(c.version == Ice.intVersion())
        print("ok")
