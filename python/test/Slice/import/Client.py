#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

import Test

status = True

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

try:
    sys.stdout.write("testing imports... ")
    sys.stdout.flush()

    test(Test.SubA.SubSubA1.Value1 == 10)
    test(Test.SubA.SubSubA1.Value2 == 11)
    test(Test.SubA.SubSubA2.Value1 == 30)
    test(Test.SubB.SubSubB1.Value1 == 20)
    test(Test.SubB.SubSubB1.Value2 == 21)

    print("ok")
except:
    traceback.print_exc()
    status = False

sys.exit(not status)
