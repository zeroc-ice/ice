#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, fnmatch, re, getopt, atexit, shutil, subprocess

#
# Program usage.
#
def usage():
    print
    print "Options:"
    print "-h               Show this message."
    print "-v               Be verbose."
    print "-b DIR           Directory to build the packages"
    print "-d DIR           Directory with Ice source distribution"
    print "-k KEYID         Id of the Key used to sign the packages"
    print "Example:"
    print ""
    print "makeubuntupackages.py -b trusty -d dist-HEAD -k 748BB043"
    print

#
# Check arguments
#
try:
    opts, args = getopt.getopt(sys.argv[1:], "hvb:d:k:")
except getopt.GetoptError:
    print sys.argv[0] + ": unknown option"
    print
    usage()
    sys.exit(1)

iceVersion = "3.6.0"
debVersion = "3.6.0"
mmVersion = "3.6"

verbose = 0
buildDir = None
distributionDir = None
keyid = None

for (o, a) in opts:
    if o == "-h":
        usage()
        sys.exit(0)
    elif o == "-v":
        verbose = 1
    elif o == "-b":
        buildDir = a
    elif o == "-d":
        distributionDir = a
    elif o == "-k":
        keyid = a

if buildDir == None:
    print "Missing -b argument"
    usage()
    sys.exit(1)

if distributionDir == None:
    print "Missing -d argument"
    usage()
    sys.exit(1)

buildpackageOps = ("-k%s" % keyid) if keyid != None else "-us -uc"

sourceDir = "zeroc-ice" + mmVersion + "-" + debVersion
distFile = "zeroc-ice" + mmVersion + "_" + debVersion + ".orig.tar.gz"
distFiles = "distfiles-" + iceVersion + ".tar.gz"

buildDir = os.path.abspath(os.path.join(os.getcwd(), buildDir))
sourceDir = os.path.abspath(os.path.join(buildDir, sourceDir))
distributionDir = os.path.abspath(os.path.join(os.getcwd(), distributionDir))

distFile = os.path.join(distributionDir, distFile)
distFiles = os.path.join(distributionDir, distFiles)

if not os.path.exists(buildDir):
    os.mkdir(buildDir)
else:
    if not os.listdir(buildDir) == []:
        print "Build dir `%s' not empty" % (buildDir)
        sys.exit(1)

if not os.path.exists(distFiles):
    print "File not exists %s " % (distFiles)
    print "Path `%s' doesn't appears to contain a valid distribution" % (distributionDir)
    sys.exit(1)

if not os.path.exists(distFile):
    print "File not exists %s " % (distFile)
    print "Path `%s' doesn't appears to contain a valid distribution" % (distributionDir)
    sys.exit(1)


def runCommand(cmd, verbose):
    print(cmd)
    if os.system(cmd) != 0:
        sys.exit(1)

os.chdir(buildDir)
runCommand("tar zxf %s " % (distFile), verbose)
shutil.copy(distFile, buildDir)

os.chdir(sourceDir)
runCommand("tar zxf %s distfiles-" + iceVersion + "/src/deb/debian --strip-components 3" % distFiles, verbose)
os.chdir(os.path.join(sourceDir, "debian"))
runCommand("tar zxf %s distfiles-" + iceVersion + "/src/unix/README.Linux --strip-components 3" % distFiles, verbose)
runCommand("tar zxf %s distfiles-" + iceVersion + "/src/unix/JGOODIES_LICENSE --strip-components 3" % distFiles, verbose)

os.chdir(sourceDir)
runCommand("dpkg-buildpackage %s -j8" % buildpackageOps, verbose)
