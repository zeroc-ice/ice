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

import os, sys, re

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

directory = os.path.join(toplevel, "test", "Transform", "dbmap")
transformdb = os.path.join(toplevel, "bin", "transformdb")

dbdir = os.path.join(directory, "db")
TestUtil.cleanDbDir(dbdir)

regex1 = re.compile(r"_old\.ice$", re.IGNORECASE)
files = []
for file in os.listdir(os.path.join(directory, "fail")):
    if(regex1.search(file)):
        files.append(file)

print "testing error detection... ",
sys.stdout.flush()

files.sort()
for oldfile in files:

    newfile = oldfile.replace("old", "new")

    if oldfile.find("19") == 0:
        value = "::C"
    else:
        value = "int"

    command = transformdb + " --old " + os.path.join(directory, "fail", oldfile) + " --new " + os.path.join(directory, "fail", newfile) + " -o tmp.xml --key string --value " + value

    stdin, stdout, stderr = os.popen3(command)
    lines1 = stderr.readlines()
    lines2 = open(os.path.join(directory, "fail", oldfile.replace("_old.ice", ".err")), "r").readlines()
    if len(lines1) != len(lines2):
        print "failed!"
        sys.exit(1)
    
    i = 0
    while i < len(lines1):
        line1 = lines1[i].replace(transformdb, "")
        line2 = lines2[i].replace("transformdb", "")
        if line1 != line2:
            print "failed!"
            sys.exit(1)
        i = i + 1

print "ok"

print "testing default transformations... ",
sys.stdout.flush()

makedb = os.path.join(directory, "makedb") + " " + directory
os.system(makedb)

testold = os.path.join(directory, "TestOld.ice")
testnew = os.path.join(directory, "TestNew.ice")

command = transformdb + " --old " + testold + " --new " + testold + " -f init.xml " + dbdir + " default.db init.db"
os.system(command)

command = transformdb + " --old " + testold + " --new " + testnew + " --key int --value ::S " + dbdir + " init.db check.db"
stdin, stdout, stderr = os.popen3(command)

stderr.readlines()
command = transformdb + " --old " + testnew + " --new " + testnew + " -f check.xml " + dbdir + " check.db tmp.db"
os.system(command)

print "ok"

sys.exit(0)
