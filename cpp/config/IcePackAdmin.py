#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003
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

import sys, os, TestUtil

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

icePackPort = "0";

def startIcePackRegistry(port, testdir):

    global icePackPort

    icePackPort = port
    
    icePack = os.path.join(toplevel, "bin", "icepackregistry")

    dataDir = os.path.join(testdir, "db", "registry")
    if not os.path.exists(dataDir):
        os.mkdir(dataDir)

    print "starting icepack registry...",
    command = icePack + TestUtil.clientServerOptions + ' --nowarn ' + \
              r' --IcePack.Registry.Client.Endpoints="default -p ' + icePackPort + '  -t 5000" ' + \
              r' --IcePack.Registry.Server.Endpoints=default' + \
              r' --IcePack.Registry.Internal.Endpoints=default' + \
              r' --IcePack.Registry.Admin.Endpoints=default' + \
              r' --IcePack.Registry.Data=' + dataDir + \
              r' --IcePack.Registry.DynamicRegistration' + \
              r' --Ice.ProgramName=icepackregistry'

    icePackPipe = os.popen(command)
    TestUtil.getServerPid(icePackPipe)
    TestUtil.getAdapterReady(icePackPipe)
    TestUtil.getAdapterReady(icePackPipe)
    TestUtil.getAdapterReady(icePackPipe)
    TestUtil.getAdapterReady(icePackPipe)
    print "ok"

    return icePackPipe

def startIcePackNode(testdir):

    icePack = os.path.join(toplevel, "bin", "icepacknode")

    dataDir = os.path.join(testdir, "db", "node")
    if not os.path.exists(dataDir):
        os.mkdir(dataDir)

    overrideOptions = '"' + TestUtil.clientServerOptions.replace("--", "") + '"'

    print "starting icepack node...",
    command = icePack + TestUtil.clientServerOptions + ' --nowarn ' + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' --IcePack.Node.Endpoints=default' + \
              r' --IcePack.Node.Data=' + dataDir + \
              r' --IcePack.Node.Name=localnode' + \
              r' --IcePack.Node.PropertiesOverride=' + overrideOptions + \
              r' --Ice.ProgramName=icepacknode' + \
              r' --IcePack.Node.Trace.Activator=0' + \
              r' --IcePack.Node.Trace.Adapter=0' + \
              r' --IcePack.Node.Trace.Server=0'
    
    icePackPipe = os.popen(command)
    TestUtil.getServerPid(icePackPipe)
    TestUtil.getAdapterReady(icePackPipe)
    print "ok"
    return icePackPipe

def shutdownIcePackRegistry(icePackPipe):

    global icePackPort
    icePackAdmin = os.path.join(toplevel, "bin", "icepackadmin")

    print "shutting down icepack registry...",
    command = icePackAdmin + TestUtil.clientOptions + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' -e "shutdown" '

    icePackAdminPipe = os.popen(command)
    icePackAdminStatus = icePackAdminPipe.close()
    icePackPipe.close()
    print "ok"

    if icePackAdminStatus:
        TestUtil.killServers()
        sys.exit(1)
        
def shutdownIcePackNode(icePackPipe):

    global icePackPort
    icePackAdmin = os.path.join(toplevel, "bin", "icepackadmin")

    print "shutting down icepack node...",
    command = icePackAdmin + TestUtil.clientOptions + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' -e "node shutdown localnode" '

    icePackAdminPipe = os.popen(command)
    icePackAdminStatus = icePackAdminPipe.close()
    icePackPipe.close()
    print "ok"

    if icePackAdminStatus:
        TestUtil.killServers()
        sys.exit(1)
        
def addApplication(descriptor, targets):

    global icePackPort
    icePackAdmin = os.path.join(toplevel, "bin", "icepackadmin")

    command = icePackAdmin + TestUtil.clientOptions + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' -e "application add \"' + descriptor + '\\" ' + targets + ' \"'

    icePackAdminPipe = os.popen(command)
    icePackAdminStatus = icePackAdminPipe.close()
    if icePackAdminStatus:
        TestUtil.killServers()
        sys.exit(1)

def removeApplication(descriptor):

    global icePackPort
    icePackAdmin = os.path.join(toplevel, "bin", "icepackadmin")

    command = icePackAdmin + TestUtil.clientOptions + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' -e "application remove \"' + descriptor + '\\" \"'

    icePackAdminPipe = os.popen(command)
    icePackAdminStatus = icePackAdminPipe.close()
    if icePackAdminStatus:
        TestUtil.killServers()
        sys.exit(1)

def addServer(name, serverDescriptor, server, libpath, targets):

    global icePackPort
    icePackAdmin = os.path.join(toplevel, "bin", "icepackadmin")

    command = icePackAdmin + TestUtil.clientOptions + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' -e "server add localnode \"' + name + '\\" \\"' + serverDescriptor + '\\" ' + \
              r' \"' + server + '\\" \\"' + libpath + '\\" ' + targets + '\"'

    icePackAdminPipe = os.popen(command)
    icePackAdminStatus = icePackAdminPipe.close()
    if icePackAdminStatus:
        TestUtil.killServers()
        sys.exit(1)

def removeServer(name):

    global icePackPort
    icePackAdmin = os.path.join(toplevel, "bin", "icepackadmin")

    command = icePackAdmin + TestUtil.clientOptions + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' -e "server remove \"' + name + '\\" \"'

    icePackAdminPipe = os.popen(command)
    icePackAdminStatus = icePackAdminPipe.close()
    if icePackAdminStatus:
        TestUtil.killServers()
        sys.exit(1)

def startServer(name):
    global icePackPort
    icePackAdmin = os.path.join(toplevel, "bin", "icepackadmin")

    command = icePackAdmin + TestUtil.clientOptions + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' -e "server start \"' + name + '\\""'

    icePackAdminPipe = os.popen(command)
    icePackAdminStatus = icePackAdminPipe.close()
    if icePackAdminStatus:
        TestUtil.killServers()
        sys.exit(1)

def listAdapters():
    global icePackPort
    icePackAdmin = os.path.join(toplevel, "bin", "icepackadmin")

    command = icePackAdmin + TestUtil.clientOptions + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' -e "adapter list"'

    icePackAdminPipe = os.popen(command)
    return icePackAdminPipe

def cleanDbDir(path):
    
    try:
        cleanServerDir(os.path.join(path, "node", "servers"))
    except:
        pass

    try:
        TestUtil.cleanDbDir(os.path.join(path, "node", "db"))
    except:
        pass

    try:
        TestUtil.cleanDbDir(os.path.join(path, "registry"))
    except:
        pass

def cleanServerDir(path):

    files = os.listdir(path)

    for filename in files:
        fullpath = os.path.join(path, filename);
        if os.path.isdir(fullpath):
            cleanServerDir(fullpath)
            os.rmdir(fullpath)
        else:
            os.remove(fullpath)
            
