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

Ice.loadSlice('-I' + os.environ['ICE_HOME'] + '/slice --checksum Test.ice CTypes.ice')
import Test
import AllTests

def run(args, communicator):
    checksum = AllTests.allTests(communicator)
    checksum.shutdown()
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
