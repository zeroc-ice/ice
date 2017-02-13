#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# This script upgrades the IceGrid registry database environment for
# 3.5.x version of IceGrid to the new format (>= 3.6).
#
# Usage:
#
# python upgradeicegrid36.py olddbenv newdbenv
#
# Where for example:
#
# olddbenv is the path of the Ice 3.5.x registry database environment
# newdbenv is the path of new registry database environment
#
#
# NOTE: the 3.x slice definitions for the IceGrid database are stored
# in the icegrid-slice.3.x.tar.gz file. These definitions are used by
# the script to perform the database transformation.
#

import sys, os, gzip, time, shutil, getopt

olddbenv = None
newdbenv = None
bindir = None
slicedir = None

transformdbExe = "transformdb"
dumpdbExe = "dumpdb"

win32 = (sys.platform == "win32")
if win32:
    transformdbExe += ".exe"
    dumpdbExe += ".exe"
#
# Show usage information.
#
def usage():
    print("Usage: " + sys.argv[0] + " [options] olddbenv newdbenv")
    print("")
    print("This script upgrades a 3.5 IceGrid registry database environment")
    print("to an IceGrid registry 3.6 (or newer) database environment.")
    print("")
    print("Options:")
    print("-h                      Show this message.")
    print("--server-version VER    Specifies an alternate Ice version for your")
    print("                        IceGrid servers.")
    sys.exit(2)

def printOutputFromPipe(pipe):
    while 1:
        line = pipe.readline()
        if not line:
            break
        if line.find("warning") == -1:
            sys.stdout.write(line)

def error(message):
    print("error: " + message)
    sys.exit(1)

def transformdb(olddbenv, newdbenv, db, desc, oldslice, newslice):
    global bindir

    oldslicefile = open(os.path.join(newdbenv, oldslice), "wb+")
    oldslicefile.write(gzip.GzipFile(os.path.join(os.path.dirname(__file__), oldslice + ".gz")).read())
    oldslicefile.close()

    newslicefile = open(os.path.join(newdbenv, newslice), "wb+")
    newslicefile.write(gzip.GzipFile(os.path.join(os.path.dirname(__file__), newslice + ".gz")).read())
    newslicefile.close()

    tmpdesc = os.path.join(newdbenv, "tmpdesc.xml")
    tmpfile = open(tmpdesc, "w+")
    tmpfile.write(desc)
    tmpfile.close()

    transformdb = "\"" + os.path.join(bindir, transformdbExe) + "\" -i" + \
                  " --old " + os.path.join(newdbenv, oldslice) + \
                  " --new " + os.path.join(newdbenv, newslice)

    pipe = os.popen(transformdb + " -f " + tmpdesc + " " + olddbenv + " " + db + " " + newdbenv + " 2>&1" )
    printOutputFromPipe(pipe)
    os.remove(tmpdesc)
    os.remove(os.path.join(newdbenv, oldslice))
    os.remove(os.path.join(newdbenv, newslice))
    if pipe.close():
        sys.exit(1)

def upgrade35(olddbenv, newdbenv, iceServerVersion):

    databases = \
              '<database name="adapters" key="string" value="::IceGrid::AdapterInfo"><record/></database>' + \
              '<database name="applications" key="string" value="::IceGrid::ApplicationInfo"><record/></database>' + \
              '<database name="internal-objects" key="::Ice::Identity" value="::IceGrid::ObjectInfo">' + \
              '<record/></database>' + \
              '<database name="objects" key="::Ice::Identity" value="::IceGrid::ObjectInfo"><record/></database>'

    if iceServerVersion and not iceServerVersion.startswith("3.6"):
        desc = \
             '<transformdb>' + \
             databases + \
             '    <transform type="::IceGrid::ServerDescriptor">' + \
             '        <set target="new.iceVersion" value="\'' + iceServerVersion + '\'"/>' + \
             '    </transform>' + \
             '</transformdb>'
    else:
        desc = \
             '<transformdb>' + \
             databases + \
             '</transformdb>'
            
    transformdb(olddbenv, newdbenv, "objects", desc, "icegrid-slice.3.5.ice", "icegrid-slice.3.6.ice")
    transformdb(olddbenv, newdbenv, "adapters", desc, "icegrid-slice.3.5.ice", "icegrid-slice.3.6.ice")
    transformdb(olddbenv, newdbenv, "applications", desc, "icegrid-slice.3.5.ice", "icegrid-slice.3.6.ice")
    if os.path.exists(os.path.join(olddbenv, "internal-objects")):
        transformdb(olddbenv, newdbenv, "internal-objects", desc, "icegrid-slice.3.5.ice", "icegrid-slice.3.6.ice")

#
# Check arguments
#

try:
    opts, args = getopt.getopt(sys.argv[1:], "hs:", ["help", "server-version="])
except getopt.GetoptError:
    usage()
    
if not args or len(args) != 2:
    print(sys.argv[0] + ": missing database environment arguments")
    usage()
olddbenv = args[0]
newdbenv = args[1]

serverVersion = None
for o, a in opts:
    if o in ("-h", "--help"):
        usage()
    if o in ("-s", "--server-version"):
        serverVersion = a
    
if not os.path.exists(olddbenv):
    error("database environment `" + olddbenv + "' doesn't exist")

if not os.path.exists(newdbenv):
    error("database environment `" + newdbenv + "' doesn't exist")
elif os.path.exists(os.path.join(newdbenv, "applications")) or \
     os.path.exists(os.path.join(newdbenv, "adapters")) or \
     os.path.exists(os.path.join(newdbenv, "internal-objects")) or \
     os.path.exists(os.path.join(newdbenv, "objects")):
    error("database environment `" + newdbenv + "' already has databases")

for bindir in [os.path.join(os.path.dirname(__file__), "..", "bin"), "/usr/bin"]:
    bindir = os.path.normpath(bindir)
    if os.path.exists(os.path.join(bindir, transformdbExe)):
        break

else:
    #
    # Check if transformdb and dumpdb are present in path
    #
    if(os.system(transformdbExe + " -v") != 0):
        error("can't locate the `" + transformdbExe + "' executable")
    
    if(os.system(dumpdbExe + " -v") != 0):
        error("can't locate the `" + dumpdbExe + "' executable")

    #
    # Use transformdb and dumpdb from system path
    #
    print("Using transformdb and dumpdb from system path")
    bindir = ""


sys.stdout.write("upgrading 3.5 database environment...")
sys.stdout.flush()
upgrade35(olddbenv, newdbenv, serverVersion)
print("ok")
