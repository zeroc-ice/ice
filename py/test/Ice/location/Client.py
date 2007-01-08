#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
import Test, AllTests

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def run(args, communicator):
    AllTests.allTests(communicator, "ServerManager:default -p 12010 -t 10000")
    return True

try:
    data = Ice.InitializationData()
    data.properties = Ice.createProperties(sys.argv)
    data.properties.setProperty("Ice.Default.Locator", "locator:default -p 12010")
    communicator = Ice.initialize(sys.argv, data)
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
