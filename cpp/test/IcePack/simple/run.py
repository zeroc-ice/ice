#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2001
# MutableRealms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

import os, sys

for toplevel in ["", "..", os.path.join("..", ".."), os.path.join("..", "..", "..")]:
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

icePack = os.path.join(toplevel, "bin", "icepack")
icePackAdmin = os.path.join(toplevel, "bin", "icepackadmin")

print "starting icepack...",
icePackPipe = os.popen(icePack + ' --nowarn --pid' + \
                       ' "--Ice.Adapter.Forward.Endpoints=tcp -p 12346 -t 2000"' + \
                       ' "--Ice.Adapter.Admin.Endpoints=tcp -p 12347 -t 2000"')
output = icePackPipe.readline().strip()
if not output:
    print "failed!"
    sys.exit(0)
TestUtil.serverPids.append(int(output))
print "ok"

print "registering server with icepack...",
icePackAdminPipe = os.popen(icePackAdmin + \
                            ' "--Ice.Adapter.Admin.Endpoints=tcp -p 12347 -t 2000"' + \
                            ' add "test:tcp -p 12345 -t 2000"')
icePackAdminPipe.close()
print "ok"

name = os.path.join("IcePack", "simple")
TestUtil.clientServerTest(toplevel, name)
TestUtil.collocatedTest(toplevel, name)

print "shutting down icepack...",
icePackAdminPipe = os.popen(icePackAdmin + \
                            ' "--Ice.Adapter.Admin.Endpoints=tcp -p 12347 -t 2000"' + \
                            ' shutdown')
icePackAdminPipe.close()
print "ok"

sys.exit(1)
