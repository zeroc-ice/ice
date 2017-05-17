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

import Ice

Ice.loadSlice('Test.ice')
import Test, copy

status = True

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

try:
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
except:
    traceback.print_exc()
    status = False

sys.exit(not status)
