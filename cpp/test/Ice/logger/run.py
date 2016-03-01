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

import os, sys, subprocess

path = [ ".", "..", "../..", "../../..", "../../../..", "../../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil

def test(cmd, match, enc):
    p = subprocess.Popen([cmd], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, env=env)
    out, err = p.communicate()
    ret = p.poll()
    if ret != 0:
        print("%s failed! status %s " % (cmd, ret))
    if out.decode(enc).find(match.decode(enc)) == -1:
        raise RuntimeError("test failed")

TestUtil.simpleTest(os.path.join(os.getcwd(), "client1"))
env = TestUtil.getTestEnv("cpp", os.getcwd())

sys.stdout.write("testing logger ISO-8859-15 output... ")
test(os.path.join(os.getcwd(), "client2"), b'aplicaci\xf3n', "ISO-8859-15")
print("ok")

sys.stdout.write("testing logger UTF8 output without string converter... ")
test(os.path.join(os.getcwd(), "client3"), b'aplicaci\xc3\xb3n', "UTF8")
print("ok")

sys.stdout.write("testing logger UTF8 output with ISO-8859-15 narrow string converter... ")
#
# In Windows expected output is UTF8, because the console output code page is set to UTF-8
# in Linux and OS X, the expected output is ISO-8859-15 because that is the narrow string
# encoding used by the application.
#
if TestUtil.isWin32():
    test(os.path.join(os.getcwd(), "client4"), b'aplicaci\xc3\xb3n', "UTF8")
else:
    test(os.path.join(os.getcwd(), "client4"), b'aplicaci\xf3n', "ISO-8859-15")
print("ok")
