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

#
# Clean the contents of the database directory.
#
dbdir = os.path.join(os.getcwd(), "db")
TestUtil.cleanDbDir(dbdir)

sys.stdout.write("starting populate... ")
sys.stdout.flush()
populateProc = TestUtil.startClient("test.Freeze.complex.Client", ' --dbdir "%s" populate' % os.getcwd(),
                                    startReader=False)
print("ok")
populateProc.startReader()

populateProc.waitTestSuccess()

sys.stdout.write("starting verification client... ")
sys.stdout.flush()
clientProc = TestUtil.startClient("test.Freeze.complex.Client", ' --dbdir "%s" validate' % os.getcwd(), startReader=False)

print("ok")
clientProc.startReader()
clientProc.waitTestSuccess()
