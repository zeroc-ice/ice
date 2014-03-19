#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.abspath(toplevel)
    if os.path.exists(os.path.join(toplevel, "demoscript")):
        break
else:
    raise RuntimeError("can't find toplevel directory!")

sys.path.append(os.path.join(toplevel))
from demoscript import Util

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
    "Ice/interleaved",
    "Ice/invoke",
    "Ice/latency",
    "Ice/minimal",
    "Ice/multicast",
    "Ice/nested",
    "Ice/nrvo",
    "Ice/plugin",
    "Ice/session",
    "Ice/throughput",
    "Ice/value",
    "Ice/properties",
    "Ice/optional",
    "IceStorm/clock",
    "IceStorm/counter",
    "IceStorm/replicated",
    "IceStorm/replicated2",
    "IceGrid/simple",
    "IceGrid/allocate",
    "IceGrid/sessionActivation",
    "IceGrid/replication",
    "IceGrid/secure",
    "Glacier2/chat",
    "Glacier2/callback",
    "Freeze/bench",
    "Freeze/customEvictor",
    "Freeze/phonebook",
    "Freeze/library",
    "Freeze/backup",
    "Freeze/transform",
    "Freeze/casino",
    "book/map_filesystem",
    "book/evictor_filesystem",
    "book/simple_filesystem",
    "book/printer",
    "book/lifecycle",
]

if not Util.isNoServices():
    demos += ["IceBox/hello", "IceGrid/icebox"]

if __name__ == "__main__":
    Util.run(demos)
