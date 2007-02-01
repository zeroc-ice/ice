#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, re

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

#
# For LD_LIBRARY_PATH and similar env variables
#
sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

directory = os.path.join(toplevel, "test", "Slice", "errorDetection")
slice2cpp = os.path.join(toplevel, "bin", "slice2cpp")

regex1 = re.compile(r"\.ice$", re.IGNORECASE)
files = []
for file in os.listdir(directory):
    if(regex1.search(file)):
        files.append(file)

files.sort()

for file in files:

    print file + "...",

    if file == "CaseSensitive.ice":
        command = slice2cpp + " --case-sensitive -I. " + os.path.join(directory, file);
    else:
        command = slice2cpp + " -I. " + os.path.join(directory, file);
    stdin, stdout, stderr = os.popen3(command)
    lines1 = stdout.readlines()
    lines2 = open(os.path.join(directory, regex1.sub(".err", file)), "r").readlines()
    if len(lines1) != len(lines2):
        print "failed!"
        sys.exit(1)
    
    regex2 = re.compile(r"^.*(?=" + file + ")")
    i = 0
    while i < len(lines1):
        line1 = regex2.sub("", lines1[i]).strip()
        line2 = regex2.sub("", lines2[i]).strip()
        if line1 != line2:
            print "failed!"
            sys.exit(1)
        i = i + 1
    else:
        print "ok"

sys.exit(0)
