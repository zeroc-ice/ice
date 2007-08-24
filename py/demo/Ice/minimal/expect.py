#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os

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
demoscript.Util.defaultLanguage = "Python"
import demoscript.pexpect as pexpect

server = demoscript.Util.spawn('Server.py --Ice.PrintAdapterReady')
server.expect('.* ready')

print "testing...",
sys.stdout.flush()
client = demoscript.Util.spawn('Client.py')
client.waitTestSuccess()
server.expect('Hello World!')
print "ok"

import signal
server.kill(signal.SIGTERM)
server.expect(pexpect.EOF)
server.wait()
assert server.signalstatus == signal.SIGTERM
