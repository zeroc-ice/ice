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

import os, sys, subprocess, glob, atexit

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

sys.stdout.write("testing logger file rotation... ")

def cleanup():
    for f in glob.glob("client5-*.log"):
        os.remove(f)
    if os.path.exists("log/client5-4.log"):
        os.remove("log/client5-4.log")

cleanup()

atexit.register(cleanup)


if not os.path.exists("log"):
    os.makedirs("log")

open("log/client5-4.log", 'a').close()

if TestUtil.isWin32():
    os.system("echo Y|cacls log /P %USERNAME%:R 1> nul")
else:
    os.system("chmod -w log")


p = subprocess.Popen(os.path.join(os.getcwd(), "client5"), stdout=subprocess.PIPE, stderr=subprocess.STDOUT, env=env)
out, err = p.communicate()
ret = p.poll()
if ret != 0:
    print("failed! status %s " % ret)
    sys.exit(1)

if TestUtil.isWin32():
    os.system("echo Y|cacls log /P %USERNAME%:F 1> nul")
else:
    os.system("chmod +w log")

if (not os.path.isfile("client5-0.log") or
    not os.stat("client5-0.log").st_size == 512 or
    len(glob.glob("client5-0-*.log")) != 19):
    print("failed!")
    sys.exit(1)

for f in glob.glob("client5-0-*.log"):
    if not os.stat(f).st_size == 512:
        print("failed! file {0} size: {1} unexpected".format(f, os.stat(f).st_size))
        sys.exit(1)

if (not os.path.isfile("client5-1.log") or
    not os.stat("client5-1.log").st_size == 1024 or
    len(glob.glob("client5-1-*.log")) != 0):
    print("failed!")
    sys.exit(1)

if (not os.path.isfile("client5-2.log") or
    not os.stat("client5-2.log").st_size == 128 or
    len(glob.glob("client5-2-*.log")) != 7):
    print("failed!")
    sys.exit(1)

for f in glob.glob("client5-2-*.log"):
    if not os.stat(f).st_size == 128:
        print("failed! file {0} size: {1} unexpected".format(f, os.stat(f).st_size))
        sys.exit(1)

if (not os.path.isfile("client5-3.log") or
    not os.stat("client5-2.log").st_size == 128 or
    len(glob.glob("client5-2-*.log")) != 7):
    print("failed!")
    sys.exit(1)

for f in glob.glob("client5-3-*.log"):
    if not os.stat(f).st_size == 128:
        print("failed! file {0} size: {1} unexpected".format(f, os.stat(f).st_size))
        sys.exit(1)

if (not os.path.isfile("log/client5-4.log") or
    os.stat("log/client5-4.log").st_size < 1024 or
    len(glob.glob("log/client5-4-*.log")) > 0):
    print("failed!")
    sys.exit(1)

with open("log/client5-4.log", 'r') as f:
    if f.read().count("error: FileLogger: cannot rename `log/client5-4.log'") != 1:
        print("failed!")
        sys.exit(1)

print("ok")
