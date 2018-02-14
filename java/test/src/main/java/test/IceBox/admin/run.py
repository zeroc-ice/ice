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

icebox = TestUtil.getIceBox()
config = os.path.join(os.getcwd(), "config.icebox")

TestUtil.clientServerTest(additionalServerOptions='--Ice.Config="%s"' % config, server=icebox)

sys.stdout.write("testing iceboxadmin... ")
sys.stdout.flush()

admin = TestUtil.getIceBoxAdmin()
adminconfig = os.path.join(os.getcwd(), "config.admin")

ib = TestUtil.startServer(icebox, args = '--Ice.Config=\"%s\"' % config)
iba = TestUtil.startClient(admin, args = '--Ice.Config=\"%s\" stop TestService' % adminconfig)
iba.waitTestSuccess()
iba = TestUtil.startClient(admin, args = '--Ice.Config=\"%s\" start TestService' % adminconfig)
iba.waitTestSuccess()
iba = TestUtil.startClient(admin, args = '--Ice.Config=\"%s\" shutdown' % adminconfig)
iba.waitTestSuccess()
ib.waitTestSuccess()

print("ok")
