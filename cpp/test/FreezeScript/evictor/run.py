#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
#
# **********************************************************************

import os, sys, re, shutil

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

directory = os.path.join(toplevel, "test", "FreezeScript", "evictor")
transformdb = os.path.join(toplevel, "bin", "transformdb")

dbdir = os.path.join(directory, "db")
TestUtil.cleanDbDir(dbdir)

check_dbdir = os.path.join(directory, "db_check")
if os.path.exists(check_dbdir):
    shutil.rmtree(check_dbdir)
os.mkdir(check_dbdir)

tmp_dbdir = os.path.join(directory, "db_tmp")
if os.path.exists(tmp_dbdir):
    shutil.rmtree(tmp_dbdir)
os.mkdir(tmp_dbdir)

print "creating test database... ",
sys.stdout.flush()

makedb = os.path.join(directory, "makedb") + " " + directory
os.system(makedb)

print "ok"

testold = os.path.join(directory, "TestOld.ice")
testnew = os.path.join(directory, "TestNew.ice")
transformxml = os.path.join(directory, "transform.xml")
checkxml = os.path.join(directory, "check.xml")

print "executing evictor transformations... ",
sys.stdout.flush()

command = transformdb + " -p --old " + testold + " --new " + testnew + " -f " + transformxml + " " + dbdir + " evictor.db " + check_dbdir
stdin, stdout, stderr = os.popen3(command)
stderr.readlines()

print "ok"

print "validating database... ",
sys.stdout.flush()

command = transformdb + " --old " + testnew + " --new " + testnew + " -f " + checkxml + " " + check_dbdir + " evictor.db " + tmp_dbdir
os.system(command)

print "ok"

sys.exit(0)
