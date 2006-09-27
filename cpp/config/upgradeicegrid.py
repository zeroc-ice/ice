#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# This script upgrades the IceGrid registry database environment for
# 3.0.x version of IceGrid to the new format.
#
# Usage:
#
# python upgradeicegrid.py olddbenv newdbenv
#
# Where:
#
# olddbenv is the path of the Ice 3.0.x registry database environment
# newdbenv is the path of the Ice 3.1 registry database environment
#
#
# NOTE: the 3.0 slice definitions for the IceGrid database are stored
# in the icegrid-slice.3.0.tar.gz file. These definitions are used by
# the script to perform the database transformation.
#

import sys, os, gzip

olddbenv = None
newdbenv = None
bindir = None
slicedir = None

#
# Show usage information.
#
def usage():
    print "Usage: " + sys.argv[0] + " olddbenv newdbenv"
    print
    print "Options:"
    print "-h    Show this message."

def printOutputFromPipe(pipe):
    while 1:
        line = pipe.readline()
        if not line:
            break
        os.write(1, line)    

def transformdb(dbname, desc):
    global olddbenv, newdbenv, slicedir, bindir

    oldslice = os.path.join(newdbenv, "icegrid-slice.3.0.ice")
    oldslicefile = open(oldslice, "w+")
    oldslicefile.write(gzip.GzipFile(os.path.join(os.path.dirname(sys.argv[0]), "icegrid-slice.3.0.ice.gz")).read())
    oldslicefile.close()

    tmpdesc = os.path.join(newdbenv, "tmpdesc" + dbname + ".xml")
    tmpfile = open(tmpdesc, "w+")
    tmpfile.write(desc)
    tmpfile.close()

    transformdb = os.path.join(bindir, "transformdb") + \
              " --old " + oldslice + \
              " --include-new " + slicedir + " --new " + os.path.join(slicedir, "IceGrid", "Admin.ice")

    pipe = os.popen(transformdb + " -f " + tmpdesc + " " + olddbenv + " " + dbname + " " + newdbenv)
    printOutputFromPipe(pipe)
    os.remove(tmpdesc)
    os.remove(oldslice)
    if pipe.close():
        sys.exit(1)


#
# Check arguments
#
for x in sys.argv[1:]:
    if x == "-h":
        usage()
        sys.exit(0)
    elif x.startswith("-"):
        print sys.argv[0] + ": unknown option `" + x + "'"
        print
        usage()
        sys.exit(1)

if len(sys.argv) != 3:
    usage()
    sys.exit(0)
    
olddbenv = sys.argv[1]
if not os.path.exists(olddbenv):
    raise "database environment `" + olddbenv + "' doesn't exist"

newdbenv = sys.argv[2]
if not os.path.exists(newdbenv):
    raise "database environment `" + newdbenv + "' doesn't exist"

icedir = os.getenv("ICE_HOME")
if icedir == None:
    icedir = os.path.join(os.path.dirname(sys.argv[0]), "..")

for bindir in [os.path.join(icedir, "bin"), "/usr/bin"]:
    bindir = os.path.normpath(bindir)
    if os.path.exists(os.path.join(bindir, "transformdb")):
        break
else:
    raise "can't locate the `transformdb' executable"

for slicedir in [os.path.join(icedir, "slice"), "/usr/share"]:
    slicedir = os.path.normpath(slicedir)
    if os.path.exists(os.path.join(slicedir, "IceGrid", "Admin.ice")):
        break
else:
    raise "can't locate the IceGrid slice files"

transformdb("applications", \
'<transformdb>' + \
'    <database key="string" value="::IceGrid::ApplicationDescriptor">' + \
'      <record/>' + \
'    </database>' + \
'    <transform type="::IceGrid::CommunicatorDescriptor">' + \
'      <set target="new.propertySet.properties" value="old.properties"/>' + \
'    </transform>' + \
'</transformdb>')

transformdb("adapters", \
'<transformdb>' + \
'    <database key="string" value="::IceGrid::AdapterInfo">' + \
'      <record>' + \
'        <set target="newvalue.id" value="oldkey"/>' + \
'      </record>' + \
'    </database>' + \
'</transformdb>')

transformdb("objects", \
'<transformdb>' + \
'    <database key="::Ice::Identity" value="::IceGrid::ObjectInfo">' + \
'      <record/>' + \
'    </database>' + \
'</transformdb>')
