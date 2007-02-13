#!/usr/bin/env python

# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys

def usage():
    print >> sys.stderr, "usage: " + sys.argv[0] + " slice-dir output-file"
    sys.exit(1)

if (len(sys.argv)) != 3:
    usage();

if not os.path.exists(sys.argv[1]):
    print >> sys.stderr, "`" + sys.argv[1] + "' does not exist."
    sys.exit(1);

if not os.path.isdir(sys.argv[1]):
    print >> sys.stderr, "`" + sys.argv[1] + "' is not a directory."
    sys.exit(1);

out = file(sys.argv[2], "w+")

print >> out, "SLICEFILES =",
first = True
for root, dirs, files in os.walk(sys.argv[1]):
    for name in files:
        if not name.startswith(".") and name.endswith(".ice"):
            if not first:
                print >> out, "\\"
                print >> out, "            ",
            else:
                first = False
            print >> out, os.path.join(root, name),
print >> out, "\n"

sys.exit(0)
