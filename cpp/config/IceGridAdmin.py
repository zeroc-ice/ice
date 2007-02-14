#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os, TestUtil
from threading import Thread

#
# Set nreplicas to a number N to test replication with N replicas.
#
#nreplicas=0
nreplicas=1

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

iceGridPort = 12010;

nodeOptions = r' --Ice.Warn.Connections=0' + \
              r' --IceGrid.Node.Endpoints=default' + \
              r' --IceGrid.Node.WaitTime=30' + \
              r' --Ice.ProgramName=icegridnode' + \
              r' --IceGrid.Node.Trace.Replica=0' + \
              r' --IceGrid.Node.Trace.Activator=0' + \
              r' --IceGrid.Node.Trace.Adapter=0' + \
              r' --IceGrid.Node.Trace.Server=0' + \
              r' --IceGrid.Node.ThreadPool.SizeWarn=0' + \
              r' --IceGrid.Node.PrintServersReady=node' + \
              r' --Ice.NullHandleAbort' + \
              r' --Ice.ThreadPool.Server.Size=0' + \
              r' --Ice.ServerIdleTime=0';

registryOptions = r' --Ice.Warn.Connections=0' + \
                  r' --IceGrid.Registry.PermissionsVerifier=IceGrid/NullPermissionsVerifier' + \
                  r' --IceGrid.Registry.AdminPermissionsVerifier=IceGrid/NullPermissionsVerifier' + \
                  r' --IceGrid.Registry.SSLPermissionsVerifier=IceGrid/NullSSLPermissionsVerifier' + \
                  r' --IceGrid.Registry.AdminSSLPermissionsVerifier=IceGrid/NullSSLPermissionsVerifier' + \
                  r' --IceGrid.Registry.Server.Endpoints=default' + \
                  r' --IceGrid.Registry.Internal.Endpoints=default' + \
                  r' --IceGrid.Registry.SessionManager.Endpoints=default' + \
                  r' --IceGrid.Registry.Trace.Session=0' + \
                  r' --IceGrid.Registry.Trace.Application=0' + \
                  r' --IceGrid.Registry.Trace.Node=0' + \
                  r' --IceGrid.Registry.Trace.Replica=0' + \
                  r' --IceGrid.Registry.Trace.Adapter=0' + \
                  r' --IceGrid.Registry.Trace.Object=0' + \
                  r' --IceGrid.Registry.Trace.Server=0' + \
                  r' --IceGrid.Registry.Trace.Locator=0' + \
                  r' --Ice.ThreadPool.Server.Size=0 ' + \
                  r' --Ice.ThreadPool.Client.SizeWarn=0' + \
                  r' --IceGrid.Registry.Client.ThreadPool.SizeWarn=0' + \
                  r' --Ice.ServerIdleTime=0' + \
                  r' --IceGrid.Registry.DefaultTemplates=' + os.path.join(toplevel, "config", "templates.xml")

def getDefaultLocatorProperty():

   i = 0
   property = '--Ice.Default.Locator=IceGrid/Locator';
   while i < nreplicas + 1:
       property = property + ':default -p ' + str(iceGridPort + i)
       i = i + 1

   return '"' + property + '"'

def startIceGridRegistry(testdir, dynamicRegistration = False):

    iceGrid = os.path.join(toplevel, "bin", "icegridregistry")

    command = iceGrid + TestUtil.clientServerOptions + ' --nowarn ' + registryOptions
    if dynamicRegistration:
        command += r' --IceGrid.Registry.DynamicRegistration'        

    i = 0
    while i < (nreplicas + 1):

        if i == 0:
            name = "registry"
        else:
            name = "replica-" + str(i)

        dataDir = os.path.join(testdir, "db", name)
        if not os.path.exists(dataDir):
            os.mkdir(dataDir)
        else:
            cleanDbDir(dataDir)

        print "starting icegrid " + name + "...",
        cmd = command + \
              r' --Ice.ProgramName=' + name + \
              r' --IceGrid.Registry.Client.Endpoints="default -p ' + str(iceGridPort + i) + ' -t 30000" ' + \
              r' --IceGrid.Registry.Data=' + dataDir

        if i > 0:
            cmd += r' --IceGrid.Registry.ReplicaName=' + name + ' ' + getDefaultLocatorProperty()

        if TestUtil.debug:
            print "(" + cmd + ")",

        pipe = os.popen(cmd + " 2>&1")
        TestUtil.getServerPid(pipe)
        TestUtil.getAdapterReady(pipe, True, 4)
        print "ok"

        i = i + 1

def shutdownIceGridRegistry():

    i = nreplicas
    while i > 0:
        print "shutting down icegrid replica-" + str(i) + "...",
        iceGridAdmin("registry shutdown replica-" + str(i))
        print "ok"
        i = i - 1

    print "shutting down icegrid registry...",
    iceGridAdmin("registry shutdown")
    print "ok"

def startIceGridNode(testdir):

    iceGrid = os.path.join(toplevel, "bin", "icegridnode")

    dataDir = os.path.join(testdir, "db", "node")
    if not os.path.exists(dataDir):
        os.mkdir(dataDir)
    else:
        cleanDbDir(dataDir)

    overrideOptions = '"' 
    for opt in TestUtil.clientServerOptions.split():
        opt = opt.replace("--", "")
        if opt.find("=") == -1:
            opt += "=1"
        overrideOptions += opt + " "
    overrideOptions += ' Ice.ServerIdleTime=0 Ice.PrintProcessId=0 Ice.PrintAdapterReady=0"'

    print "starting icegrid node...",
    command = iceGrid + TestUtil.clientServerOptions + ' --nowarn ' + nodeOptions + \
              r' ' + getDefaultLocatorProperty() + \
              r' --IceGrid.Node.Data=' + dataDir + \
              r' --IceGrid.Node.Name=localnode' + \
              r' --IceGrid.Node.PropertiesOverride=' + overrideOptions

    if TestUtil.debug:
        print "(" + command + ")",

    iceGridPipe = os.popen(command + " 2>&1")
    TestUtil.getServerPid(iceGridPipe)
    TestUtil.getAdapterReady(iceGridPipe, False)
    TestUtil.waitServiceReady(iceGridPipe, 'node')
        
    print "ok"

    return iceGridPipe

def iceGridAdmin(cmd, ignoreFailure = False):

    iceGridAdmin = os.path.join(toplevel, "bin", "icegridadmin")

    user = r"admin1"
    if cmd == "registry shutdown":
        user = r"shutdown"
    command = iceGridAdmin + TestUtil.clientOptions + ' ' + getDefaultLocatorProperty() + \
              r" --IceGridAdmin.Username=" + user + " --IceGridAdmin.Password=test1 " + \
              r' -e "' + cmd + '"'

    if TestUtil.debug:
        print "(" + command +")",

    iceGridAdminPipe = os.popen(command + " 2>&1")

    output = iceGridAdminPipe.readlines()
    iceGridAdminStatus = TestUtil.closePipe(iceGridAdminPipe)
    if not ignoreFailure and iceGridAdminStatus:
        for line in output:
            print line
        TestUtil.killServers()
        sys.exit(1)

    return output
    
def killNodeServers():
    
    for server in iceGridAdmin("server list"):
        server = server.strip()
        iceGridAdmin("server disable " + server, True)
        iceGridAdmin("server signal " + server + " SIGKILL", True)

def iceGridTest(name, application, additionalOptions = "", applicationOptions = ""):

    if not TestUtil.isWin32() and os.getuid() == 0:
        print
        print "*** can't run test as root ***"
        print
        return

    testdir = os.path.join(toplevel, "test", name)
    client = os.path.join(testdir, "client")

    clientOptions = ' ' + getDefaultLocatorProperty() + ' ' + additionalOptions

    startIceGridRegistry(testdir)
    iceGridNodePipe = startIceGridNode(testdir)
    
    if application != "":
        print "adding application...",
        iceGridAdmin('application add ' + os.path.join(testdir, application) + ' ' + \
                     '"test.dir=' + testdir + '" "ice.dir=' + toplevel + '" ' + applicationOptions)
        print "ok"

    print "starting client...",
    command = client + TestUtil.clientOptions + " " + clientOptions

    if TestUtil.debug:
        print "(" + command +")",

    clientPipe = os.popen(command + " 2>&1")
    print "ok"

    TestUtil.printOutputFromPipe(clientPipe)
    
    clientStatus = TestUtil.closePipe(clientPipe)
    if clientStatus:
        killNodeServers()
        if application != "":
            print "remove application...",
            iceGridAdmin("application remove Test", True)
            print "ok"
        TestUtil.killServers()
        sys.exit(1)

    if application != "":
        print "remove application...",
        iceGridAdmin("application remove Test")
        print "ok"

    print "shutting down icegrid node...",
    iceGridAdmin("node shutdown localnode")
    print "ok"
    shutdownIceGridRegistry()

    TestUtil.joinServers()

    if TestUtil.serverStatus():
        sys.exit(1)                

def iceGridClientServerTest(name, additionalClientOptions, additionalServerOptions):

    testdir = os.path.join(toplevel, "test", name)
    server = os.path.join(testdir, "server")
    client = os.path.join(testdir, "client")

    clientOptions = getDefaultLocatorProperty() + ' ' + additionalClientOptions
    serverOptions = getDefaultLocatorProperty() + ' ' + additionalServerOptions
    
    startIceGridRegistry(testdir, True)

    print "starting sever...",

    command = server + TestUtil.clientServerOptions + " " + serverOptions

    if TestUtil.debug:
        print "(" + command +")",

    serverPipe = os.popen(command + " 2>&1")
    TestUtil.getServerPid(serverPipe)
    TestUtil.getAdapterReady(serverPipe)
    print "ok"

    print "starting client...",
    command = client + TestUtil.clientOptions + " " + clientOptions

    if TestUtil.debug:
        print "(" + command +")",

    clientPipe = os.popen(command + " 2>&1")
    print "ok"

    TestUtil.printOutputFromPipe(clientPipe)
    
    clientStatus = TestUtil.closePipe(clientPipe)
    if clientStatus:
        TestUtil.killServers()
        sys.exit(1)

    shutdownIceGridRegistry()

    TestUtil.joinServers()

    if TestUtil.serverStatus():
        sys.exit(1)

def cleanDbDir(path):
    
    files = os.listdir(path)
    for filename in files:
        if filename != "CVS" and filename != ".dummy":
            fullpath = os.path.join(path, filename);
            if os.path.isdir(fullpath):
                cleanDbDir(fullpath)
                try:
                    os.rmdir(fullpath)
                except OSError:
                    # This might fail if the directory is empty (because it itself is
                    # a CVS directory).
                    pass
            else:
                os.remove(fullpath)
