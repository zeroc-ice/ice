#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2001
# MutableRealms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

import os, sys

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

name = os.path.join("IceUtil", "unicode")
testdir = os.path.join(toplevel, "test", name)

client = os.path.join(testdir, "client")
clientOptions = ' ' + testdir;

print "creating random utf-8 data...",
import random, string
values = range(32, 500) + range(2000, 2500) + range(40000, 40100)
random.shuffle(values)
characters = string.join(map(unichr, values), u"")
file = open("numeric.txt", "w")
for w in values:
    file.write(str(w) + "\n")
file.close();
file = open("utf8.txt", "wb")
file.write(characters.encode("utf-8"))
file.close();
print "ok"

print "starting client...",
clientPipe = os.popen(client + clientOptions)
output = clientPipe.read().strip()
if not output:
    print "failed!"
    sys.exit(1)
print "ok"
print output

os.remove("numeric.txt")
os.remove("utf8.txt")

sys.exit(0)
