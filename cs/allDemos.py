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
    "Ice/async",
    "Ice/bidir",
    "Ice/callback",
    "Ice/hello",
    "Ice/invoke",
    "Ice/latency",
    "Ice/minimal",
    "Ice/multicast",
    "Ice/nested",
    "Ice/plugin",
    "Ice/serialize",
    "Ice/session",
    "Ice/throughput",
    "Ice/value",
    "Ice/optional",
    "Ice/context",
    "IceDiscovery/hello",
    "IceDiscovery/replication",
    "IceBox/hello",
    "IceStorm/clock",
    "IceGrid/simple",
    "IceGrid/icebox",
    "Glacier2/callback",
    "Manual/simple_filesystem",
    "Manual/printer",
    "Manual/lifecycle",
    ]

if __name__ == "__main__":
    Util.run(demos)
