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

import os, sys, re

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    if os.path.exists(os.path.normpath(toplevel + "/config/TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

directory = os.path.normpath(toplevel + "/test/Slice/errorDetection")
slice2cpp = os.path.normpath(toplevel + "/bin/slice2cpp")

regex1 = re.compile(r".ice$", re.IGNORECASE)
files = []
for file in os.listdir(directory):
    if(regex1.search(file)):
        files.append(file)

for file in files:

    print file + "...",

    stdin, stdout, stderr = os.popen3(slice2cpp + " " + os.path.normpath(directory + "/" + file))
    lines1 = stderr.readlines()
    lines2 = open(os.path.normpath(directory + "/" + regex1.sub(".err", file)), "r").readlines()

    if len(lines1) != len(lines2):
        print "failed!"
        continue
    
    regex2 = re.compile(r"^.*(?=" + file + ")")
    i = 0
    while i < len(lines1):
        line1 = regex2.sub("", lines1[i]).strip()
        line2 = regex2.sub("", lines2[i]).strip()
        if line1 != line2:
            print "failed!"
            break
        i += 1
    else:
        print "ok"
