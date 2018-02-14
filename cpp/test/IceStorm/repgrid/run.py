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
import TestUtil, IceStormUtil, IceGridAdmin

targets = []
if TestUtil.appverifier:
    targets = [TestUtil.getIceBox()]
    TestUtil.setAppVerifierSettings(targets, cwd = os.getcwd())

#
# Remove IceStorm databases possibly left from SQL run.
#
for filename in [os.path.join("db", f) for f in os.listdir("db") if f.endswith(".db")]:
    os.remove(filename)

variables = "icebox.exe='%s'" % TestUtil.getIceBox()

#
# Don't use a slave registry for this test.
#
IceGridAdmin.nreplicas=0

#
# Test client/server without on demand activation.
#
IceGridAdmin.iceGridTest("application.xml", "", variables)

if TestUtil.appverifier:
    TestUtil.appVerifierAfterTestEnd(targets, cwd = os.getcwd())
