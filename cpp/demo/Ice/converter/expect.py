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

server = demoscript.Util.spawn('./server --Ice.PrintAdapterReady')
server.expect('.* ready')
client = demoscript.Util.spawn('./client')
client.expect('.*==>')

print "testing with conversion... ",
sys.stdout.flush()
client.sendline('u')
server.expect('Received \\(UTF-8\\): "Bonne journ\\\\351e"')
client.expect('Received \\(LATIN-1\\): "Bonne journ\\\\303\\\\251e"')
print "ok"

print "testing without conversion... ",
client.sendline('t')
server.expect('Received \\(UTF-8\\): "Bonne journ\\\\303\\\\251e"')
client.expect('Received \\(LATIN-1\\): "Bonne journ\\\\351e"')
print "ok"

client.sendline('s')
server.expect(pexpect.EOF)
assert server.wait() == 0

client.sendline('x')
client.expect(pexpect.EOF)
assert client.wait() == 0
