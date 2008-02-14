#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys

for toplevel in [".", "..", "../..", "../../..", "../../../..", "../../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil
TestUtil.processCmdLine()

testdir = os.path.dirname(os.path.abspath(__file__))

workqueue = os.path.join(testdir, "workqueue")

print "starting workqueue...",
if TestUtil.debug:
    print "(" + workqueue + ")",
workqueuePipe = os.popen(workqueue + " 2>&1")
print "ok"

TestUtil.printOutputFromPipe(workqueuePipe)
    
workqueueStatus = TestUtil.closePipe(workqueuePipe)

if workqueueStatus:
    sys.exit(1)

match = os.path.join(testdir, "match")

print "starting signal match...",
matchPipe = os.popen(match + " 2>&1")
print "ok"

TestUtil.printOutputFromPipe(matchPipe)

matchStatus = TestUtil.closePipe(matchPipe)

if matchStatus:
    sys.exit(1)

print "starting broadcast match...",
matchPipe = os.popen(match + " -b" + " 2>&1")
print "ok"

TestUtil.printOutputFromPipe(matchPipe)

matchStatus = TestUtil.closePipe(matchPipe)

if matchStatus:
    sys.exit(1)

sys.exit(0)
