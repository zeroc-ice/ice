#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys

path = [ ".", "..", "../..", "../../..", "../../../..", "../../../../..", "../../../../../..",
         "../../../../../../..", "../../../../../../../..", "../../../../../../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil

dbdir = os.path.join(os.getcwd(), "db")
TestUtil.cleanDbDir(dbdir)

sys.stdout.write("testing Freeze file lock... ")
sys.stdout.flush()

client = os.path.join("test.Freeze.fileLock.Client")
clientFail = os.path.join("test.Freeze.fileLock.ClientFail")

clientExe = TestUtil.startClient(client, "", None, None, False)
clientExe.expect('File lock acquired.\.*')

clientFailExe = TestUtil.startClient(clientFail, "", None, None, False)
clientFailExe.expect('File lock not acquired.')
clientFailExe.wait()

# send some output to client to terminate it.
clientExe.sendline('go')
clientExe.expect('File lock released.')
clientExe.wait()

# The lock is gone try to acquire it again.
clientExe = TestUtil.startClient(client, "", None, None, False)
clientExe.expect('File lock acquired.\.*')
clientExe.sendline('go')
clientExe.expect('File lock released.')
clientExe.wait()
print("ok")
