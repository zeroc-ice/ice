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

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    if os.path.exists(os.path.normpath(toplevel + "/config/TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.normpath(toplevel + "/config"))
import TestUtil

icePack = os.path.normpath(toplevel + "/bin/icepack")
icePackAdmin = os.path.normpath(toplevel + "/bin/icepackadmin")

print "starting icepack...",
icePackPipe = os.popen(icePack + ' --Ice.PrintProcessId --nowarn' + \
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

name = "IcePack/simple"
TestUtil.clientServerTest(toplevel, name)
TestUtil.collocatedTest(toplevel, name)

print "shutting down icepack...",
icePackAdminPipe = os.popen(icePackAdmin + \
                            ' "--Ice.Adapter.Admin.Endpoints=tcp -p 12347 -t 2000"' + \
                            ' shutdown')
icePackAdminPipe.close()
print "ok"

sys.exit(1)
