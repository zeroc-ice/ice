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

for top_srcdir in [".", "..", "../..", "../../..", "../../../.."]:
    top_srcdir = os.path.normpath(top_srcdir)
    if os.path.exists(os.path.join(top_srcdir, "..", "config", "makedepend.py")):
        break
else:
    raise RuntimeError("can't find top level source directory!")


prefix = None

if len(sys.argv) > 1:
    prefix = sys.argv[1]


lang = None
for line in fileinput.input("-"):
    line = line.strip()

    if line.endswith(": \\"):
        line = prefix + line

    print(line)
