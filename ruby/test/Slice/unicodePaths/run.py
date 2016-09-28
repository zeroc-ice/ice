#!/usr/bin/env python
# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, locale, shutil

path = [ ".", "..", "../..", "../../..", "../../../..", "../../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil

def test(b):
    if not b:
        print("failed!")
        sys.exit(1)

if TestUtil.isAIX() or TestUtil.isLinux():
    encoding = locale.getdefaultlocale()[1]
    if encoding != "UTF-8":
        print("Please set LC_ALL to xx_xx.UTF-8, for example FR_FR.UTF-8")
        print("Skipping test")
        sys.exit(0)

if sys.version_info[0] == 2 and TestUtil.isWin32():
    print("To run this test on Windows you need to be using Python 3.x")
    print("Python 2.x subprocess module doesn't support unicode on Windows")
    print("Skipping tes")
    sys.exit(0)

if os.environ.get("USE_BIN_DIST", "no") == "yes":
    if TestUtil.isYocto():
        slice2rb = os.path.join(TestUtil.getCppBinDir(), "slice2rb")
    if os.path.isfile("/usr/bin/slice2rb.ruby2.1"):
        slice2rb = "slice2rb.ruby2.1"
    else:
        slice2rb = "slice2rb"
else:
    if TestUtil.isYocto():
        slice2rb = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "..", "..", "..", "cpp", "bin", "slice2rb")
    else:
        slice2rb =  "ruby " + os.path.join(path[0], "ruby", "config", "s2rb.rb")

sys.stdout.write("testing Slice compiler and unicode file paths... ")
sys.stdout.flush()

#
# Write Slice file
#
if sys.version_info[0] == 2:
    srcPath = "./\xe4\xb8\xad\xe5\x9b\xbd".decode("utf-8")
else:
    srcPath = "./\u4e2d\u56fd"
if os.path.exists(srcPath):
    shutil.rmtree(srcPath)
os.mkdir(srcPath)

TestUtil.createFile("%s/Test.ice" % srcPath, ["module Test { ",
                                              "class Point{int x; int y; };",
                                              "interface Canvas{ void draw(Point p); };",
                                              "};"], "utf-8")
p = TestUtil.runCommand('%s %s/Test.ice --output-dir %s' % (slice2rb, srcPath, srcPath))
test(p.wait() == 0)
test(os.path.isfile("%s/Test.rb" % srcPath))

if os.path.exists(srcPath):
    shutil.rmtree(srcPath)
print("ok")
