#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os, TestUtil
from threading import Thread

if not os.environ.has_key('ICE_HOME'):
    print "ICE_HOME is not defined."
    sys.exit(0)

ice_home = os.environ['ICE_HOME']

icePackPort = "0";

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

def startIcePackRegistry(port, testdir):

    global icePackPort

    icePackPort = port
    
    icePack = os.path.join(ice_home, "bin", "icepackregistry")

    dataDir = os.path.join(testdir, "db", "registry")
    if not os.path.exists(dataDir):
        os.mkdir(dataDir)

    print "starting icepack registry...",
    command = icePack + TestUtil.cppClientServerOptions + ' --nowarn ' + \
              r' --IcePack.Registry.Client.Endpoints="default -p ' + icePackPort + ' -t 5000" ' + \
              r' --IcePack.Registry.Server.Endpoints=default' + \
              r' --IcePack.Registry.Internal.Endpoints=default' + \
              r' --IcePack.Registry.Admin.Endpoints=default' + \
              r' --IcePack.Registry.Data=' + dataDir + \
              r' --IcePack.Registry.DynamicRegistration' + \
              r' --Ice.ProgramName=icepackregistry'

    (stdin, icePackPipe) = os.popen4(command)
    TestUtil.getAdapterReady(icePackPipe)
    TestUtil.getAdapterReady(icePackPipe)
    TestUtil.getAdapterReady(icePackPipe)
    TestUtil.getAdapterReady(icePackPipe)
    print "ok"

    readerThread = ReaderThread(icePackPipe, "IcePackRegistry")
    readerThread.start()
    return readerThread

def startIcePackNode(testdir):

    icePack = os.path.join(ice_home, "bin", "icepacknode")

    dataDir = os.path.join(testdir, "db", "node")
    if not os.path.exists(dataDir):
        os.mkdir(dataDir)

    overrideOptions = '"' + TestUtil.clientServerOptions.replace("--", "") + \
	              ' Ice.PrintProcessId=0 Ice.PrintAdapterReady=0' + '"'

    print "starting icepack node...",
    command = icePack + TestUtil.cppClientServerOptions + ' --nowarn ' + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' --IcePack.Node.Endpoints=default' + \
              r' --IcePack.Node.Data=' + dataDir + \
              r' --IcePack.Node.Name=localnode' + \
              r' --IcePack.Node.PropertiesOverride=' + overrideOptions + \
              r' --Ice.ProgramName=icepacknode' + \
              r' --IcePack.Node.Trace.Activator=0' + \
              r' --IcePack.Node.Trace.Adapter=0' + \
              r' --IcePack.Node.Trace.Server=0' + \
              r' --IcePack.Node.PrintServersReady=node'
    
    (stdin, icePackPipe) = os.popen4(command)
    TestUtil.getAdapterReady(icePackPipe)
    TestUtil.waitServiceReady(icePackPipe, 'node')
    print "ok"

    readerThread = ReaderThread(icePackPipe, "IcePackNode")
    readerThread.start()
    return readerThread

def shutdownIcePackRegistry():

    global icePackPort
    icePackAdmin = os.path.join(ice_home, "bin", "icepackadmin")

    print "shutting down icepack registry...",
    command = icePackAdmin + TestUtil.cppClientOptions + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' -e "shutdown" ' + " 2>&1"

    icePackAdminPipe = os.popen(command)
    TestUtil.printOutputFromPipe(icePackAdminPipe)
    icePackAdminStatus = icePackAdminPipe.close()
    if icePackAdminStatus:
        TestUtil.killServers()
        sys.exit(1)
    print "ok"
        
def shutdownIcePackNode():

    global icePackPort
    icePackAdmin = os.path.join(ice_home, "bin", "icepackadmin")

    print "shutting down icepack node...",
    command = icePackAdmin + TestUtil.cppClientOptions + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' -e "node shutdown localnode" ' + " 2>&1"

    icePackAdminPipe = os.popen(command)
    TestUtil.printOutputFromPipe(icePackAdminPipe)
    icePackAdminStatus = icePackAdminPipe.close()
    if icePackAdminStatus:
        TestUtil.killServers()
        sys.exit(1)
    print "ok"
        
def addApplication(descriptor, options):

    global icePackPort
    icePackAdmin = os.path.join(ice_home, "bin", "icepackadmin")

    descriptor = descriptor.replace("\\", "/")
    command = icePackAdmin + TestUtil.cppClientOptions + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' -e "application add \"' + descriptor + '\\" ' + options + ' \"' + " 2>&1"

    icePackAdminPipe = os.popen(command)
    TestUtil.printOutputFromPipe(icePackAdminPipe)
    icePackAdminStatus = icePackAdminPipe.close()
    if icePackAdminStatus:
        TestUtil.killServers()
        sys.exit(1)

def removeApplication(descriptor):

    global icePackPort
    icePackAdmin = os.path.join(ice_home, "bin", "icepackadmin")

    descriptor = descriptor.replace("\\", "/")
    command = icePackAdmin + TestUtil.cppClientOptions + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' -e "application remove \"' + descriptor + '\\" \"' + " 2>&1"

    icePackAdminPipe = os.popen(command)
    TestUtil.printOutputFromPipe(icePackAdminPipe)
    icePackAdminStatus = icePackAdminPipe.close()
    if icePackAdminStatus:
        TestUtil.killServers()
        sys.exit(1)

def addServer(serverDescriptor, options):

    global icePackPort
    icePackAdmin = os.path.join(ice_home, "bin", "icepackadmin")

    serverDescriptor = serverDescriptor.replace("\\", "/");
    command = icePackAdmin + TestUtil.cppClientOptions + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' -e "server add ' + serverDescriptor + ' localnode ' + \
              r' ' + options + '\"' + " 2>&1"

    icePackAdminPipe = os.popen(command)
    TestUtil.printOutputFromPipe(icePackAdminPipe)
    icePackAdminStatus = icePackAdminPipe.close()
    if icePackAdminStatus:
        TestUtil.killServers()
        sys.exit(1)

def removeServer(name):

    global icePackPort
    icePackAdmin = os.path.join(ice_home, "bin", "icepackadmin")

    command = icePackAdmin + TestUtil.cppClientOptions + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' -e "server remove \"' + name + '\\" \"' + " 2>&1"

    icePackAdminPipe = os.popen(command)
    TestUtil.printOutputFromPipe(icePackAdminPipe)
    icePackAdminStatus = icePackAdminPipe.close()
    if icePackAdminStatus:
        TestUtil.killServers()
        sys.exit(1)
  
def startServer(name):

    global icePackPort
    icePackAdmin = os.path.join(ice_home, "bin", "icepackadmin")

    command = icePackAdmin + TestUtil.cppClientOptions + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' -e "server start \"' + name + '\\""' + " 2>&1"

    icePackAdminPipe = os.popen(command)
    TestUtil.printOutputFromPipe(icePackAdminPipe)
    icePackAdminStatus = icePackAdminPipe.close()
    if icePackAdminStatus:
        TestUtil.killServers()
        sys.exit(1)

def listAdapters():

    global icePackPort
    icePackAdmin = os.path.join(ice_home, "bin", "icepackadmin")

    command = icePackAdmin + TestUtil.cppClientOptions + \
              r' "--Ice.Default.Locator=IcePack/Locator:default -p ' + icePackPort + '" ' + \
              r' -e "adapter list"' + " 2>&1"

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
            
