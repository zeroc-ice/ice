#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import fileinput, re, string

previous = ""

commentre = re.compile("^#")

for line in fileinput.input():
    line = line.strip()

    if commentre.search(line, 0):
        continue;

    line = string.replace(line, ".o:", "$(OBJEXT):")

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
            
