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
    "Ice/context",
    "Ice/converter",
    "Ice/hello",
    "Ice/interleaved",
    "Ice/invoke",
    "Ice/latency",
    "Ice/minimal",
    "Ice/multicast",
    "Ice/nested",
    "Ice/nrvo",
    "Ice/optional",
    "Ice/plugin",
    "Ice/properties",
    "Ice/session",
    "Ice/throughput",
    "Ice/value",
    "IceDiscovery/hello",
    "IceDiscovery/replication",
    "IceStorm/clock",
    "IceStorm/counter",
    "IceStorm/replicated",
    "IceStorm/replicated2",
    "IceGrid/allocate",
    "IceGrid/customLoadBalancing",
    "IceGrid/replication",
    "IceGrid/sessionActivation",
    "IceGrid/secure",
    "IceGrid/simple",
    "Glacier2/chat",
    "Glacier2/callback",
    "Freeze/bench",
    "Freeze/customEvictor",
    "Freeze/phonebook",
    "Freeze/library",
    "Freeze/backup",
    "Freeze/transform",
    "Freeze/casino",
    "Manual/map_filesystem",
    "Manual/evictor_filesystem",
    "Manual/simple_filesystem",
    "Manual/printer",
    "Manual/lifecycle"]

if not Util.isNoServices():
    demos += ["IceBox/hello", "IceGrid/icebox"]

if __name__ == "__main__":
    Util.run(demos)
