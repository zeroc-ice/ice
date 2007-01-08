#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import fileinput, re, os, string

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "makedepend.py")):
        break
else:
    raise "can't find toplevel directory!"

if os.path.exists(os.path.join(toplevel, "slice")):
    slicedir = os.path.join(toplevel, "slice")
else:
    slicedir = os.path.join(os.getenv("ICE_HOME", ""), "slice")

previous = ""

for line in fileinput.input():
    line = line.strip()

    line = string.replace(line, slicedir, "$(slicedir)")

    if(previous):
        line = previous + " " + line

    if(line[-1] == "\\"):
        previous = line[:-2]
        continue
    else:
        previous = ""

    for s in line.split():
        if(s[0] != "/"):
            print s,
    print
