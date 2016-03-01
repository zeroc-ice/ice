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

TestUtil.queueClientServerTest(configName = "sliced", message = "Running test with sliced format.")

TestUtil.queueClientServerTest(configName = "1.0", message = "Running test with 1.0 encoding.",
                               additionalClientOptions="--Ice.Default.EncodingVersion=1.0", 
                               additionalServerOptions="--Ice.Default.EncodingVersion=1.0")

TestUtil.queueClientServerTest(configName = "slicedAMD", localOnly = True,
                               message = "Running test with sliced format and AMD server.",
                               server="test.Ice.slicing.exceptions.AMDServer")

TestUtil.queueClientServerTest(configName = "1.0AMD", localOnly = True,
                               message = "Running test with 1.0 encoding and AMD server.",
                               server="test.Ice.slicing.exceptions.AMDServer", 
                               additionalClientOptions="--Ice.Default.EncodingVersion=1.0", 
                               additionalServerOptions="--Ice.Default.EncodingVersion=1.0")

TestUtil.runQueuedTests()
