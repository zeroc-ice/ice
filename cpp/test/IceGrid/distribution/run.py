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
import TestUtil, IceGridAdmin

def icepatch2Calc(datadir, dirname):
    icePatch2Calc = os.path.join(TestUtil.getCppBinDir(), "icepatch2calc")
    commandProc = TestUtil.spawn('"%s" "%s"' % (icePatch2Calc, os.path.join(datadir, dirname)))
    commandProc.waitTestSuccess()

datadir = os.path.join(os.getcwd(), "data")
 
files = [ 
  [ "original/rootfile", "rootfile" ],
  [ "original/dir1/file1", "dummy-file1"],
  [ "original/dir1/file2", "dummy-file2"],
  [ "original/dir2/file3", "dummy-file3"],
  [ "updated/rootfile", "rootfile-updated!" ],
  [ "updated/dir1/file2", "dummy-file2-updated!"],
  [ "updated/dir2/file3", "dummy-file3"],
  [ "updated/dir2/file4", "dummy-file4"],
]


sys.stdout.write("creating IcePatch2 data directory... ")
sys.stdout.flush()
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
print("ok")

IceGridAdmin.iceGridTest("application.xml")

IceGridAdmin.cleanDbDir(datadir)
os.rmdir(datadir)

