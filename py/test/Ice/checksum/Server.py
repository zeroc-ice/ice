# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os, traceback, Ice

if not os.environ.has_key('ICE_HOME'):
    print sys.argv[0] + ': ICE_HOME is not defined'
    sys.exit(1)

Ice.loadSlice('-I' + os.environ['ICE_HOME'] + '/slice --checksum Test.ice STypes.ice')
import Test

class ChecksumI(Test.Checksum):
    def __init__(self, adapter):
        self._adapter = adapter

    def getSliceChecksums(self, current=None):
        return Ice.sliceChecksums

    def shutdown(self, current=None):
        self._adapter.getCommunicator().shutdown()

def run(args, communicator):
    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12345 -t 10000")
    adapter = communicator.createObjectAdapter("TestAdapter")
    object = ChecksumI(adapter)
    adapter.add(object, Ice.stringToIdentity("test"))
    adapter.activate()
    communicator.waitForShutdown()
    return True

try:
    communicator = Ice.initialize(sys.argv)
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
