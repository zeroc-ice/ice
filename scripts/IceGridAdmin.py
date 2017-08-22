#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os, TestUtil, shlex
from threading import Thread

#
# Set nreplicas to a number N to test replication with N replicas.
#
nreplicas=0
#nreplicas=1

iceGridPort = 12010;

nodeOptions = r' --Ice.Warn.Connections=0' + \
              r' --IceGrid.InstanceName=TestIceGrid' + \
              r' --IceGrid.Node.Endpoints=default' + \
              r' --IceGrid.Node.WaitTime=240' + \
              r' --Ice.ProgramName=icegridnode' + \
              r' --IceGrid.Node.Trace.Replica=0' + \
              r' --IceGrid.Node.Trace.Activator=0' + \
              r' --IceGrid.Node.Trace.Adapter=0' + \
              r' --IceGrid.Node.Trace.Server=0' + \
              r' --IceGrid.Node.ThreadPool.SizeWarn=0' + \
              r' --IceGrid.Node.PrintServersReady=node1' + \
              r' --Ice.NullHandleAbort' + \
              r' --Ice.ThreadPool.Server.Size=1' + \
              r' --Ice.ServerIdleTime=0'

registryOptions = r' --Ice.Warn.Connections=0' + \
                  r' --IceGrid.InstanceName=TestIceGrid' + \
                  r' --IceGrid.Registry.PermissionsVerifier=TestIceGrid/NullPermissionsVerifier' + \
                  r' --IceGrid.Registry.AdminPermissionsVerifier=TestIceGrid/NullPermissionsVerifier' + \
                  r' --IceGrid.Registry.SSLPermissionsVerifier=TestIceGrid/NullSSLPermissionsVerifier' + \
                  r' --IceGrid.Registry.AdminSSLPermissionsVerifier=TestIceGrid/NullSSLPermissionsVerifier' + \
                  r' --IceGrid.Registry.Server.Endpoints=default' + \
                  r' --IceGrid.Registry.Internal.Endpoints=default' + \
                  r' --IceGrid.Registry.SessionManager.Endpoints=default' + \
                  r' --IceGrid.Registry.AdminSessionManager.Endpoints=default' + \
                  r' --IceGrid.Registry.Trace.Session=0' + \
                  r' --IceGrid.Registry.Trace.Application=0' + \
                  r' --IceGrid.Registry.Trace.Node=0' + \
                  r' --IceGrid.Registry.Trace.Replica=0' + \
                  r' --IceGrid.Registry.Trace.Adapter=0' + \
                  r' --IceGrid.Registry.Trace.Object=0' + \
                  r' --IceGrid.Registry.Trace.Server=0' + \
                  r' --IceGrid.Registry.Trace.Locator=0' + \
                  r' --IceGrid.Registry.SessionTimeout=60' + \
                  r' --Ice.ThreadPool.Server.Size=1 ' + \
                  r' --Ice.ThreadPool.Client.SizeWarn=0' + \
                  r' --IceGrid.Registry.Client.ThreadPool.SizeWarn=0' + \
                  r' --Ice.ServerIdleTime=0' + \
                  r' --IceGrid.Registry.DefaultTemplates="' + \
                  os.path.abspath(os.path.join(TestUtil.toplevel, "cpp", "config", "templates.xml") + '"')

if not TestUtil.isLinux():
    registryOptions += r' --IceGrid.Registry.Discovery.Interface="{}"'.format("::1" if TestUtil.ipv6 else "127.0.0.1")

def getDefaultLocatorProperty():

   i = 0
   property = '--Ice.Default.Locator="TestIceGrid/Locator';
   objrefs = ""
   while i < nreplicas + 1:
       objrefs = objrefs + ':default -p ' + str(iceGridPort + i)
       i = i + 1

   return ' %s%s"' % (property, objrefs)

def startIceGridRegistry(testdir, dynamicRegistration = False):

    iceGrid = TestUtil.getIceGridRegistry()

    command = ' --nowarn ' + registryOptions
    if dynamicRegistration:
        command += r' --IceGrid.Registry.DynamicRegistration'

    procs = []
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

        sys.stdout.write("starting icegrid " + name + "... ")
        sys.stdout.flush()
        cmd = command + ' ' + \
              r' --Ice.ProgramName=' + name + \
              r' --IceGrid.Registry.Client.Endpoints="default -p ' + str(iceGridPort + i) + '" ' + \
              r' --IceGrid.Registry.Data="' + dataDir + '" '

        if i > 0:
            cmd += r' --IceGrid.Registry.ReplicaName=' + name + ' ' + getDefaultLocatorProperty()

        driverConfig = TestUtil.DriverConfig("server")
        driverConfig.lang = "cpp"
        proc = TestUtil.startServer(iceGrid, cmd, driverConfig, count = 5)
        procs.append(proc)
        print("ok")

        i = i + 1
    return procs

def shutdownIceGridRegistry(procs):

    i = nreplicas
    while i > 0:
        sys.stdout.write("shutting down icegrid replica-" + str(i) + "... ")
        sys.stdout.flush()
        iceGridAdmin("registry shutdown replica-" + str(i))
        print("ok")
        i = i - 1

    sys.stdout.write("shutting down icegrid registry... ")
    sys.stdout.flush()
    iceGridAdmin("registry shutdown")
    print("ok")

    for p in procs:
        p.waitTestSuccess()

def iceGridNodePropertiesOverride():

    #
    # Create property overrides from command line options.
    #
    overrideOptions = ''
    for opt in shlex.split(TestUtil.getCommandLineProperties("", TestUtil.DriverConfig("server"))):
       opt = opt.strip().replace("--", "")
       index = opt.find("=")
       if index == -1:
          overrideOptions += ("%s=1 ") % opt
       else:
          key = opt[0:index]
          value = opt[index + 1:]
          if(value.find(' ') == -1):
             overrideOptions += ("%s=%s ") % (key, value)
          else:
             #
             # NOTE: We need 2 backslash before the quote to run the
             # C# test/IceGrid/simple test with SSL.
             #
             overrideOptions += ("%s=\\\"%s\\\" ") % (key, value.replace('"', '\\\\\\"'))

    return overrideOptions

def startIceGridNode(testdir):

    iceGrid = TestUtil.getIceGridNode()
    dataDir = os.path.join(testdir, "db", "node")
    if not os.path.exists(dataDir):
        os.mkdir(dataDir)
    else:
        cleanDbDir(dataDir)

    overrideOptions = '" ' + iceGridNodePropertiesOverride()
    overrideOptions += ' Ice.ServerIdleTime=0 Ice.PrintProcessId=0 Ice.PrintAdapterReady=0"'

    sys.stdout.write("starting icegrid node... ")
    sys.stdout.flush()
    command = r' --nowarn ' + nodeOptions + getDefaultLocatorProperty() + \
              r' --IceGrid.Node.Data="' + dataDir + '"' \
              r' --IceGrid.Node.Name=localnode' + \
              r' --IceGrid.Node.PropertiesOverride=' + overrideOptions

    driverConfig = TestUtil.DriverConfig("server")
    driverConfig.lang = "cpp"
    proc = TestUtil.startServer(iceGrid, command, driverConfig, adapter='node1')

    print("ok")

    return proc

def iceGridAdmin(cmd, ignoreFailure = False):

    iceGridAdmin = TestUtil.getIceGridAdmin()

    user = r"admin1"
    if cmd == "registry shutdown":
        user = r"shutdown"
    command = getDefaultLocatorProperty() + r" --IceGridAdmin.Username=" + user + " --IceGridAdmin.Password=test1 " + \
              r' -e "' + cmd + '"'

    if TestUtil.appverifier:
        TestUtil.setAppVerifierSettings([TestUtil.getIceGridAdmin()])

    driverConfig = TestUtil.DriverConfig("client")
    driverConfig.lang = "cpp"
    proc = TestUtil.startClient(iceGridAdmin, command, driverConfig)
    status = proc.wait()

    if TestUtil.appverifier:
        TestUtil.appVerifierAfterTestEnd([TestUtil.getIceGridAdmin()])

    if not ignoreFailure and status:
        print(proc.buf)
        sys.exit(1)
    return proc.buf

def killNodeServers():

    for server in iceGridAdmin("server list"):
        server = server.strip()
        iceGridAdmin("server disable " + server, True)
        iceGridAdmin("server signal " + server + " SIGKILL", True)

def iceGridTest(application, additionalOptions = "", applicationOptions = ""):

    testdir = os.getcwd()
    if not TestUtil.isWin32() and os.getuid() == 0:
        print
        print("*** can't run test as root ***")
        print
        return

    client = TestUtil.getDefaultClientFile()
    if TestUtil.getDefaultMapping() != "java":
        client = os.path.join(testdir, client)

    clientOptions = ' ' + getDefaultLocatorProperty() + ' ' + additionalOptions

    targets = []
    if TestUtil.appverifier:
        targets = [client, TestUtil.getIceGridNode(), TestUtil.getIceGridRegistry()]
        TestUtil.setAppVerifierSettings(targets)

    registryProcs = startIceGridRegistry(testdir)
    iceGridNodeProc = startIceGridNode(testdir)

    javaHome = os.environ.get("JAVA_HOME", None)
    javaExe = os.path.join(javaHome, "bin", "java") if javaHome else "java"

    if application != "":
        sys.stdout.write("adding application... ")
        sys.stdout.flush()
        iceGridAdmin("application add -n '" + os.path.join(testdir, application) + "' " +
                     "test.dir='" + testdir + "' " +
                     "ice.bindir='" + TestUtil.getCppBinDir() + "' " +
                     "java.exe='" + javaExe + "' " +
                     applicationOptions)
        print("ok")

    sys.stdout.write("starting client... ")
    sys.stdout.flush()
    clientProc = TestUtil.startClient(client, clientOptions, TestUtil.DriverConfig("client"), startReader = False)
    print("ok")
    clientProc.startReader()
    clientProc.waitTestSuccess()

    if application != "":
        sys.stdout.write("remove application... ")
        sys.stdout.flush()
        iceGridAdmin("application remove Test")
        print("ok")

    sys.stdout.write("shutting down icegrid node... ")
    sys.stdout.flush()
    iceGridAdmin("node shutdown localnode")
    print("ok")
    shutdownIceGridRegistry(registryProcs)
    iceGridNodeProc.waitTestSuccess()

    if TestUtil.appverifier:
        TestUtil.appVerifierAfterTestEnd(targets)

def iceGridClientServerTest(additionalClientOptions, additionalServerOptions):

    testdir = os.getcwd()
    server = TestUtil.getDefaultServerFile()
    client = TestUtil.getDefaultClientFile()
    if TestUtil.getDefaultMapping() != "java":
        server = os.path.join(testdir, server)
        client = os.path.join(testdir, client)

    targets = []
    if TestUtil.appverifier:
        targets = [client, server, TestUtil.getIceGridRegistry()]
        TestUtil.setAppVerifierSettings(targets)

    clientOptions = getDefaultLocatorProperty() + ' ' + additionalClientOptions
    serverOptions = getDefaultLocatorProperty() + ' ' + additionalServerOptions

    registryProcs = startIceGridRegistry(testdir, True)

    sys.stdout.write("starting server... ")
    sys.stdout.flush()
    serverProc= TestUtil.startServer(server, serverOptions, TestUtil.DriverConfig("server"))
    print("ok")

    sys.stdout.write("starting client... ")
    sys.stdout.flush()
    clientProc = TestUtil.startClient(client, clientOptions, TestUtil.DriverConfig("client"))
    print("ok")

    clientProc.waitTestSuccess()
    serverProc.waitTestSuccess()

    shutdownIceGridRegistry(registryProcs)

    if TestUtil.appverifier:
        TestUtil.appVerifierAfterTestEnd(targets)

def cleanDbDir(path):
    for filename in [ os.path.join(path, f) for f in os.listdir(path) if f != ".gitignore"]:
        if os.path.isdir(filename):
            cleanDbDir(filename)
            try:
                os.rmdir(filename)
            except OSError:
                # This might fail if the directory is empty (because
                # it itself contains a .gitignore file.
                pass
        else:
            os.remove(filename)
