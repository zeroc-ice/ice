#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# This script upgrades the IceGrid registry database environment for
# 3.1.x or 3.2.x version of IceGrid to the new format (>= 3.3).
#
# Usage:
#
# python upgradeicegrid33.py olddbenv newdbenv
#
# Where for example:
#
# olddbenv is the path of the Ice 3.1.x registry database environment
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
    print "Usage: " + sys.argv[0] + " [options] olddbenv newdbenv"
    print
    print "This script upgrades a 3.1 or 3.2 IceGrid registry database environment"
    print "to an IceGrid registry 3.3 (or newer) database environment."
    print
    print "Options:"
    print "-h                      Show this message."
    print "--server-version VER    Specifies an alternate Ice version for your"
    print "                        IceGrid servers."
    sys.exit(2)

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

def transformdb(olddbenv, newdbenv, db, desc, oldslice, newslice):
    global bindir

    oldslicefile = open(os.path.join(newdbenv, oldslice), "w+")
    oldslicefile.write(gzip.GzipFile(os.path.join(os.path.dirname(__file__), oldslice + ".gz")).read())
    oldslicefile.close()

    newslicefile = open(os.path.join(newdbenv, newslice), "w+")
    newslicefile.write(gzip.GzipFile(os.path.join(os.path.dirname(__file__), newslice + ".gz")).read())
    newslicefile.close()

    tmpdesc = os.path.join(newdbenv, "tmpdesc.xml")
    tmpfile = open(tmpdesc, "w+")
    tmpfile.write(desc)
    tmpfile.close()

    transformdb = os.path.join(bindir, transformdbExe) + " -i" + \
                  " --old " + os.path.join(newdbenv, oldslice) + \
                  " --new " + os.path.join(newdbenv, newslice)

    pipe = os.popen(transformdb + " -f " + tmpdesc + " " + olddbenv + " " + db + " " + newdbenv + " 2>&1" )
    printOutputFromPipe(pipe)
    os.remove(tmpdesc)
    os.remove(os.path.join(newdbenv, oldslice))
    os.remove(os.path.join(newdbenv, newslice))
    if pipe.close():
        sys.exit(1)

def upgrade31(olddbenv, newdbenv):
    desc = \
         '<transformdb>' + \
         ' ' + \
         '    <database name="applications" ' + \
         '              key="string" ' + \
         '              value="::IceGrid::ApplicationDescriptor,::IceGrid::ApplicationInfo">' + \
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
         '    <database name="objects" key="::Ice::Identity" value="::IceGrid::ObjectInfo"><record/></database>' + \
         '    <database name="adapters" key="string" value="::IceGrid::AdapterInfo"><record/></database>' + \
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

    transformdb(olddbenv, newdbenv, "objects", desc, "icegrid-slice.3.1.ice", "icegrid-slice.3.2.ice")
    transformdb(olddbenv, newdbenv, "adapters", desc, "icegrid-slice.3.1.ice", "icegrid-slice.3.2.ice")
    transformdb(olddbenv, newdbenv, "applications", desc, "icegrid-slice.3.1.ice", "icegrid-slice.3.2.ice")

def upgrade32(olddbenv, newdbenv, iceServerVersion):

    databases = \
              '<database name="adapters" key="string" value="::IceGrid::AdapterInfo"><record/></database>' + \
              '<database name="applications" key="string" value="::IceGrid::ApplicationInfo"><record/></database>' + \
              '<database name="internal-objects" key="::Ice::Identity" value="::IceGrid::ObjectInfo">' + \
              '<record/></database>' + \
              '<database name="objects" key="::Ice::Identity" value="::IceGrid::ObjectInfo"><record/></database>'

    if iceServerVersion and not iceServerVersion.startswith("3.3") and not iceServerVersion.startswith("3.4"):
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
             '    <transform type="::IceGrid::AdapterDescriptor">' + \
             '        <set target="new.registerProcess" value="false"/>' + \
             '    </transform>' + \
             '    <transform type="::IceGrid::ServerDescriptor">' + \
             '       <define name="adminEndpointProp" type="::IceGrid::PropertyDescriptor"/>' + \
             '       <set target="adminEndpointProp.name" value="\'Ice.Admin.Endpoints\'"/>' + \
             '       <set target="adminEndpointProp.value" value="\'tcp -h 127.0.0.1\'"/>' + \
             '       <add target="new.propertySet.properties" index="0" value="adminEndpointProp"/>' + \
             '    </transform>' + \
             '</transformdb>'
             
    transformdb(olddbenv, newdbenv, "objects", desc, "icegrid-slice.3.2.ice", "icegrid-slice.3.3.ice")
    transformdb(olddbenv, newdbenv, "adapters", desc, "icegrid-slice.3.2.ice", "icegrid-slice.3.3.ice")
    transformdb(olddbenv, newdbenv, "applications", desc, "icegrid-slice.3.2.ice", "icegrid-slice.3.3.ice")
    if os.path.exists(os.path.join(olddbenv, "internal-objects")):
        transformdb(olddbenv, newdbenv, "internal-objects", desc, "icegrid-slice.3.2.ice", "icegrid-slice.3.3.ice")

def getIceGridEnvVersion(dbenv):
    global bindir

    pipe = os.popen(os.path.join(bindir, dumpdbExe) + " -c " + dbenv + " 2>&1")
    ver = None
    for line in pipe.readlines():
        if line.find("value type = ::IceGrid::ApplicationDescriptor") > 0:
            ver = "3.1"
            break
        elif line.find("value type = ::IceGrid::ApplicationInfo") > 0:
            ver = "3.2"
            break
        
    if pipe.close():
        sys.exit(1)

    return ver

#
# Check arguments
#

try:
    opts, args = getopt.getopt(sys.argv[1:], "hs:", ["help", "server-version="])
except getopt.GetoptError:
    usage()
    
if not args or len(args) != 2:
    print sys.argv[0] + ": missing database environment arguments"
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
    print "Check " + transformdbExe + " -v"
    if(os.system(transformdbExe + " -v") != 0):
        print "...error"
        error("can't locate the `" + transformdbExe + "' executable")
    
    print "Check " + dumpdbExe + " -v "
    if(os.system(dumpdbExe + " -v") != 0):
        print "...error"
        error("can't locate the `" + dumpdbExe + "' executable")

    #
    # Use transformdb and dumpdb from system path
    #
    print "Using transformdb and dumpdb from system path"
    bindir = ""


dbEnvVersion = getIceGridEnvVersion(olddbenv)
if dbEnvVersion == "3.1":
    print "upgrading 3.1 database environment...",
    sys.stdout.flush()
    os.mkdir(os.path.join(newdbenv, "3.2"))
    upgrade31(olddbenv, os.path.join(newdbenv, "3.2"))
    upgrade32(os.path.join(newdbenv, "3.2"), newdbenv, "3.3")
    shutil.rmtree(os.path.join(newdbenv, "3.2"))
    print "ok"
elif dbEnvVersion == "3.2":
    print "upgrading 3.2 database environment...",
    sys.stdout.flush()
    upgrade32(olddbenv, newdbenv, serverVersion)
    print "ok"
else:
    error("can't figure out the version of the IceGrid database environment `" + olddbenv + "'")
