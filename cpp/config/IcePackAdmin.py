#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2001
# ZeroC, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

import sys, os, TestUtil

icePackPort = "0";

def startIcePackRegistry(toplevel, port, testdir):

    global icePackPort

    options = TestUtil.clientServerOptions.replace("TOPLEVELDIR", toplevel)

    icePackPort = port
    
    icePack = os.path.join(toplevel, "bin", "icepackregistry")

    dataDir = os.path.join(testdir, "db/registry")
    if not os.path.exists(dataDir):
        os.mkdir(dataDir)

    print "starting icepack registry...",
    command = icePack + options + ' --nowarn ' + \
          r' --IcePack.Registry.Locator.Endpoints="default -p ' + icePackPort + '  -t 5000" ' + \
          r' --IcePack.Registry.LocatorRegistry.Endpoints=default' + \
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

def startIcePackNode(toplevel, testdir):

    options = TestUtil.clientServerOptions.replace("TOPLEVELDIR", toplevel)
    
    icePack = os.path.join(toplevel, "bin", "icepacknode")

    dataDir = os.path.join(testdir, "db/node")
    if not os.path.exists(dataDir):
        os.mkdir(dataDir)

    print "starting icepack node...",
    command = icePack + options + ' --nowarn ' + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' --IcePack.Node.Endpoints=default' + \
              r' --IcePack.Node.Data=' + dataDir + \
              r' --IcePack.Node.Name=localnode' + \
              r' --Ice.ProgramName=icepacknode' + \
              r' --IcePack.Node.Trace.Activator=0' + \
              r' --IcePack.Node.Trace.Adapter=0' + \
              r' --IcePack.Node.Trace.Server=0'

    icePackPipe = os.popen(command)
    TestUtil.getServerPid(icePackPipe)
    TestUtil.getAdapterReady(icePackPipe)
    print "ok"
    return icePackPipe

def shutdownIcePackRegistry(toplevel, icePackPipe):

    global icePackPort
    icePackAdmin = os.path.join(toplevel, "bin", "icepackadmin")

    options = TestUtil.clientOptions.replace("TOPLEVELDIR", toplevel)

    print "shutting down icepack registry...",
    command = icePackAdmin + options + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' -e "shutdown" '

    icePackAdminPipe = os.popen(command)
    icePackAdminStatus = icePackAdminPipe.close()
    icePackPipe.close()
    print "ok"

    if icePackAdminStatus:
        TestUtil.killServers()
        sys.exit(1)
        
def shutdownIcePackNode(toplevel, icePackPipe):

    global icePackPort
    icePackAdmin = os.path.join(toplevel, "bin", "icepackadmin")

    options = TestUtil.clientOptions.replace("TOPLEVELDIR", toplevel)

    print "shutting down icepack node...",
    command = icePackAdmin + options + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' -e "node shutdown localnode" '

    icePackAdminPipe = os.popen(command)
    icePackAdminStatus = icePackAdminPipe.close()
    icePackPipe.close()
    print "ok"

    if icePackAdminStatus:
        TestUtil.killServers()
        sys.exit(1)
        
def addApplication(toplevel, descriptor, targets):

    global icePackPort
    icePackAdmin = os.path.join(toplevel, "bin", "icepackadmin")

    options = TestUtil.clientOptions.replace("TOPLEVELDIR", toplevel)
    
    command = icePackAdmin + options + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' -e "application add \"' + descriptor + '\\" ' + targets + ' \"'

    icePackAdminPipe = os.popen(command)
    icePackAdminStatus = icePackAdminPipe.close()
    if icePackAdminStatus:
        TestUtil.killServers()
        sys.exit(1)

def removeApplication(toplevel, descriptor):

    global icePackPort
    icePackAdmin = os.path.join(toplevel, "bin", "icepackadmin")

    options = TestUtil.clientOptions.replace("TOPLEVELDIR", toplevel)
    
    command = icePackAdmin + options + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' -e "application remove \"' + descriptor + '\\" \"'

    icePackAdminPipe = os.popen(command)
    icePackAdminStatus = icePackAdminPipe.close()
    if icePackAdminStatus:
        TestUtil.killServers()
        sys.exit(1)

def addServer(toplevel, name, serverDescriptor, server, libpath, targets):

    global icePackPort
    icePackAdmin = os.path.join(toplevel, "bin", "icepackadmin")

    options = TestUtil.clientOptions.replace("TOPLEVELDIR", toplevel)
    
    command = icePackAdmin + options + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' -e "server add localnode \"' + name + '\\" \\"' + serverDescriptor + '\\" ' + \
              r' \"' + server + '\\" \\"' + libpath + '\\" ' + targets + '\"'

    icePackAdminPipe = os.popen(command)
    icePackAdminStatus = icePackAdminPipe.close()
    if icePackAdminStatus:
        TestUtil.killServers()
        sys.exit(1)

def removeServer(toplevel, name):

    global icePackPort
    icePackAdmin = os.path.join(toplevel, "bin", "icepackadmin")

    options = TestUtil.clientOptions.replace("TOPLEVELDIR", toplevel)
    
    command = icePackAdmin + options + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' -e "server remove \"' + name + '\\" \"'

    icePackAdminPipe = os.popen(command)
    icePackAdminStatus = icePackAdminPipe.close()
    if icePackAdminStatus:
        TestUtil.killServers()
        sys.exit(1)

def startServer(toplevel, name):
    global icePackPort
    icePackAdmin = os.path.join(toplevel, "bin", "icepackadmin")

    options = TestUtil.clientOptions.replace("TOPLEVELDIR", toplevel)

    command = icePackAdmin + options + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' -e "server start \"' + name + '\\""'

    icePackAdminPipe = os.popen(command)
    icePackAdminStatus = icePackAdminPipe.close()
    if icePackAdminStatus:
        TestUtil.killServers()
        sys.exit(1)

def listAdapters(toplevel):
    global icePackPort
    icePackAdmin = os.path.join(toplevel, "bin", "icepackadmin")

    options = TestUtil.clientOptions.replace("TOPLEVELDIR", toplevel)

    command = icePackAdmin + options + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' -e "adapter list"'

    icePackAdminPipe = os.popen(command)
    return icePackAdminPipe

