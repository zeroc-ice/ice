#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, shutil, subprocess

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil

testdir = os.path.dirname(os.path.abspath(__file__))

if os.path.exists(os.path.join(testdir, "Test1_ice.py")):
    os.remove(os.path.join(testdir, "Test1_ice.py"))
if os.path.exists(os.path.join(testdir, "Test2_ice.py")):
    os.remove(os.path.join(testdir, "Test2_ice.py"))
if os.path.exists(os.path.join(testdir, "Test")):
    shutil.rmtree(os.path.join(testdir, "Test"))

if os.environ.get("USE_BIN_DIST", "no") == "yes":
    if TestUtil.isDarwin():
        slice2py = sys.executable + " /usr/local/bin/slice2py"
    elif TestUtil.isWin32():
        pythonHome = os.path.dirname(sys.executable)
        slice2py = os.path.join(pythonHome, "Scripts", "slice2py.exe")
    elif os.path.isfile(os.path.join(TestUtil.getCppBinDir(), "slice2py")):
        slice2py = os.path.join(TestUtil.getCppBinDir(), "slice2py")
    else:
        import slice2py
        slice2py = sys.executable + " " + os.path.normpath(os.path.join(slice2py.__file__, "..", "..", "..", "..", "bin", "slice2py"))
else:
    if TestUtil.isYocto():
        slice2py = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "..", "..", "..", "cpp", "bin", "slice2py")
    else:
        slice2py = sys.executable + " " + os.path.join(path[0], "python", "config", "s2py.py")

s2p = TestUtil.spawn(slice2py + " Test1.ice")
s2p.waitTestSuccess()
s2p = TestUtil.spawn(slice2py + " Test2.ice")
s2p.waitTestSuccess()

sys.stdout.write("starting client... ")
sys.stdout.flush()
clientProc = TestUtil.startClient("Client.py", "--Ice.Default.Host=127.0.0.1", startReader = False)
print("ok")
clientProc.startReader()
clientProc.waitTestSuccess()
