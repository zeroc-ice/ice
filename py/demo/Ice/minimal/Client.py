#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, Ice

Ice.loadSlice('Hello.ice')
import Demo

try:
    communicator = Ice.initialize(sys.argv)
    hello = Demo.HelloPrx.checkedCast(communicator.stringToProxy("hello:default -h localhost -p 10000"))
    hello.sayHello()
    communicator.destroy()
except:
    traceback.print_exc()
    sys.exit(1)
