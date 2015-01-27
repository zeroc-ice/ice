#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, re

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil

def clean():
    for f in ["iceslices", 
              "linktoslices", 
              os.path.join("slices", "linktodir2"), 
              os.path.join("slices", "linktodir1"),
              os.path.join("slices", "dir1", "linktoa3.ice")]:
        if os.path.exists(f):
            os.unlink(f)

clean()
os.symlink("slices", "linktoslices")
os.symlink("dir1", os.path.join("slices", "linktodir1"))
os.symlink("a3.ice", os.path.join("slices", "dir1", "linktoa3.ice"))
os.symlink("dir2", os.path.join("slices", "linktodir2"))

slice2cpp = TestUtil.getSliceTranslator()

slicedir = os.path.join(TestUtil.getIceDir(), "slice")
os.symlink(slicedir, "iceslices")

def runTest(cmd):
    os.system(cmd)
    f = open("b.h")
    if not re.search('#include <dir1\/a1\.h>\n'
                     '#include <linktodir1\/a2\.h>\n'
                     '#include <linktodir1\/linktoa3\.h>\n'
                     '#include <Ice\/Identity\.h>\n', f.read(), re.S):
        print "failed!"
        sys.exit(1)
    os.unlink("b.h")
    os.unlink("b.cpp")

sys.stdout.write("compiling slice files and checking headers... ")
sys.stdout.flush()
runTest("%s -Iiceslices -Islices slices/dir2/b.ice" % (slice2cpp))
runTest("%s -Iiceslices -I../headers/slices slices/dir2/b.ice" % (slice2cpp))
runTest("%s -Iiceslices -Ilinktoslices slices/dir2/b.ice" % (slice2cpp))
runTest("%s -Iiceslices -Ilinktoslices/../linktoslices slices/dir2/b.ice" % (slice2cpp))
runTest("%s -I%s -Islices linktoslices/dir2/b.ice" % (slice2cpp, slicedir))
runTest("%s -I%s -Ilinktoslices linktoslices/linktodir2/b.ice" % (slice2cpp, slicedir))

# Also ensure it works with case insensitive file system
if os.path.exists("SLICES"):
    runTest("%s -IICESLICES -ISLICES SLICES/DIR2/B.ice" % (slice2cpp))
    runTest("%s -IICESLICES -ILINKTOSLICES LINKTOSLICES/LINKTODIR2/B.ice" % (slice2cpp))

print("ok")

clean()
