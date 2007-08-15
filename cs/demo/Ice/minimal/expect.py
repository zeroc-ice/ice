#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import pexpect, sys, os

try:
    import demoscript
except ImportError:
    for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
        toplevel = os.path.normpath(toplevel)
        if os.path.exists(os.path.join(toplevel, "demoscript")):
            break
    else:
        raise "can't find toplevel directory!"
    sys.path.append(os.path.join(toplevel))
    import demoscript

import demoscript.Util

server = demoscript.Util.spawn('%sserver.exe --Ice.PrintAdapterReady' % (demoscript.Util.mono()))
server.expect('.* ready')

print "testing...",
sys.stdout.flush()
client = demoscript.Util.spawn('%sclient.exe' % (demoscript.Util.mono()))
client.expect(pexpect.EOF)
server.expect('Hello World!')
print "ok"
