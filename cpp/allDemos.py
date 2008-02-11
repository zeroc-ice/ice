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

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.abspath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "DemoUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import DemoUtil

#
# List of all basic demos.
#
demos = [
    "IceUtil/workqueue",
    "Ice/async",
    "Ice/bidir",
    "Ice/callback",
    "Ice/converter",
    "Ice/hello",
    "Ice/invoke",
    "Ice/latency",
    "Ice/minimal",
    "Ice/multicast",
    "Ice/nested",
    "Ice/session",
    "Ice/throughput",
    "Ice/value",
    "IceBox/hello",
    "IceStorm/clock",
    "IceStorm/counter",
    "IceStorm/replicated",
    "IceGrid/simple",
    "IceGrid/allocate",
    "IceGrid/sessionActivation",
    "IceGrid/replication",
    "IceGrid/icebox",
    "Glacier2/chat",
    "Glacier2/callback",
    "Freeze/bench",
    "Freeze/customEvictor",
    "Freeze/phonebook",
    "Freeze/library",
    "Freeze/backup",
    "Freeze/transform",
    "Freeze/casino",
    "book/freeze_filesystem",
    "book/simple_filesystem",
    "book/printer",
    "book/lifecycle",
]

#
# These demos are currently disabled on cygwin
#
if DemoUtil.isCygwin() == 0:
    demos += [ ]

DemoUtil.run(demos)
