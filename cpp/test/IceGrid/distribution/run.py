#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

def icepatch2Calc(datadir, dirname):

    icePatch2Calc = os.path.join(toplevel, "bin", "icepatch2calc")
    commandPipe = os.popen(icePatch2Calc + " " + os.path.join(datadir, dirname) + " 2>&1")

    TestUtil.printOutputFromPipe(commandPipe)
    
    commandStatus = TestUtil.closePipe(commandPipe)
    if commandStatus:
        sys.exit(1)

name = os.path.join("IceGrid", "distribution")
testdir = os.path.join(toplevel, "test", name)
datadir = os.path.join(testdir, "data")
 
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


print "creating IcePatch2 data directory...",
if not os.path.exists(datadir):
    os.mkdir(datadir)
else:
    IceGridAdmin.cleanDbDir(datadir)

for [file, content] in files:
    file = os.path.join(datadir, file)
    if not os.path.exists(os.path.dirname(file)):
        os.makedirs(os.path.dirname(file))
    f = open(file, 'w')    
    f.write(content)
    f.close()

icepatch2Calc(datadir, "original")
icepatch2Calc(datadir, "updated")
print "ok"

IceGridAdmin.iceGridTest(name, "application.xml", "")

IceGridAdmin.cleanDbDir(datadir)
os.rmdir(datadir)

sys.exit(0)
