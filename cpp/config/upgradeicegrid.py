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
# 3.0.x version of IceGrid to the new format in 3.1.
#
# Usage:
#
# python upgradeicegrid.py ice30_home ice31_home olddbenv newdbenv
#
# Where:
#
# ice30_home is the path of the Ice 3.0.x distribution.
# ice31_home is the path of the Ice 3.1 distribution.
# olddbenv is the path of the Ice 3.0.x registry database environment
# newdbenv is the path of the Ice 3.1 registry database environment
#

import sys, os

ice30_home = None
ice31_home = None
olddbenv = None
newdbenv = None

#
# Show usage information.
#
def usage():
    print "Usage: " + sys.argv[0] + " ice30_home ice31_home olddbenv newdbenv"
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
    global ice30_home, ice31_home, olddbenv, newdbenv

    transformdb = os.path.join(ice31_home, "bin", "transformdb") + \
              " --include-old " + os.path.join(ice30_home, "slice") + \
              " --include-new " + os.path.join(ice31_home, "slice") + \
              " --old " + os.path.join(ice30_home, "slice", "IceGrid", "Admin.ice") + \
              " --new " + os.path.join(ice31_home, "slice", "IceGrid", "Admin.ice")

    tmpdesc = os.path.join(newdbenv, "tmpdesc" + dbname + ".xml")
    tmpfile = open(tmpdesc, "w+")
    tmpfile.write(desc)
    tmpfile.close()

    pipe = os.popen(transformdb + " -f " + tmpdesc + " " + olddbenv + " " + dbname + " " + newdbenv)
    printOutputFromPipe(pipe)
    os.remove(tmpdesc)
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

if len(sys.argv) < 5:
    usage()
    sys.exit(0)
    
ice30_home = sys.argv[1]
ice31_home = sys.argv[2]
olddbenv = sys.argv[3]
newdbenv = sys.argv[4]

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
