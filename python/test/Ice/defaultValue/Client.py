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

import Ice

Ice.loadSlice('Test.ice')
import AllTests

def run(args):
    AllTests.allTests()
    return True

try:
    status = run(sys.argv)
except:
    traceback.print_exc()
    status = False

sys.exit(not status)
