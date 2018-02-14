#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil

testdir = os.getcwd()

client = os.path.join(testdir, "Client.php")
sys.stdout.write("testing php INI settings... ")
sys.stdout.flush()
clientProc = TestUtil.startClient(client, startReader = False, clientConfig = True, iceOptions = "--Ice.Trace.Network=1 --Ice.Warn.Connections=1")
clientProc.startReader()
clientProc.waitTestSuccess()
print("ok")

client = os.path.join(testdir, "ClientWithProfile.php")
sys.stdout.write("testing php INI settings with profiles... ")
sys.stdout.flush()
clientProc = TestUtil.startClient(client, startReader = False, clientConfig = True, iceOptions = "--Ice.Trace.Network=1 --Ice.Warn.Connections=1", iceProfile="Test")
clientProc.startReader()
clientProc.waitTestSuccess()
print("ok")
