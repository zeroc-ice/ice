# Copyright (c) ZeroC, Inc.

from __future__ import annotations

import os
import shutil
from typing import TYPE_CHECKING, Any

from Glacier2Util import Glacier2Router
from IceBoxUtil import IceBox
from Util import (
    Client,
    CSharpMapping,
    Driver,
    Envs,
    Linux,
    Mapping,
    Process,
    ProcessFromBinDir,
    ProcessIsReleaseOnly,
    Props,
    Server,
    TestCase,
    TestSuite,
    platform,
    toplevel,
    val,
)

# IceGridProcess is mixed into the Client and Server specializations below rather than deriving from
# Process, so that each can pick its own parent properties. Giving it Process as a base for type
# checking only lets it use the Process API without changing the runtime method resolution order.
_IceGridProcessBase = Process if TYPE_CHECKING else object


class IceGridProcess(_IceGridProcessBase):
    def __init__(self, replica: IceGridRegistry | str | None):
        self.replica = replica

    def getParentProps(self, current: Driver.Current) -> Props:
        # Bound to the parent Client or Server getProps by each specialization below.
        raise NotImplementedError()

    def getProps(self, current: Driver.Current) -> Props:
        props = self.getParentProps(current)
        testcase = current.testcase
        while testcase and not isinstance(testcase, IceGridTestCase):
            testcase = testcase.parent
        assert isinstance(testcase, IceGridTestCase)
        if self.replica is None:
            props["Ice.Default.Locator"] = testcase.getMasterLocator(current)
        else:
            for r in testcase.icegridregistry:
                # Match either the IceGridRegistry object or its replica name (e.g. "Master", "Slave1")
                if self.replica in [r, r.name]:
                    props["Ice.Default.Locator"] = r.getLocator(current)
                    break
        return props


class IceGridServer(IceGridProcess, Server):
    def __init__(self, replica: IceGridRegistry | str | None = None, *args: Any, **kargs: Any):
        Server.__init__(self, *args, **kargs)
        IceGridProcess.__init__(self, replica)

    def getParentProps(self, current: Driver.Current) -> Props:
        return Server.getProps(self, current)


class IceGridClient(IceGridProcess, Client):
    def __init__(self, replica: IceGridRegistry | str | None = None, *args: Any, **kargs: Any):
        Client.__init__(self, *args, **kargs)
        IceGridProcess.__init__(self, replica)

    def getParentProps(self, current: Driver.Current) -> Props:
        return Client.getProps(self, current)


class IceGridAdmin(ProcessFromBinDir, ProcessIsReleaseOnly, IceGridClient):
    def __init__(
        self,
        replica: IceGridRegistry | str | None = None,
        username: str = "admin1",
        password: str = "test1",
        *args: Any,
        **kargs: Any,
    ):
        IceGridClient.__init__(
            self,
            replica=replica,
            exe="icegridadmin",
            mapping=Mapping.getByName("cpp"),
            *args,
            **kargs,
        )
        self.username = username
        self.password = password

    def getExe(self, current: Driver.Current) -> str:
        assert self.exe is not None
        return self.exe + "_32" if current.config.buildPlatform == "ppc" else self.exe

    def getProps(self, current: Driver.Current) -> Props:
        props = IceGridClient.getProps(self, current)
        props["IceGridAdmin.Username"] = self.username
        props["IceGridAdmin.Password"] = self.password
        return props


class IceGridNode(ProcessFromBinDir, Server):
    def __init__(self, name: str = "localnode", *args: Any, **kargs: Any):
        Server.__init__(
            self,
            "icegridnode",
            mapping=Mapping.getByName("cpp"),
            desc="IceGrid node " + name,
            ready="node",
            *args,
            **kargs,
        )
        self.name = name

    def getExe(self, current: Driver.Current) -> str:
        assert self.exe is not None
        return self.exe + "_32" if current.config.buildPlatform == "ppc" else self.exe

    def setup(self, current: Driver.Current) -> None:
        # Create the database directory
        self.dbdir = os.path.join(current.testsuite.getPath(), "node-{0}".format(self.name))
        if os.path.exists(self.dbdir):
            shutil.rmtree(self.dbdir)
        os.mkdir(self.dbdir)

    def teardown(self, current: Driver.Current, success: bool) -> None:
        Server.teardown(self, current, success)
        # Remove the database directory tree
        if success or current.driver.isInterrupted():
            try:
                shutil.rmtree(self.dbdir)
            except Exception:
                pass

    def getProps(self, current: Driver.Current) -> Props:
        testcase = current.getTestCase()
        assert isinstance(testcase, IceGridTestCase)
        locator = testcase.getLocator(current)
        props: Props = {
            "IceGrid.InstanceName": "TestIceGrid",
            "IceGrid.Node.Endpoints": "default",
            "IceGrid.Node.WaitTime": 240,
            "Ice.ProgramName": "icegridnode",
            "IceGrid.Node.Trace.Replica": 0,
            "IceGrid.Node.Trace.Activator": 0,
            "IceGrid.Node.Trace.Adapter": 0,
            "IceGrid.Node.Trace.Server": 0,
            "IceGrid.Node.ThreadPool.SizeWarn": 0,
            "IceGrid.Node.PrintServersReady": "node",
            "IceGrid.Node.Name": self.name,
            "IceGrid.Node.Data": "{testdir}/node-{process.name}",
            "IceGrid.Node.PropertiesOverride": self.getPropertiesOverride(current),
            "Ice.Default.Locator": locator,
        }
        return props

    def getEnv(self, current: Driver.Current) -> Envs:
        # Add environment variable for servers based on the test case mapping.
        return Server().getEffectiveEnv(current)

    def getPropertiesOverride(self, current: Driver.Current) -> str:
        # Add properties for servers based on the test case mapping.
        props = Server().getEffectiveProps(current, {})
        # Remove Server thread pool properties set by the base Server class.
        # These properties are not used by icegridnode and cause warnings during tests.
        del props["Ice.ThreadPool.Server.Size"]
        del props["Ice.ThreadPool.Server.SizeMax"]
        del props["Ice.ThreadPool.Server.SizeWarn"]
        return " ".join(["{0}={1}".format(k, val(v)) for k, v in props.items()])

    def shutdown(self, current: Driver.Current) -> None:
        testcase = current.getTestCase()
        assert isinstance(testcase, IceGridTestCase)
        testcase.runadmin(current, "node shutdown {0}".format(self.name))


class IceGridRegistry(ProcessFromBinDir, Server):
    def __init__(self, name: str, portnum: int = 20, ready: str = "AdminSessionManager", *args: Any, **kargs: Any):
        Server.__init__(
            self,
            "icegridregistry",
            mapping=Mapping.getByName("cpp"),
            desc="IceGrid registry " + name,
            ready=ready,
            *args,
            **kargs,
        )
        self.portnum = portnum
        self.readyCount = -1
        self.name = name

    def getExe(self, current: Driver.Current) -> str:
        assert self.exe is not None
        return self.exe + "_32" if current.config.buildPlatform == "ppc" else self.exe

    def setup(self, current: Driver.Current) -> None:
        # Create the database directory
        self.dbdir = os.path.join(current.testsuite.getPath(), "registry-{0}".format(self.name))
        if os.path.exists(self.dbdir):
            shutil.rmtree(self.dbdir)
        os.mkdir(self.dbdir)

    def teardown(self, current: Driver.Current, success: bool) -> None:
        Server.teardown(self, current, success)
        # Remove the database directory tree
        if success or current.driver.isInterrupted():
            try:
                shutil.rmtree(self.dbdir)
            except Exception:
                pass

    def getProps(self, current: Driver.Current) -> Props:
        props: Props = {
            "IceGrid.InstanceName": "TestIceGrid",
            "IceGrid.Registry.PermissionsVerifier": "TestIceGrid/NullPermissionsVerifier",
            "IceGrid.Registry.AdminPermissionsVerifier": "TestIceGrid/NullPermissionsVerifier",
            "IceGrid.Registry.SSLPermissionsVerifier": "TestIceGrid/NullSSLPermissionsVerifier",
            "IceGrid.Registry.AdminSSLPermissionsVerifier": "TestIceGrid/NullSSLPermissionsVerifier",
            "IceGrid.Registry.Server.Endpoints": "default",
            "IceGrid.Registry.Internal.Endpoints": "default",
            "IceGrid.Registry.Client.Endpoints": self.getEndpoints(current),
            "IceGrid.Registry.Discovery.Port": current.driver.getTestPort(99),
            "IceGrid.Registry.SessionManager.Endpoints": "default",
            "IceGrid.Registry.AdminSessionManager.Endpoints": "default",
            "IceGrid.Registry.ReplicaName": self.name,
            "Ice.ProgramName": self.name,
            "Ice.PrintAdapterReady": 1,
            "Ice.Warn.Connections": 0,
            "Ice.ThreadPool.Client.SizeWarn": 0,
            "IceGrid.Registry.LMDB.MapSize": 1,
            "IceGrid.Registry.LMDB.Path": "{testdir}/registry-{process.name}",
            "IceGrid.Registry.Client.ThreadPool.SizeWarn": 0,
            "IceGrid.Registry.DefaultTemplates": '"'
            + os.path.abspath(os.path.join(toplevel, "cpp", "config", "templates.xml"))
            + '"',
        }
        if not isinstance(platform, Linux):
            props["IceGrid.Registry.Discovery.Interface"] = "::1" if current.config.ipv6 else "127.0.0.1"
        return props

    def getEndpoints(self, current: Driver.Current) -> str:
        return current.getTestEndpoint(self.portnum)

    def getLocator(self, current: Driver.Current) -> str:
        return "TestIceGrid/Locator:{0}".format(self.getEndpoints(current))

    def shutdown(self, current: Driver.Current) -> None:
        testcase = current.getTestCase()
        assert isinstance(testcase, IceGridTestCase)
        testcase.runadmin(current, "registry shutdown {0}".format(self.name), replica=self.name)


class IceGridRegistryMaster(IceGridRegistry):
    def __init__(self, portnum: int = 20, *args: Any, **kargs: Any):
        IceGridRegistry.__init__(self, "Master", portnum, *args, **kargs)


class IceGridRegistrySlave(IceGridRegistry):
    def __init__(self, replica: int = 1, portnum: int | None = None, *args: Any, **kargs: Any):
        IceGridRegistry.__init__(
            self,
            "Slave{0}".format(replica),
            (20 + replica) if portnum is None else portnum,
            *args,
            **kargs,
        )

    def getProps(self, current: Driver.Current) -> Props:
        props = IceGridRegistry.getProps(self, current)
        testcase = current.getTestCase()
        assert isinstance(testcase, IceGridTestCase)
        props["Ice.Default.Locator"] = testcase.getMasterLocator(current)
        return props


class IceGridTestCase(TestCase):
    def __init__(
        self,
        name: str = "IceGrid",
        icegridregistry: IceGridRegistry | list[IceGridRegistry] | None = None,
        icegridnode: IceGridNode | list[IceGridNode] | None = None,
        application: str = "application.xml",
        variables: dict[str, Any] = {},
        targets: list[str] = [],
        exevars: dict[str, str] = {},
        *args: Any,
        **kargs: Any,
    ):
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
        self.exevars = {"server.dir": "server"}
        self.exevars.update(exevars)

    def init(self, mapping: Mapping, testsuite: TestSuite) -> None:
        TestCase.init(self, mapping, testsuite)

        #
        # Add IceGrid servers at the beginning of the server list, IceGrid needs to be
        # started first!
        #
        self.servers = list(self.icegridregistry) + list(self.icegridnode) + self.getServers()

    def setupClientSide(self, current: Driver.Current) -> None:
        if self.application:
            try:
                self.runadmin(current, "application remove Test", quiet=True)
            except Exception:
                pass

            javaHome = os.environ.get("JAVA_HOME", None)
            variables: dict[str, Any] = {
                "test.dir": self.getPath(current),
                "java.exe": os.path.join(javaHome, "bin", "java") if javaHome else "java",
                "icebox.exe": IceBox().getCommandLine(current),
                "icegridnode.exe": IceGridNode().getCommandLine(current),
                "glacier2router.exe": Glacier2Router().getCommandLine(current),
                "icegridregistry.exe": IceGridRegistryMaster().getCommandLine(current),
                "properties-override": self.icegridnode[0].getPropertiesOverride(current),
            }

            dotnetExe = platform.getDotNetExe()
            if dotnetExe:
                variables["dotnet.exe"] = dotnetExe

            # Add variables that point to the directories containing the built executables
            for k, v in self.exevars.items():
                variables[k] = current.getBuildDir(v)

            variables.update(self.variables)
            varStr = " ".join(["{0}={1}".format(k, val(v)) for k, v in variables.items()])
            targets = " ".join(self.targets)
            application = self.application
            if isinstance(self.mapping, CSharpMapping) and current.config.dotnet:
                application = application.replace(".xml", ".dotnet.xml")
            self.runadmin(
                current,
                "application add {0} {1} {2}".format(application, varStr, targets),
            )

    def teardownClientSide(self, current: Driver.Current, success: bool) -> None:
        if (success or current.driver.isInterrupted()) and self.application:
            self.runadmin(current, "application remove Test")

        for p in list(self.icegridnode) + list(self.icegridregistry):
            p.shutdown(current)

    def getLocator(self, current: Driver.Current) -> str:
        endpoints = ":".join([s.getEndpoints(current) for s in self.getServers() if isinstance(s, IceGridRegistry)])
        return "TestIceGrid/Locator:{0}".format(endpoints)

    def getMasterLocator(self, current: Driver.Current) -> str | None:
        for s in self.getServers():
            if isinstance(s, IceGridRegistryMaster):
                return "TestIceGrid/Locator:{0}".format(s.getEndpoints(current))
        return None

    def runadmin(
        self,
        current: Driver.Current,
        cmd: str,
        replica: str = "Master",
        exitstatus: int = 0,
        quiet: bool = False,
    ) -> str:
        admin = IceGridAdmin(args=["-r", replica, "-e", cmd], replica=replica, quiet=quiet)
        admin.run(current, exitstatus=exitstatus)
        return admin.getOutput(current)

    def runWithDriver(self, current: Driver.Current) -> None:
        current.driver.runClientServerTestCase(current)

    def getClientType(self) -> str | None:
        return "client"
