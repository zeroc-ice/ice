#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys

path = [ ".", "..", "../..", "../../..", "../../../..", "../../../../..", "../../../../../..",
         "../../../../../../..", "../../../../../../../..", "../../../../../../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil

TestUtil.queueClientServerTest(additionalClientOptions = "--Ice.Warn.AMICallback=0")

TestUtil.queueClientServerTest(configName = "amd", localOnly = True, message = "Running test with AMD server.",
                               additionalClientOptions = "--Ice.Warn.AMICallback=0",
                               server="test.Ice.operations.AMDServer")

TestUtil.queueCollocatedTest()
TestUtil.runQueuedTests()
