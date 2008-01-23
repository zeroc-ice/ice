#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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
demoscript.Util.defaultLanguage = "VB"
import signal

server = demoscript.Util.spawn('server.exe --Ice.PrintAdapterReady')
server.expect('.* ready')

print "testing...",
sys.stdout.flush()
client = demoscript.Util.spawn('client.exe')
client.expect('Contents of root directory:\r{1,2}\n.*Down to a sunless sea.')
client.waitTestSuccess()
server.kill(signal.SIGINT)
server.waitTestSuccess()
print "ok"
