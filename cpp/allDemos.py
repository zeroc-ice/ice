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
    "IceBox/hello",
    "IceStorm/counter",
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
    "Freeze/transform",
    "Freeze/casino",
    "Manual/map_filesystem",
    "Manual/evictor_filesystem",
    "Manual/simple_filesystem",
    "Manual/printer",
    "Manual/lifecycle"]

#
# For this demos the descriptor hardcode the icebox name and will
# not work with configurations that use different icebox names.
#
if Util.getIceBox().endswith("icebox"):
   demos += ["IceStorm/replicated", "IceGrid/icebox"]

#
# Freeze backup doesn't work on x86 multiarch because it require to
# use x86 db tools that are currently not available for x64 
# distributions.
#
if not Util.isX86():
    demos += ["Freeze/backup"]

if __name__ == "__main__":
    Util.run(demos)
