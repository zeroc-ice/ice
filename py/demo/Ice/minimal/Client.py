#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, Ice

Ice.loadSlice('Hello.ice')
import Demo

status = 0
communicator = None
try:
    communicator = Ice.initialize(sys.argv)
    hello = Demo.HelloPrx.checkedCast(communicator.stringToProxy("hello:tcp -p 10000"))
    if not hello:
        print args[0] + ": invalid proxy"
        status = 1
    else:
        hello.sayHello()
except:
    traceback.print_exc()
    status = 1

if communicator:
    try:
        communicator.destroy()
    except:
        traceback.print_exc()
        status = 1

sys.exit(status)
