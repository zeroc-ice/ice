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

if TestUtil.isAIX():
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

sys.stdout.write("testing Slice compiler and unicode file paths... ")
sys.stdout.flush()
tests = [
    ("cpp", ["Test.cpp", "Test.h", "TestI.cpp", "TestI.h"], "--impl"),
    ("cs", ["Test.cs", "TestI.cs"], "--impl"),
    ("html", ["index.html"], ""),
    ("java", ["Test/Point.java", "Test/CanvasI.java"], "--impl"),
    ("js", ["Test.js"], ""),
    ("objc", ["Test.m"], ""),
    ("php", ["Test.php"], "")]

#
# Write config
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

for language, generated, args in tests:
    compiler = '%s' % TestUtil.getSliceTranslator(language)
    if not os.path.isfile(compiler):
        continue
    p = TestUtil.runCommand('"%s" %s/Test.ice --output-dir %s %s' % (compiler, srcPath, srcPath, args))
    test(p.wait() == 0)
    for f in generated:
        test(os.path.isfile("%s/%s" % (srcPath, f)))


if os.path.exists(srcPath):
    shutil.rmtree(srcPath)
print("ok")
