#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
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

directory = os.path.join(toplevel, "test", "FreezeScript", "dbmap")
transformdb = os.path.join(toplevel, "bin", "transformdb")

dbdir = os.path.join(directory, "db")
TestUtil.cleanDbDir(dbdir)

init_dbdir = os.path.join(directory, "db_init")
if os.path.exists(init_dbdir):
    shutil.rmtree(init_dbdir)
os.mkdir(init_dbdir)

check_dbdir = os.path.join(directory, "db_check")
if os.path.exists(check_dbdir):
    shutil.rmtree(check_dbdir)
os.mkdir(check_dbdir)

tmp_dbdir = os.path.join(directory, "db_tmp")
if os.path.exists(tmp_dbdir):
    shutil.rmtree(tmp_dbdir)
os.mkdir(tmp_dbdir)

regex1 = re.compile(r"_old\.ice$", re.IGNORECASE)
files = []
for file in os.listdir(os.path.join(directory, "fail")):
    if(regex1.search(file)):
        files.append(file)

regex2 = re.compile(r"^.*transf(ormdb|~1)(\.exe)?", re.IGNORECASE)

print "testing error detection...",
sys.stdout.flush()
if TestUtil.debug:
    print

files.sort()
for oldfile in files:

    newfile = oldfile.replace("old", "new")

    if oldfile.find("19") == 0:
        value = "::Test::C"
    else:
        value = "int"

    command = transformdb + " --old " + os.path.join(directory, "fail", oldfile) + " --new " + \
        os.path.join(directory, "fail", newfile) + " -o tmp.xml --key string --value " + value

    if TestUtil.debug:
        print command

    stdin, stdout, stderr = os.popen3(command)
    lines1 = stderr.readlines()
    lines2 = open(os.path.join(directory, "fail", oldfile.replace("_old.ice", ".err")), "r").readlines()
    if len(lines1) != len(lines2):
        print "failed! (1)"
        sys.exit(1)
    
    i = 0
    while i < len(lines1):
        line1 = regex2.sub("", lines1[i]).strip()
        line2 = regex2.sub("", lines2[i]).strip()
        if line1 != line2:
            print "failed! (2)"
            print "line1 = " + line1
            print "line2 = " + line2
            # sys.exit(1)
        i = i + 1

print "ok"

print "creating test database...",
sys.stdout.flush()

makedb = os.path.join(directory, "makedb") + " " + directory
if TestUtil.debug:
    print "(" + makedb + ")",
if os.system(makedb) != 0:
    sys.exit(1)

print "ok"

testold = os.path.join(directory, "TestOld.ice")
testnew = os.path.join(directory, "TestNew.ice")
initxml = os.path.join(directory, "init.xml")
checkxml = os.path.join(directory, "check.xml")

print "initializing test database...",
sys.stdout.flush()

command = transformdb + " --old " + testold + " --new " + testold + " -f " + initxml + " " + dbdir + \
    " default.db " + init_dbdir
if TestUtil.debug:
    print "(" + command + ")",
if os.system(command) != 0:
    sys.exit(1)

print "ok"

print "executing default transformations...",
sys.stdout.flush()

command = transformdb + " --old " + testold + " --new " + testnew + " --key int --value ::Test::S " + init_dbdir + \
    " default.db " + check_dbdir
if TestUtil.debug:
    print "(" + command + ")",
stdin, stdout, stderr = os.popen3(command)
stderr.readlines()

print "ok"

print "validating database...",
sys.stdout.flush()

command = transformdb + " --old " + testnew + " --new " + testnew + " -f " + checkxml + " " + check_dbdir + \
    " default.db " + tmp_dbdir
if TestUtil.debug:
    print "(" + command + ")",
if os.system(command) != 0:
    sys.exit(1)

print "ok"

sys.exit(0)
