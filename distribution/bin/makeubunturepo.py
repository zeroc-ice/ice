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
    print "-k KEYID     Specify keyid to use when signing (support@zeroc.com)"
    print "-o OSNAME    Ubuntu/Debian os name (raring, quantal, precise, ... )"
    print "-p DIR       Directory containing the packages to add"
    print "-r DIR       Repository base directory"
    print ""
    print "Example:"
    print ""
    print "makeubunturepo.py -k support@zeroc.com -o raring -p ./packages -r ./ubuntu"
    print

#
# Check arguments
#
try:
    opts, args = getopt.getopt(sys.argv[1:], "hvk:o:p:r:")
except getopt.GetoptError:
    print sys.argv[0] + ": unknown option"
    print
    usage()
    sys.exit(1)


verbose = 0
keyid = None
osName = None
packagesDir = None
repositoryDir = None

for (o, a) in opts:
    if o == "-h":
        usage()
        sys.exit(0)
    elif o == "-v":
        verbose = 1
    elif o == "-k":
        keyid = a
    elif o == "-o":
        osName = a
    elif o == "-p":
        packagesDir = a
    elif o == "-r":
        repositoryDir = a

if keyid == None:
    print "Missing -k argument"
    usage()
    sys.exit(1)

if osName == None:
    print "Missing -o argument"
    usage()
    sys.exit(1)

if packagesDir == None:
    print "Missing -p argument"
    usage()
    sys.exit(1)

if repositoryDir == None:
    print "Missing -r argument"
    usage()
    sys.exit(1)

packagesDir = os.path.abspath(os.path.join(os.getcwd(), packagesDir))
repositoryDir = os.path.abspath(os.path.join(os.getcwd(), repositoryDir))
tmpDir = os.path.join(repositoryDir, "tmp")

if not os.path.exists(tmpDir):
    os.mkdir(tmpDir)

for root, dirnames, filenames in os.walk(packagesDir):
    for f in filenames:
        if f.endswith(".deb") or f.endswith(".dsc") or f.endswith(".tar.gz"):
            if not os.path.exists(os.path.join(tmpDir, f)):
                shutil.copy(os.path.join(root, f), os.path.join(tmpDir, f))

print("adding packages for OS: %s from: %s to repository: %s" % (osName, packagesDir, repositoryDir))

os.chdir(repositoryDir)

def runCommand(cmd, verbose):
    if len(cmd) > 0:
        if verbose:
            print(cmd)
        if os.system(cmd) != 0:
            sys.exit(1)

for root, dirnames, filenames in os.walk(tmpDir):
    for f in filenames:
        if not f.endswith(".deb") and not f.endswith(".dsc"):
            continue

        #
        # Remove the package if already exists in the repository.
        #
        i = f.index("_")
        packageName = f[:i]
        arch = ""
        archFlags = ""
        remove = "remove " + osName + " " + packageName
        listpackages = "list " + osName + " " + packageName

        if f.endswith(".deb"):
            i = f.rindex("_")
            dot = f.rindex(".")
            arch = f[i + 1:dot]
        elif f.endswith(".dsc"):
            arch = "source"

        if arch != "all":
            archFlags = " -A " + arch

        print("Adding package %s arch: %s" % (packageName, arch))
        p = subprocess.Popen("reprepro -C 'main' %s %s" % (archFlags, listpackages), 
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True)

        while  True:
            r = p.stdout.readline().decode("utf-8").strip()
            if r == '':
                break

            archs = []
            if arch == "all":
                archs = ["i386", "amd64"]
            else:
                archs = [arch]

            for a in archs:
                if r.find("%s|main|%s: %s" % (osName, a, packageName)) != -1:
                    runCommand("reprepro -C 'main' %s %s 1> /dev/null" % (" -A " + a, remove), verbose)

        include = "includedeb" if f.endswith(".deb") else "includedsc"

        runCommand("reprepro --ask-passphrase -Vb . -C main %s %s %s %s 1> /dev/null" % 
            (archFlags, include, osName, os.path.join(tmpDir, f)), verbose)

shutil.rmtree(tmpDir)
os.chdir(os.path.dirname(repositoryDir))
print("Creating repository archive in " + os.path.basename(repositoryDir) + ".tar.gz")
os.system("GZIP=-9 tar --exclude=db --exclude=conf -czf " + os.path.basename(repositoryDir) + ".tar.gz " + 
    os.path.basename(repositoryDir))
