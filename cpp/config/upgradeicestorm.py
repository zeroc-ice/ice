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
# This script upgrades an IceStorm registry database environment from
# the 3.1.x version of IceStorm to the new format.
#
# Usage:
#
# python upgradeicestorm.py olddbenv newdbenv
#
# Where:
#
# olddbenv is the path of the Ice 3.1.x registry database environment
# newdbenv is the path of the Ice 3.2 registry database environment
#

import sys, os, tempfile

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

def transformdb(db, desc, oldSlice, newSlice):
    global olddbenv, newdbenv, slicedir, bindir

    temp, oldSliceFile = tempfile.mkstemp("oldslice.ice")
    os.write(temp, oldSlice)
    os.close(temp)

    temp, newSliceFile = tempfile.mkstemp("newslice.ice")
    os.write(temp, newSlice)
    os.close(temp)

    temp, descFile = tempfile.mkstemp("desc.xml")
    os.write(temp, desc)
    os.close(temp)

    transformdb = os.path.join(bindir, "transformdb") + " -i --old " + oldSliceFile + " --new " + newSliceFile + \
        " -f " + descFile + " " + olddbenv + " " + db + " " + newdbenv + " 2>&1"

    pipe = os.popen(transformdb)
    printOutputFromPipe(pipe)
    os.remove(oldSliceFile)
    os.remove(newSliceFile)
    os.remove(descFile)
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
elif os.path.exists(os.path.join(newdbenv, "topics")):
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

desc = \
'<transformdb>' + \
'    <database key="string,Ice::Identity" value="IceStorm::LinkRecordDict,IceStorm::LinkRecordSeq">' + \
'      <record>' + \
'          <set target="newkey.name" value="oldkey"/>' + \
'          <iterate target="oldvalue">' + \
'            <add target="newvalue" index="0" value="value"/>' + \
'          </iterate>' + \
'      </record>' + \
'    </database>' + \
'</transformdb>'

oldSlice = \
'module IceStorm' + \
'{' + \
'interface TopicLink;' + \
'interface Topic;' + \
'struct LinkRecord' + \
'{' + \
'    TopicLink* obj;' + \
'    int cost;' + \
'    Topic* theTopic;' + \
'};' + \
'dictionary<string, LinkRecord> LinkRecordDict;' + \
'};'

newSlice = \
'module Ice' + \
'{' + \
'struct Identity' + \
'{' + \
'    string name;' + \
'    string category;' + \
'};' + \
'};' + \
'module IceStorm' + \
'{' + \
'interface TopicLink;' + \
'interface Topic;' + \
'struct LinkRecord' + \
'{' + \
'    TopicLink* obj;' + \
'    int cost;' + \
'    Topic* theTopic;' + \
'};' + \
'sequence<LinkRecord> LinkRecordSeq;' + \
'};'

transformdb("topics", desc, oldSlice, newSlice)
