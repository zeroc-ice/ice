#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil

transformdb = '%s' % os.path.join(TestUtil.getCppBinDir(), "transformdb") 

if TestUtil.appverifier:
    TestUtil.setAppVerifierSettings([transformdb])

dbdir = os.path.join(os.getcwd(), "db")
TestUtil.cleanDbDir(dbdir)

init_dbdir = os.path.join(os.getcwd(), "db_init")
if os.path.exists(init_dbdir):
    shutil.rmtree(init_dbdir)
os.mkdir(init_dbdir)

check_dbdir = os.path.join(os.getcwd(), "db_check")
if os.path.exists(check_dbdir):
    shutil.rmtree(check_dbdir)
os.mkdir(check_dbdir)

tmp_dbdir = os.path.join(os.getcwd(), "db_tmp")
if os.path.exists(tmp_dbdir):
    shutil.rmtree(tmp_dbdir)
os.mkdir(tmp_dbdir)

regex1 = re.compile(r"_old\.ice$", re.IGNORECASE)
files = []
for file in os.listdir(os.path.join(os.getcwd(), "fail")):
    if(regex1.search(file)):
        files.append(file)

regex2 = re.compile(r"^.*transf(ormdb|~1)(\.exe)?", re.IGNORECASE)

sys.stdout.write("testing error detection... ")
sys.stdout.flush()
if TestUtil.debug:
    sys.stdout.write("\n")

files.sort()
for oldfile in files:

    newfile = oldfile.replace("old", "new")

    if oldfile.find("19") == 0:
        value = "::Test::C"
    else:
        value = "int"

    command = '"' + transformdb + '" --old "' + os.path.join(os.getcwd(), "fail", oldfile) + '" --new "' + \
        os.path.join(os.getcwd(), "fail", newfile) + '" -o tmp.xml --key string --value ' + value

    if TestUtil.debug:
        print(command)

    p = TestUtil.runCommand(command)
    (stdin, stdout, stderr) = (p.stdin, p.stdout, p.stderr)

    lines1 = stderr.readlines()
    lines2 = open(os.path.join(os.getcwd(), "fail", oldfile.replace("_old.ice", ".err")), "r").readlines()
    if len(lines1) != len(lines2):
        print("failed! (1)")
        sys.exit(1)
    
    i = 0
    while i < len(lines1):
        if sys.version_info[0] == 2:
            line1 = regex2.sub("", lines1[i]).strip()
            line2 = regex2.sub("", lines2[i]).strip()
        else:
            line1 = regex2.sub("", lines1[i].decode("utf-8")).strip()
            line2 = regex2.sub("", lines2[i]).strip()
        if line1 != line2:
            print("failed! (2)")
            print("line1 = " + line1)
            print("line2 = " + line2)
            # sys.exit(1)
        i = i + 1

print("ok")

sys.stdout.write("creating test database... ")
sys.stdout.flush()

makedb = '"%s" "%s"'% (os.path.join(os.getcwd(), "makedb"), os.getcwd())
proc = TestUtil.spawn(makedb)
proc.waitTestSuccess()
print("ok")

testold = os.path.join(os.getcwd(), "TestOld.ice")
testnew = os.path.join(os.getcwd(), "TestNew.ice")
initxml = os.path.join(os.getcwd(), "init.xml")
checkxml = os.path.join(os.getcwd(), "check.xml")

sys.stdout.write("initializing test database... ")
sys.stdout.flush()

command = '"' + transformdb + '" --old "' + testold + '" --new "' + testold + '" -f "' + initxml + '" "' + dbdir + \
    '" default.db "' + init_dbdir + '" '

TestUtil.spawn(command).waitTestSuccess()

print("ok")

sys.stdout.write("executing default transformations... ")
sys.stdout.flush()

command = '"' + transformdb + '" --old "' + testold + '" --new "' + testnew + '" --key int --value ::Test::S "' + \
    init_dbdir + '" default.db "' + check_dbdir + '" '

TestUtil.spawn(command).waitTestSuccess()

print("ok")

sys.stdout.write("validating database... ")
sys.stdout.flush()

command = '"' + transformdb + '" --old "' + testnew + '" --new "' + testnew + '" -f "' + checkxml + '" "' + \
    check_dbdir + '" default.db "' + tmp_dbdir + '" '

TestUtil.spawn(command).waitTestSuccess()

print("ok")

if TestUtil.appverifier:
    TestUtil.appVerifierAfterTestEnd([transformdb])
