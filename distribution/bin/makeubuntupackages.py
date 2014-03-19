#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
    print "-p [core|all]    Distribution you want to build"
    print "                     * core builds the core distribution"
    print "                     * all builds the main distribution"
    print ""
    print "Example:"
    print ""
    print "makeubuntupackages.py -p all -b ./tmp -d ./dist-HEAD"
    print

#
# Check arguments
#
try:
    opts, args = getopt.getopt(sys.argv[1:], "hvb:d:p:")
except getopt.GetoptError:
    print sys.argv[0] + ": unknown option"
    print
    usage()
    sys.exit(1)


verbose = 0
buildDir = None
distributionDir = None
distribution = None


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
    elif o == "-p":
        distribution = a

if buildDir == None:
    print "Missing -b argument"
    usage()
    sys.exit(1)

if distributionDir == None:
    print "Missing -d argument"
    usage()
    sys.exit(1)

if distribution == None:
    print "Missing -p argument"
    usage()
    sys.exit(1)

if distribution != "all" and distribution != "core":
    print "Unknown distribution: " + distribution
    usage()
    sys.exit(1)


sourceDir = "ice3.5-3.5.1" if distribution == "all" else "ice3.5++-3.5.1"
distFile = "ice3.5_3.5.1.orig.tar.gz" if distribution == "all" else "ice3.5++_3.5.1.orig.tar.gz"
distFiles = "distfiles-3.5.1.tar.gz"

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
    if len(cmd) > 0:
        if verbose:
            print(cmd)
        if os.system(cmd) != 0:
            sys.exit(1)

os.chdir(buildDir)
runCommand("tar zxf %s " % (distFile), verbose)
shutil.copy(distFile, buildDir)

os.chdir(sourceDir)
runCommand("tar zxf %s distfiles-3.5.1/src/deb/%s/debian --strip-components 4" \
    % (distFiles, distribution), verbose)
runCommand("dpkg-buildpackage", verbose)
