#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import sys, os
from Util import *
from IceBoxUtil import *
from Glacier2Util import *

class IceGridProcess:

    def __init__(self, replica):
        self.replica = replica

    def getProps(self, current):
        props = self.getParentProps(current)
        testcase = current.testcase
        while testcase and not isinstance(testcase, IceGridTestCase): testcase = testcase.parent
        if self.replica is None:
            props["Ice.Default.Locator"] = testcase.getMasterLocator(current)
        else:
            for r in testcase.icegridregistry:
                # Match either the IceGridRegistrySlave object or the slave replica number
                if self.replica in [r, r.name]:
                    props["Ice.Default.Locator"] = r.getLocator(current)
                    break
        return props

class IceGridServer(IceGridProcess, Server):

    def __init__(self, replica=None, *args, **kargs):
        Server.__init__(self, *args, **kargs)
        IceGridProcess.__init__(self, replica)

    getParentProps = Server.getProps # Used by IceGridProcess to get the server properties

class IceGridClient(IceGridProcess, Client):

    def __init__(self, replica=None, *args, **kargs):
        Client.__init__(self, *args, **kargs)
        IceGridProcess.__init__(self, replica)

    getParentProps = Client.getProps # Used by IceGridProcess to get the client properties

class IceGridAdmin(ProcessIsFromPath, ProcessIsReleaseOnly, IceGridClient):

    def __init__(self, replica=None, username="admin1", password="test1", *args, **kargs):
        IceGridClient.__init__(self, replica=replica, exe="icegridadmin", mapping=Mapping.getByName("cpp"),
                               *args, **kargs)
        self.username = username
        self.password = password

    def getExe(self, current):
        return self.exe + "_32" if current.config.buildPlatform == "ppc" else self.exe

    def getProps(self, current):
        props = IceGridClient.getProps(self, current)
        props["IceGridAdmin.Username"] = self.username
        props["IceGridAdmin.Password"] = self.password
        return props

class IceGridNode(ProcessIsFromPath, Server):

    def __init__(self, name="localnode", *args, **kargs):

        Server.__init__(self, "icegridnode", mapping=Mapping.getByName("cpp"), desc="IceGrid node " + name,
                        ready="node", *args, **kargs)
        self.name = name

    def getExe(self, current):
        return self.exe + "_32" if current.config.buildPlatform == "ppc" else self.exe

    def setup(self, current):
        # Create the database directory
        self.dbdir = os.path.join(current.testsuite.getPath(), "node-{0}".format(self.name))
        if os.path.exists(self.dbdir):
            shutil.rmtree(self.dbdir)
        os.mkdir(self.dbdir)

    def teardown(self, current, success):
        Server.teardown(self, current, success)
        # Remove the database directory tree
        if success or current.driver.isInterrupted():
            try:
                shutil.rmtree(self.dbdir)
            except:
                pass

    def getProps(self, current):
        host = current.driver.getHost(current.config.transport, current.config.ipv6)
        props = {
            'IceGrid.InstanceName' : 'TestIceGrid',
            'IceGrid.Node.Endpoints' : f'default -h {host}',
            'IceGrid.Node.WaitTime' : 240,
            'Ice.ProgramName' : 'icegridnode',
            'IceGrid.Node.Trace.Replica' : 0,
            'IceGrid.Node.Trace.Activator' : 0,
            'IceGrid.Node.Trace.Adapter' : 0,
            'IceGrid.Node.Trace.Server' : 0,
            'IceGrid.Node.ThreadPool.WarnSize' : 0,
            'IceGrid.Node.PrintServersReady' : 'node',
            'IceGrid.Node.Name' : self.name,
            'IceGrid.Node.Data' : '{testdir}/node-{process.name}',
            'IceGrid.Node.PropertiesOverride' : self.getPropertiesOverride(current),
            'Ice.Default.Locator' : current.testcase.getLocator(current),
            'Ice.NullHandleAbort'  : 1,
        }
        return props

    def getEnv(self, current):
        # Add environment variable for servers based on the test case mapping.
        return Server().getEffectiveEnv(current)

    def getPropertiesOverride(self, current):
        # Add properties for servers based on the test case mapping.
        props = Server().getEffectiveProps(current, {})
        return ' '.join(["{0}={1}".format(k, val(v)) for k, v in props.items()])

    def shutdown(self, current):
        current.testcase.runadmin(current, "node shutdown {0}".format(self.name))

class IceGridRegistry(ProcessIsFromPath, Server):

    def __init__(self, name, portnum=20, ready="AdminSessionManager", *args, **kargs):
        Server.__init__(self, "icegridregistry", mapping=Mapping.getByName("cpp"), desc="IceGrid registry " + name,
                        ready=ready, *args, **kargs)
        self.portnum = portnum
        self.readyCount = -1
        self.name = name

    def getExe(self, current):
        return self.exe + "_32" if current.config.buildPlatform == "ppc" else self.exe

    def setup(self, current):
        # Create the database directory
        self.dbdir = os.path.join(current.testsuite.getPath(), "registry-{0}".format(self.name))
        if os.path.exists(self.dbdir):
            shutil.rmtree(self.dbdir)
        os.mkdir(self.dbdir)

    def teardown(self, current, success):
        Server.teardown(self, current, success)
        # Remove the database directory tree
        if success or current.driver.isInterrupted():
            try:
                shutil.rmtree(self.dbdir)
            except:
                pass

    def getProps(self, current):
        host = current.driver.getHost(current.config.transport, current.config.ipv6)
        props = {
            'IceGrid.InstanceName' : 'TestIceGrid',
            'IceGrid.Registry.PermissionsVerifier' : 'TestIceGrid/NullPermissionsVerifier',
            'IceGrid.Registry.AdminPermissionsVerifier' : 'TestIceGrid/NullPermissionsVerifier',
            'IceGrid.Registry.SSLPermissionsVerifier' : 'TestIceGrid/NullSSLPermissionsVerifier',
            'IceGrid.Registry.AdminSSLPermissionsVerifier' : 'TestIceGrid/NullSSLPermissionsVerifier',
            'IceGrid.Registry.Server.Endpoints' : f'default -h {host}',
            'IceGrid.Registry.Internal.Endpoints' : f'default -h {host}',
            'IceGrid.Registry.Client.Endpoints' : self.getEndpoints(current),
            'IceGrid.Registry.Discovery.Port' : current.driver.getTestPort(99),
            'IceGrid.Registry.SessionManager.Endpoints' : f'default -h {host}',
            'IceGrid.Registry.AdminSessionManager.Endpoints' : f'default -h {host}',
            'IceGrid.Registry.SessionTimeout' : 60,
            'IceGrid.Registry.ReplicaName' : self.name,
            'Ice.ProgramName' : self.name,
            'Ice.PrintAdapterReady' : 1,
            'Ice.Warn.Connections' : 0,
            'Ice.ThreadPool.Client.WarnSize' : 0,
            'IceGrid.Registry.LMDB.MapSize' : 1,
            'IceGrid.Registry.LMDB.Path' : '{testdir}/registry-{process.name}',
            'IceGrid.Registry.Client.ThreadPool.WarnSize' : 0,
            'IceGrid.Registry.DefaultTemplates' :
                '"' + os.path.abspath(os.path.join(toplevel, "cpp", "config", "templates.xml")) + '"'
        }
        if not isinstance(platform, Linux):
            props["IceGrid.Registry.Discovery.Interface"] = "::1" if current.config.ipv6 else "127.0.0.1"
        return props

    def getEndpoints(self, current):
        return current.getTestEndpoint(self.portnum, current.config.transport)

    def getLocator(self, current):
        return "TestIceGrid/Locator:{0}".format(self.getEndpoints(current))

    def shutdown(self, current):
        current.testcase.runadmin(current, "registry shutdown {0}".format(self.name), replica=self.name)

class IceGridRegistryMaster(IceGridRegistry):

    def __init__(self, portnum=20, *args, **kargs):
        IceGridRegistry.__init__(self, "Master", portnum, *args, **kargs)

class IceGridRegistrySlave(IceGridRegistry):

    def __init__(self, replica=1, portnum=None, *args, **kargs):
        IceGridRegistry.__init__(self, "Slave{0}".format(replica), (20 + replica) if portnum is None else portnum,
                                 *args, **kargs)

    def getProps(self, current):
        props = IceGridRegistry.getProps(self, current)
        props["Ice.Default.Locator"] = current.testcase.getMasterLocator(current)
        return props

class IceGridTestCase(TestCase):

    def __init__(self, name="IceGrid", icegridregistry=None, icegridnode=None, application="application.xml",
                 variables={}, targets=[], exevars={}, *args, **kargs):
        TestCase.__init__(self, name, *args, **kargs)
        if icegridnode:
            self.icegridnode = icegridnode if isinstance(icegridnode, list) else [icegridnode]
        else:
            self.icegridnode = [IceGridNode()]

        if icegridregistry:
            self.icegridregistry = icegridregistry if isinstance(icegridregistry, list) else [icegridregistry]
        else:
            self.icegridregistry = [IceGridRegistryMaster(), IceGridRegistrySlave(1)]

        self.application = application
        self.variables = variables
        self.targets = targets

        # Variables for built executables
        self.exevars = { "server.dir" : "server" }
        self.exevars.update(exevars)

    def init(self, mapping, testsuite):
        TestCase.init(self, mapping, testsuite)

        #
        # Add IceGrid servers at the beginning of the server list, IceGrid needs to be
        # started first!
        #
        self.servers = self.icegridregistry + self.icegridnode + self.servers

    def setupClientSide(self, current):
        if self.application:
            try:
                self.runadmin(current, "application remove Test", quiet=True)
            except:
                pass

            javaHome = os.environ.get("JAVA_HOME", None)
            serverProps = Server().getProps(current)
            variables = {
                "test.dir" : self.getPath(current),
                "test.host": current.driver.getHost(current.config.transport, current.config.ipv6),
                "test.transport": current.config.transport,
                "java.exe" : os.path.join(javaHome, "bin", "java") if javaHome else "java",
                "icebox.exe" : IceBox().getCommandLine(current),
                "icegridnode.exe" : IceGridNode().getCommandLine(current),
                "glacier2router.exe" : Glacier2Router().getCommandLine(current),
                "icegridregistry.exe" : IceGridRegistryMaster().getCommandLine(current),
                "properties-override" : self.icegridnode[0].getPropertiesOverride(current),
            }

            if platform.getDotNetExe():
                variables["dotnet.exe"] = platform.getDotNetExe()

            # Add variables that point to the directories containing the built executables
            for (k, v) in self.exevars.items():
                variables[k] = current.getBuildDir(v)

            variables.update(self.variables)
            varStr = " ".join(["{0}={1}".format(k, val(v)) for k,v in variables.items()])
            targets = " ".join(self.targets)
            application = self.application
            if isinstance(self.mapping, CSharpMapping):
                application = application.replace(".xml", ".netcoreapp.xml")
            self.runadmin(current, "application add {0} {1} {2}".format(application, varStr, targets))

    def teardownClientSide(self, current, success):
        if (success or current.driver.isInterrupted()) and self.application:
            self.runadmin(current, "application remove Test")

        for p in self.icegridnode + self.icegridregistry:
            p.shutdown(current)

    def getLocator(self, current):
        endpoints = ":".join([s.getEndpoints(current) for s in self.servers if isinstance(s, IceGridRegistry)])
        return "TestIceGrid/Locator:{0}".format(endpoints)

    def getMasterLocator(self, current):
        for s in self.servers:
            if isinstance(s, IceGridRegistryMaster):
                return "TestIceGrid/Locator:{0}".format(s.getEndpoints(current))

    def runadmin(self, current, cmd, replica="Master", exitstatus=0, quiet=False):
        admin = IceGridAdmin(args=["-r", replica, "-e", cmd], replica=replica, quiet=quiet)
        admin.run(current, exitstatus=exitstatus)
        return admin.getOutput(current)

    def runWithDriver(self, current):
        current.driver.runClientServerTestCase(current)

    def getClientType(self):
        return "client"
