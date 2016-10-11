#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
    os.system("rm -rf project1 tmp")

clean()
os.symlink("slices", "linktoslices")
os.symlink("dir1", os.path.join("slices", "linktodir1"))
os.symlink("a3.ice", os.path.join("slices", "dir1", "linktoa3.ice"))
os.symlink("dir2", os.path.join("slices", "linktodir2"))

slice2cpp = TestUtil.getSliceTranslator()
TestUtil.addLdPath(TestUtil.getCppLibDir())

basedir = os.path.dirname(os.path.abspath(__file__))
slicedir = TestUtil.getSliceDir()
os.symlink(slicedir, "iceslices")

def runTest(cmd):
    os.system(cmd)
    f = open("b.h")
    if not re.search('#include <dir1\/a1\.h>\n'
                     '#include <linktodir1\/a2\.h>\n'
                     '#include <linktodir1\/linktoa3\.h>\n'
                     '#include <Ice\/Identity\.h>\n', f.read(), re.S):
        print("failed!")
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

#
# Slice files are symlinks, include dir is a regular directory
#
os.system("mkdir -p project1/git/services.settings.slices")
os.system("mkdir -p project1/src/services/settings")
os.system("cd project1/src/services/settings &&  ln -s ../../../git/services.settings.slices slices")

f = open("project1/git/services.settings.slices/A.ice", "w")
f.write("// dumy file")
f.close()
f = open("project1/git/services.settings.slices/B.ice", "w")
f.write("#include <services/settings/slices/A.ice>")
f.close()

os.system("cd project1 && %s -Isrc src/services/settings/slices/B.ice" % slice2cpp)
f = open("project1/B.h")

if not re.search(re.escape('#include <services/settings/slices/A.h>'), f.read()):
    print("failed!")
    sys.exit(1)

clean()

#
# Slice file is regular file, include dir is a symlink to a second symlink
#
os.system("mkdir -p tmp/Ice-x.y.z/share")
os.system("cd tmp/Ice-x.y.z/share && ln -s %s" % slicedir)


os.system("mkdir -p project1/share")
os.system("cd project1/share && ln -s %s/tmp/Ice-x.y.z/share/slice" % basedir)
f = open("project1/A.ice", "w")
f.write("#include <Ice/Identity.ice>")
f.close()
os.system("cd project1 && %s -Ishare/slice A.ice" % slice2cpp)
f = open("project1/A.h")
if not re.search(re.escape('#include <Ice/Identity.h>'), f.read()):
    print("failed!")
    sys.exit(1)

clean()

#
# Typical Ice install with symlink Ice-x.y -> Ice-x.y.z
#
os.system("mkdir -p tmp/Ice-x.y.z/slice/Ice")
os.system("cd tmp && ln -s Ice-x.y.z Ice-x.y")
f = open("tmp/Ice-x.y.z/slice/Ice/Identity.ice", "w")
f.write("// dumy file")

os.system("mkdir -p project1")
f = open("project1/A.ice", "w")
f.write("#include <Ice/Identity.ice>")
f.close()
os.system("cd project1 && %s -I%s/tmp/Ice-x.y/slice A.ice" % (slice2cpp, basedir))
f = open("project1/A.h")
if not re.search(re.escape('#include <Ice/Identity.h>'), f.read()):
    print("failed!")
    sys.exit(1)

clean()

#
# symlink directory with extra / at end
#
#
os.system("mkdir -p tmp/Ice-x.y.z/slice/Ice")
os.system("mkdir -p tmp/Ice")
os.system("cd tmp/Ice && ln -s ../Ice-x.y.z/slice/ .")
f = open("tmp/Ice-x.y.z/slice/Ice/Identity.ice", "w")
f.write("// dumy file")
f.close()
os.system("mkdir -p project1")
f = open("project1/A.ice", "w")
f.write("#include <Ice/Identity.ice>")
f.close()
os.system("cd project1 && %s -I%s/tmp/Ice/slice A.ice" % (slice2cpp, basedir))
f = open("project1/A.h")
if not re.search(re.escape('#include <Ice/Identity.h>'), f.read()):
    print("failed!2")
    sys.exit(1)
clean()

print("ok")
