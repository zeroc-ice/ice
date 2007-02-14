#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
# olddbenv is the path of the Ice 3.1.x registry database environment
# newdbenv is the path of the Ice 3.2 registry database environment
#
#
# NOTE: the 3.1 slice definitions for the IceGrid database are stored
# in the icegrid-slice.3.1.tar.gz file. These definitions are used by
# the script to perform the database transformation.
#

import sys, os, gzip, time

olddbenv = None
newdbenv = None
bindir = None
slicedir = None

#
# Show usage information.
#
def usage():
    print "Usage: " + sys.argv[0] + " [options] olddbenv newdbenv"
    print
    print "Options:"
    print "-h    Show this message."

def printOutputFromPipe(pipe):
    while 1:
        line = pipe.readline()
        if not line:
            break
        if line.find("warning") == -1:
            os.write(1, line)

def error(message):
    print "error: " + message
    sys.exit(1)

def transformdb(db, desc):
    global olddbenv, newdbenv, slicedir, bindir

    oldslice = os.path.join(newdbenv, "icegrid-slice.3.1.ice")
    oldslicefile = open(oldslice, "w+")
    oldslicefile.write(gzip.GzipFile(os.path.join(os.path.dirname(sys.argv[0]), "icegrid-slice.3.1.ice.gz")).read())
    oldslicefile.close()

    tmpdesc = os.path.join(newdbenv, "tmpdesc.xml")
    tmpfile = open(tmpdesc, "w+")
    tmpfile.write(desc)
    tmpfile.close()

    transformdb = os.path.join(bindir, "transformdb") + \
              " -i --old " + oldslice + \
              " --include-new " + slicedir + " --new " + os.path.join(slicedir, "IceGrid", "Admin.ice")

    pipe = os.popen(transformdb + " -f " + tmpdesc + " " + olddbenv + " " + db + " " + newdbenv + " 2>&1" )
    printOutputFromPipe(pipe)
    os.remove(tmpdesc)
    os.remove(oldslice)
    if pipe.close():
        sys.exit(1)

#
# Check arguments
#
olddbenv = ""
newdbenv = ""
for x in sys.argv[1:]:
    if x == "-h":
        usage()
        sys.exit(0)
    elif x.startswith("-"):
        print sys.argv[0] + ": unknown option `" + x + "'"
        print
        usage()
        sys.exit(1)
    elif olddbenv == "":
        olddbenv = x
    elif newdbenv == "":
        newdbenv = x
    else:
        usage()
        sys.exit(0)
    
if not os.path.exists(olddbenv):
    error("database environment `" + olddbenv + "' doesn't exist")

if not os.path.exists(newdbenv):
    error("database environment `" + newdbenv + "' doesn't exist")
elif os.path.exists(os.path.join(newdbenv, "applications")) or \
     os.path.exists(os.path.join(newdbenv, "adapters")) or \
     os.path.exists(os.path.join(newdbenv, "objects")):
    error("database environment `" + newdbenv + "' already has databases")

icedir = os.getenv("ICE_HOME")
if icedir == None:
    icedir = os.path.join(os.path.dirname(sys.argv[0]), "..")

for bindir in [os.path.join(icedir, "bin"), "/usr/bin"]:
    bindir = os.path.normpath(bindir)
    if os.path.exists(os.path.join(bindir, "transformdb")):
        break
else:
    error("can't locate the `transformdb' executable")

for slicedir in [os.path.join(icedir, "slice"), "/usr/share/slice"]:
    slicedir = os.path.normpath(slicedir)
    if os.path.exists(os.path.join(slicedir, "IceGrid", "Admin.ice")):
        break
else:
    error("can't locate the IceGrid slice files")

desc = \
'<transformdb>' + \
' ' + \
'    <database name="applications" key="string" value="::IceGrid::ApplicationDescriptor,::IceGrid::ApplicationInfo">' + \
'       <record>' + \
'            <set target="newvalue.revision" value="1"/>' + \
'            <set target="newvalue.uuid" value="generateUUID()"/>' + \
'            <set target="newvalue.createUser" value="\'IceGrid Registry (database upgrade)\'"/>' + \
'            <set target="newvalue.updateUser" value="\'IceGrid Registry (database upgrade)\'"/>' + \
'            <set target="newvalue.createTime" value="' + str(int(time.time() * 1000)) + '"/>' + \
'            <set target="newvalue.updateTime" value="' + str(int(time.time() * 1000)) + '"/>' + \
'            <set target="newvalue.descriptor" value="oldvalue"/>' + \
'        </record>' + \
'    </database>' + \
'    <database name="objects" key="::Ice::Identity" value="::IceGrid::ObjectInfo"/>' + \
'    <database name="adapters" key="string" value="::IceGrid::AdapterInfo"/>' + \
'' + \
'    <transform type="::IceGrid::AdapterDescriptor">' + \
'        <set target="new.serverLifetime" value="old.waitForActivation"/>' + \
'    </transform>' + \
'' + \
'    <transform type="::IceGrid::ReplicaGroupDescriptor">' + \
'         <if test="old.loadBalancing == nil">' + \
'              <set target="new.loadBalancing" type="::IceGrid::RandomLoadBalancingPolicy"/>' + \
'              <set target="new.loadBalancing.nReplicas" value="\'0\'"/>' + \
'         </if>' + \
'         <if test="old.loadBalancing != nil and old.loadBalancing.nReplicas == \'0\'">' + \
'              <set target="new.loadBalancing.nReplicas" value="\'1\'"/>' + \
'         </if>' + \
'    </transform>' + \
'    ' + \
'</transformdb>'

transformdb("objects", desc)
transformdb("adapters", desc)
transformdb("applications", desc)
