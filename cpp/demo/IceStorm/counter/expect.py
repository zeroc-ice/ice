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

import signal

print "cleaning databases...",
sys.stdout.flush()
demoscript.Util.cleanDbDir("db")
print "ok"

if demoscript.Util.defaultHost:
    args = ' --IceBox.UseSharedCommunicator.IceStorm=1'
else:
    args = ''

icestorm = demoscript.Util.spawn('%s --Ice.Config=config.icebox --Ice.PrintAdapterReady %s' % (demoscript.Util.getIceBox(), args))
icestorm.expect('.* ready')

print "testing single client...",
sys.stdout.flush()
server = demoscript.Util.spawn('./server --Ice.PrintAdapterReady')
server.expect('.* ready')
client1 = demoscript.Util.spawn('./client')
client1.expect('init: 0')
client1.sendline('i')
client1.expect('int: 1 total: 1')
print "ok"

print "testing second client...",
sys.stdout.flush()
client2 = demoscript.Util.spawn('./client')
client2.expect('init: 1')
client2.sendline('i')
client1.expect('int: 1 total: 2')
client2.expect('int: 1 total: 2')
print "ok"

print "testing third client...",
client3 = demoscript.Util.spawn('./client')
client3.expect('init: 2')
client3.sendline('d')
client1.expect('int: -1 total: 1')
client2.expect('int: -1 total: 1')
client3.expect('int: -1 total: 1')
print "ok"

print "testing removing client...",
client3.sendline('x')
client3.expect(pexpect.EOF)
assert client3.wait() == 0

client2.sendline('d')
client1.expect('int: -1 total: 0')
client2.expect('int: -1 total: 0')
client1.sendline('x')
client1.expect(pexpect.EOF)
assert client1.wait() == 0
client2.sendline('x')
client2.expect(pexpect.EOF)
assert client2.wait() == 0
print "ok"

admin = demoscript.Util.spawn('iceboxadmin --Ice.Config=config.icebox shutdown')
admin.expect(pexpect.EOF)
assert admin.wait() == 0
icestorm.expect(pexpect.EOF)
assert icestorm.wait() == 0
