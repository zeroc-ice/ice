#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2002
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

import os, sys, fnmatch, re

#
# Remove a file or directory (recursive).
#
def rm(path):
    if os.path.isdir(path) and not os.path.islink(path):
        for x in os.listdir(path):
            rm(os.path.join(path, x))
        os.rmdir(path)
    else:
        os.remove(path)

#
# Check arguments
#
tag = "-rHEAD"
for x in sys.argv[1:]:
    if x == "-h":
        print "usage: " + sys.argv[0] + " [-h] [tag]"
        sys.exit(0)
    else:
        tag = "-r" + x

#
# Remove any existing "dist" directory and create a new one.
#
if os.path.exists("dist"):
    rm("dist")
os.mkdir("dist")
os.chdir("dist")

#
# Export sources from CVS.
#
os.system("cvs -z5 -d cvs.mutablerealms.com:/home/cvsroot export " + tag + " icej")

#
# Remove files.
#
filesToRemove = [ \
    "icej/makedist.py", \
    ]
for x in filesToRemove:
    rm(x)

#
# Get Ice version.
#
config = open("icej/src/IceUtil/Version.java", "r")
version = re.search("ICE_STRING_VERSION = \"(.*)\"", config.read()).group(1)

#
# Create archives.
#
icever = "IceJ-" + version
os.mkdir(icever)
os.rename("icej", os.path.join(icever, "icej"))
os.system("tar cvzf " + icever + ".tar.gz " + icever)
os.system("zip -9 -r " + icever + ".zip " + icever)

#
# Copy files (README, etc.).
#

#
# Done.
#
rm(icever)
