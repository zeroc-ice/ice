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
    print "-h           Show this message."
    print "-v           Be verbose."
    print "-n FILE      New Ice distribution archive"
    print "-o FILE      Old Ice distribution archive"
    print "-j NUMBER    Makefile -j option to build distributions"
    print ""
    print "Example:"
    print ""
    print "python run-abi-compliance-checker.py -o Ice-3.5.0.tar.gz -n Ice-3.5.1.tar.gz"
    print

#
# Check arguments
#
try:
    opts, args = getopt.getopt(sys.argv[1:], "hvo:n:j:")
except getopt.GetoptError:
    print sys.argv[0] + ": unknown option"
    print
    usage()
    sys.exit(1)


verbose = 0
newArchive = None
oldArchive = None
outDir = "tmp"
parallelJobs = 1

for (o, a) in opts:
    if o == "-h":
        usage()
        sys.exit(0)
    elif o == "-v":
        verbose = 1
    elif o == "-n":
        newArchive = a
    elif o == "-o":
        oldArchive = a
    elif o == "-j":
        parallelJobs = a
        
if newArchive == None:
    print "Missing -n argument"
    usage()
    sys.exit(1)

if oldArchive == None:
    print "Missing -o argument"
    usage()
    sys.exit(1)

if not os.path.exists(newArchive):
    print "Ice distribution not found in `" + newArchive + "'"
    
if not os.path.exists(oldArchive):
    print "Ice distribution not found in `" + oldArchive + "'"


outDir = os.path.abspath(os.path.join(os.getcwd(), outDir))

oldBaseName = re.sub(".tar.gz", "", os.path.basename(oldArchive))
oldVersion = re.sub("Ice-", "", oldBaseName)
oldDistDir = os.path.join(outDir, "old")
oldSrcDir = os.path.join(outDir, "old", "src")
oldBinDir = os.path.join(outDir, "old", oldBaseName)
oldDescriptor = os.path.join(oldDistDir, oldVersion + ".xml")
oldArchive = os.path.abspath(os.path.join(os.getcwd(), oldArchive))

newBaseName = re.sub(".tar.gz", "", os.path.basename(newArchive))
newVersion = re.sub("Ice-", "", newBaseName)
newDistDir = os.path.join(outDir, "new")
newSrcDir = os.path.join(outDir, "new", "src")
newBinDir = os.path.join(outDir, "new", newBaseName)
newDescriptor = os.path.join(newDistDir, newVersion + ".xml")
newArchive = os.path.abspath(os.path.join(os.getcwd(), newArchive))

if not os.path.exists(outDir):
    os.makedirs(outDir)

def runCommand(cmd, verbose):
    if len(cmd) > 0:
        if verbose:
            print(cmd)
        if os.system(cmd) != 0:
            sys.exit(1)

os.chdir(outDir)

if not os.path.exists(oldSrcDir):
    os.makedirs(oldSrcDir)

if not os.path.exists(newSrcDir):
    os.makedirs(newSrcDir)

if not os.path.exists(oldBinDir):
    os.chdir(oldSrcDir)
    if os.path.exists(os.path.join(oldSrcDir, oldBaseName)):
        shutil.rmtree(os.path.join(oldSrcDir, oldBaseName))
    runCommand("tar zxvf %s" % (oldArchive), verbose)
    os.chdir(os.path.join(oldSrcDir, oldBaseName, "cpp"))
    runCommand("make install -j%s prefix=%s" % (parallelJobs, oldBinDir), verbose)
else:
    print "%s already exists skip building" % (oldBinDir)
    
if not os.path.exists(newBinDir):
    os.chdir(newSrcDir)
    if os.path.exists(os.path.join(newSrcDir, newBaseName)):
        shutil.rmtree(os.path.join(newSrcDir, newBaseName))
    runCommand("tar zxvf %s" % (newArchive), verbose)
    os.chdir(os.path.join(newSrcDir, newBaseName, "cpp"))
    runCommand("make install -j%s prefix=%s" % (parallelJobs, newBinDir), verbose)
else:
    print "%s already exists skip building" % (newBinDir)

    
descriptor = """\
<desc> 
<version>
    @version@
</version>
<headers>
    {RELPATH}/include
</headers>
<libs>
    {RELPATH}/lib/libIceGridFreezeDB.so
    {RELPATH}/lib/libIceGrid.so
    {RELPATH}/lib/libIceBox.so
    {RELPATH}/lib/libIceStormService.so
    {RELPATH}/lib/libIceSSL.so
    {RELPATH}/lib/libGlacier2.so
    {RELPATH}/lib/libIceStorm.so
    {RELPATH}/lib/libSlice.so
    {RELPATH}/lib/libIce.so
    {RELPATH}/lib/libFreeze.so
    {RELPATH}/lib/libIceDB.so
    {RELPATH}/lib/libIceStormFreezeDB.so
    {RELPATH}/lib/libIceXML.so
    {RELPATH}/lib/libIceUtil.so
    {RELPATH}/lib/libIcePatch2.so
</libs>
<skip_headers>
    Perf.h
</skip_headers>
<skip_namespaces> 
    IceUtilInternal
    Slice
    IceInternal
</skip_namespaces>
</desc>
"""

print "writing %s" % (oldDescriptor)
oldFile = open(oldDescriptor, "w")
oldFile.write(re.sub("@version@", oldVersion, descriptor))
oldFile.close()

print "writing %s" % (newDescriptor)
newFile = open(newDescriptor, "w")
newFile.write(re.sub("@version@", newVersion, descriptor))
newFile.close()

runCommand("abi-compliance-checker -l ice -l-full 'Ice' -d1 %s -d2 %s -binary -relpath1 %s -relpath2 %s" % \
           (oldDescriptor, newDescriptor, oldBinDir, newBinDir), verbose)