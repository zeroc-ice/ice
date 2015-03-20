#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os, fileinput

prefix = None
if len(sys.argv) > 1:
    prefix = sys.argv[1]

lang = None
for line in fileinput.input("-"):
    line = line.strip()

    if line.endswith(": \\"):
        line = prefix + line

    print(line)
