#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
import TestUtil, IceGridAdmin

TestUtil.addLdPath(os.getcwd())

if TestUtil.sqlType != None and TestUtil.sqlType != "QSQLITE":
    print("*** This test only supports Freeze or SQLite databases")
    sys.exit(0)

variables = "properties-override='%s'" % IceGridAdmin.iceGridNodePropertiesOverride()

if TestUtil.sqlType != None:
    variables += " db-plugin=IceGridSqlDB:createSqlDB"
else:
    variables += " db-plugin=IceGridFreezeDB:createFreezeDB"

IceGridAdmin.iceGridTest("application.xml", '--IceDir="%s" --TestDir="%s"' % (TestUtil.toplevel, os.getcwd()),
                         variables)
