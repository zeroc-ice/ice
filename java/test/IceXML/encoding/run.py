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

testdir = os.path.join(toplevel,"test", "IceXML", "encoding")
classpath = os.path.join(toplevel, "lib") + TestUtil.sep + os.path.join(testdir, "classes") + \
    TestUtil.sep + os.environ['CLASSPATH']
client = "java -classpath \"" + classpath + "\" Client"

print "starting client...",
clientPipe = os.popen(client)
output = clientPipe.read().strip()
if not output:
    print "failed!"
    clientPipe.close()
    sys.exit(1)
print "ok"
print output
clientPipe.close()

sys.exit(0)
