#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2001
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
#
# **********************************************************************

import fileinput

previous = ""

for line in fileinput.input():
    line = line.strip()

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
            
