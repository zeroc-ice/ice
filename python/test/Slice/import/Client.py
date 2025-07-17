#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys

from Test.SubA.SubSubA1 import Value1 as Test_SubA_SubSubA1_Value1
from Test.SubA.SubSubA1 import Value2 as Test_SubA_SubSubA1_Value2
from Test.SubA.SubSubA2 import Value1 as Test_SubA_SubSubA2_Value1
from Test.SubB.SubSubB1 import Value1 as Test_SubB_SubSubB1_Value1
from Test.SubB.SubSubB1 import Value2 as Test_SubB_SubSubB1_Value2
from TestHelper import TestHelper


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


class Client(TestHelper):
    def run(self, args):
        sys.stdout.write("testing imports... ")
        sys.stdout.flush()
        test(Test_SubA_SubSubA1_Value1 == 10)
        test(Test_SubA_SubSubA1_Value2 == 11)
        test(Test_SubA_SubSubA2_Value1 == 30)
        test(Test_SubB_SubSubB1_Value1 == 20)
        test(Test_SubB_SubSubB1_Value2 == 21)

        print("ok")
