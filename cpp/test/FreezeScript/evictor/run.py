#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, re, shutil

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise "can't find toplevel os.getcwd()!"
sys.path.append(os.path.join(path[0]))
from scripts import *

transformdb = os.path.join(TestUtil.getCppBinDir(), "transformdb")

dbdir = os.path.join(os.getcwd(), "db")
TestUtil.cleanDbDir(dbdir)

check_dbdir = os.path.join(os.getcwd(), "db_check")
if os.path.exists(check_dbdir):
    shutil.rmtree(check_dbdir)
os.mkdir(check_dbdir)

tmp_dbdir = os.path.join(os.getcwd(), "db_tmp")
if os.path.exists(tmp_dbdir):
    shutil.rmtree(tmp_dbdir)
os.mkdir(tmp_dbdir)

print "creating test database...",
sys.stdout.flush()

makedb = os.path.join(os.getcwd(), "makedb") + " " + os.getcwd()
proc = TestUtil.spawn(makedb)
proc.waitTestSuccess()
print "ok"

testold = os.path.join(os.getcwd(), "TestOld.ice")
testnew = os.path.join(os.getcwd(), "TestNew.ice")
transformxml = os.path.join(os.getcwd(), "transform.xml")
checkxml = os.path.join(os.getcwd(), "check.xml")

print "executing evictor transformations...",
sys.stdout.flush()

command = transformdb + " -e -p --old " + testold + " --new " + testnew + " -f " + transformxml + " " + dbdir + \
    " evictor.db " + check_dbdir
proc = TestUtil.spawn(command)
proc.waitTestSuccess()
print "ok"

print "validating database...",
sys.stdout.flush()

command = transformdb + " -e --old " + testnew + " --new " + testnew + " -f " + checkxml + " " + check_dbdir + \
    " evictor.db " + tmp_dbdir
proc = TestUtil.spawn(command)
proc.waitTestSuccess()
print "ok"

