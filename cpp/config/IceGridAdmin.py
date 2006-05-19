#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os, TestUtil
import time
from threading import Thread

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

iceGridPort = "0";

nodeOptions = r' --Ice.Warn.Connections=0' + \
              r' --IceGrid.Node.Endpoints=default' + \
              r' --IceGrid.Node.WaitTime=30' + \
              r' --Ice.ProgramName=icegridnode' + \
              r' --IceGrid.Node.Trace.Activator=0' + \
              r' --IceGrid.Node.Trace.Adapter=0' + \
              r' --IceGrid.Node.Trace.Server=0' + \
              r' --IceGrid.Node.PrintServersReady=node' + \
	      r' --Ice.NullHandleAbort' + \
              r' --Ice.ThreadPool.Server.Size=0';

registryOptions = r' --Ice.Warn.Connections=0' + \
                  r' --IceGrid.Registry.PermissionsVerifier=IceGrid/NullPermissionsVerifier' + \
                  r' --IceGrid.Registry.AdminPermissionsVerifier=IceGrid/NullPermissionsVerifier' + \
                  r' --IceGrid.Registry.Server.Endpoints=default' + \
                  r' --IceGrid.Registry.Internal.Endpoints=default' + \
                  r' --IceGrid.Registry.Admin.Endpoints=default' + \
                  r' --IceGrid.Registry.Server.Endpoints=default' + \
                  r' --IceGrid.Registry.Internal.Endpoints=default' + \
                  r' --IceGrid.Registry.Admin.Endpoints=default' + \
                  r' --IceGrid.Registry.Trace.Session=0' + \
                  r' --IceGrid.Registry.Trace.Application=0' + \
                  r' --IceGrid.Registry.Trace.Adapter=0' + \
                  r' --IceGrid.Registry.Trace.Object=0' + \
                  r' --IceGrid.Registry.Trace.Server=0' + \
                  r' --Ice.ThreadPool.Server.Size=0';

class ReaderThread(Thread):
    def __init__(self, pipe, token):
        self.pipe = pipe
        self.token = token
        Thread.__init__(self)

    def run(self):

        try:
            while 1:
                line = self.pipe.readline()
                if not line: break
                print self.token + ": " + line,
        except IOError:
            pass

        try:
            self.pipe.close()
        except IOError:
            pass

def startIceGridRegistry(port, testdir, dynamicRegistration):

    global iceGridPort

    iceGridPort = port
    
    iceGrid = os.path.join(toplevel, "bin", "icegridregistry")

    dataDir = os.path.join(testdir, "db", "registry")
    if not os.path.exists(dataDir):
        os.mkdir(dataDir)

    print "starting icegrid registry...",
    command = iceGrid + TestUtil.clientServerOptions + ' --nowarn ' + registryOptions + \
              r' --IceGrid.Registry.Client.Endpoints="default -p ' + iceGridPort + ' -t 30000" ' + \
              r' --IceGrid.Registry.Data=' + dataDir + \
              r' --IceGrid.Registry.DefaultTemplates=' + os.path.join(toplevel, "config", "templates.xml")

    if dynamicRegistration:
        command += ' --IceGrid.Registry.DynamicRegistration'        

    (stdin, iceGridPipe) = os.popen4(command)
    TestUtil.getServerPid(iceGridPipe)
    TestUtil.getAdapterReady(iceGridPipe)
    TestUtil.getAdapterReady(iceGridPipe)
    TestUtil.getAdapterReady(iceGridPipe)
    TestUtil.getAdapterReady(iceGridPipe)
    print "ok"

    readerThread = ReaderThread(iceGridPipe, "IceGridRegistry")
    readerThread.start()
    return readerThread

def startIceGridNode(testdir):

    iceGrid = os.path.join(toplevel, "bin", "icegridnode")

    dataDir = os.path.join(testdir, "db", "node")
    if not os.path.exists(dataDir):
        os.mkdir(dataDir)

    overrideOptions = '"' + TestUtil.clientServerOptions.replace("--", "") + \
	              ' Ice.ServerIdleTime=0 Ice.PrintProcessId=0 Ice.PrintAdapterReady=0 ' + \
                      ' Ice.ThreadPool.Server.Size=0"'

    print "starting icegrid node...",
    command = iceGrid + TestUtil.clientServerOptions + ' --nowarn ' + nodeOptions + \
              r' "--Ice.Default.Locator=IceGrid/Locator:default -p ' + iceGridPort + '" ' + \
              r' --IceGrid.Node.Data=' + dataDir + \
              r' --IceGrid.Node.Name=localnode' + \
              r' --IceGrid.Node.PropertiesOverride=' + overrideOptions

    (stdin, iceGridPipe) = os.popen4(command)
    TestUtil.getServerPid(iceGridPipe)
    TestUtil.getAdapterReady(iceGridPipe)
    TestUtil.waitServiceReady(iceGridPipe, 'node')
        
    print "ok"

    readerThread = ReaderThread(iceGridPipe, "IceGridNode")
    readerThread.start()
    return readerThread

def shutdownIceGridRegistry():

    global iceGridPort
    iceGridAdmin = os.path.join(toplevel, "bin", "icegridadmin")

    print "shutting down icegrid registry...",
    command = iceGridAdmin + TestUtil.clientOptions + \
              r' "--Ice.Default.Locator=IceGrid/Locator:default -p ' + iceGridPort + '" ' + \
              r' -e "shutdown" ' + " 2>&1"

    iceGridAdminPipe = os.popen(command)
    TestUtil.printOutputFromPipe(iceGridAdminPipe)
    iceGridAdminStatus = iceGridAdminPipe.close()
    if iceGridAdminStatus:
        TestUtil.killServers()
        sys.exit(1)
    print "ok"
        
def shutdownIceGridNode():

    global iceGridPort
    iceGridAdmin = os.path.join(toplevel, "bin", "icegridadmin")

    print "shutting down icegrid node...",
    command = iceGridAdmin + TestUtil.clientOptions + \
              r' "--Ice.Default.Locator=IceGrid/Locator:default -p ' + iceGridPort + '" ' + \
              r' -e "node shutdown localnode" ' + " 2>&1"

    iceGridAdminPipe = os.popen(command)
    TestUtil.printOutputFromPipe(iceGridAdminPipe)
    iceGridAdminStatus = iceGridAdminPipe.close()
    if iceGridAdminStatus:
        TestUtil.killServers()
        sys.exit(1)
    print "ok"
        
def addApplication(descriptor, options):

    global iceGridPort
    iceGridAdmin = os.path.join(toplevel, "bin", "icegridadmin")

    descriptor = descriptor.replace("\\", "/")
    command = iceGridAdmin + TestUtil.clientOptions + \
              r' "--Ice.Default.Locator=IceGrid/Locator:default -p ' + iceGridPort + '" ' + \
              r' -e "application add \"' + descriptor + '\\" ' + options + ' \"' + " 2>&1"

    iceGridAdminPipe = os.popen(command)
    TestUtil.printOutputFromPipe(iceGridAdminPipe)
    iceGridAdminStatus = iceGridAdminPipe.close()
    if iceGridAdminStatus:
        TestUtil.killServers()
        sys.exit(1)

def removeApplication(name):

    global iceGridPort
    iceGridAdmin = os.path.join(toplevel, "bin", "icegridadmin")

    command = iceGridAdmin + TestUtil.clientOptions + \
              r' "--Ice.Default.Locator=IceGrid/Locator:default -p ' + iceGridPort + '" ' + \
              r' -e "application remove \"' + name + '\\" \"' + " 2>&1"

    iceGridAdminPipe = os.popen(command)
    TestUtil.printOutputFromPipe(iceGridAdminPipe)
    iceGridAdminStatus = iceGridAdminPipe.close()
    if iceGridAdminStatus:
        TestUtil.killServers()
        sys.exit(1)

def startServer(name):

    global iceGridPort
    iceGridAdmin = os.path.join(toplevel, "bin", "icegridadmin")

    command = iceGridAdmin + TestUtil.clientOptions + \
              r' "--Ice.Default.Locator=IceGrid/Locator:default -p ' + iceGridPort + '" ' + \
              r' -e "server start \"' + name + '\\""' + " 2>&1"

    iceGridAdminPipe = os.popen(command)
    TestUtil.printOutputFromPipe(iceGridAdminPipe)
    iceGridAdminStatus = iceGridAdminPipe.close()
    if iceGridAdminStatus:
        TestUtil.killServers()
        sys.exit(1)

def stopServer(name):

    global iceGridPort
    iceGridAdmin = os.path.join(toplevel, "bin", "icegridadmin")

    command = iceGridAdmin + TestUtil.clientOptions + \
              r' "--Ice.Default.Locator=IceGrid/Locator:default -p ' + iceGridPort + '" ' + \
              r' -e "server stop \"' + name + '\\""' + " 2>&1"

    iceGridAdminPipe = os.popen(command)
    TestUtil.printOutputFromPipe(iceGridAdminPipe)
    iceGridAdminStatus = iceGridAdminPipe.close()
    if iceGridAdminStatus:
        TestUtil.killServers()
        sys.exit(1)

def listAdapters():

    global iceGridPort
    iceGridAdmin = os.path.join(toplevel, "bin", "icegridadmin")

    command = iceGridAdmin + TestUtil.clientOptions + \
              r' "--Ice.Default.Locator=IceGrid/Locator:default -p ' + iceGridPort + '" ' + \
              r' -e "adapter list"' + " 2>&1"

    iceGridAdminPipe = os.popen(command)
    return iceGridAdminPipe

def cleanDbDir(path):
    
    files = os.listdir(path)
    for filename in files:
        if filename != "CVS" and filename != ".dummy":
            fullpath = os.path.join(path, filename);
            if os.path.isdir(fullpath):
                cleanDir(fullpath)
                os.rmdir(fullpath)
            else:
                os.remove(fullpath)

def cleanDir(path):

    files = os.listdir(path)

    for filename in files:
        fullpath = os.path.join(path, filename);
        if os.path.isdir(fullpath):
            cleanDir(fullpath)
            os.rmdir(fullpath)
        else:
            os.remove(fullpath)
            
