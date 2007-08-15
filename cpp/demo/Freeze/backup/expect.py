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
import signal, time

print "cleaning databases...",
sys.stdout.flush()
os.system('rm -fr db.save')
demoscript.Util.cleanDbDir("db/data")
demoscript.Util.cleanDbDir("db/logs")
os.system('rm -fr db/__*')
for d in os.listdir('.'):
    if d.startswith('hotbackup'):
        os.system('rm -rf %s' % (d))
print "ok"

client = demoscript.Util.spawn('./client')

print "populating map...",
sys.stdout.flush()
client.expect('Updating map', timeout=60)
time.sleep(3) # Let the client do some work for a bit.
print "ok"


print "performing full backup...",
sys.stdout.flush()
backup = demoscript.Util.spawn('./backup full')
backup.expect('hot backup started', timeout=30)
backup.expect(pexpect.EOF, timeout=30)
assert backup.wait() == 0
print "ok"

print "sleeping 5s...",
sys.stdout.flush()
time.sleep(5)
print "ok"

print "performing incremental backup...",
sys.stdout.flush()
backup = demoscript.Util.spawn('./backup incremental')
backup.expect('hot backup started', timeout=30)
backup.expect(pexpect.EOF, timeout=30)
print "ok"

print "sleeping 30s...",
sys.stdout.flush()
time.sleep(30)
print "ok"

assert os.path.isdir('hotbackup')

print "killing client with SIGTERM...",
sys.stdout.flush()
client.kill(signal.SIGTERM)
client.expect(pexpect.EOF, timeout=30)
assert client.wait() != 0
print "ok"

print "Client output: ",
print "%s " % (client.before)

print "restarting client...",
sys.stdout.flush()
os.system('rm -fr db/data/* db/logs/* db/__*')
os.system('cp -Rp hotbackup/* db')
sys.stdout.flush()

rclient = demoscript.Util.spawn('./client')
rclient.expect('(.*)Updating map', timeout=60)
assert rclient.match.group(1).find('Creating new map') == -1
print "ok"

print "sleeping 5s...",
sys.stdout.flush()
time.sleep(5)
print "ok"

print "killing client with SIGTERM...",
rclient.kill(signal.SIGTERM)
rclient.expect(pexpect.EOF, timeout=30)
assert rclient.wait() != 0
print "ok"

print "Restarted client output:",
print "%s " % (rclient.before)
