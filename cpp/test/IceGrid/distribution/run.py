#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil
import IceGridAdmin

name = os.path.join("IceGrid", "distribution")
testdir = os.path.join(toplevel, "test", name)
datadir = os.path.join(toplevel, "test", "data")
 
files = [ \
  [ "original/rootfile", "rootfile" ],
  [ "original/dir1/file1", "dummy-file1"],
  [ "original/dir1/file2", "dummy-file2"],
  [ "original/dir2/file3", "dummy-file3"],
  [ "updated/rootfile", "rootfile-updated!" ],
  [ "updated/dir1/file2", "dummy-file2-updated!"],
  [ "updated/dir2/file3", "dummy-file3"],
  [ "updated/dir2/file4", "dummy-file4"],
]

if not os.path.exists(datadir):
    os.mkdir(datadir)
else:
    IceGridAdmin.cleanDbDir(datadir)
    os.rmdir(datadir)

for i in files:
     

IceGridAdmin.iceGridTest(name, "application.xml", "")
sys.exit(0)
