# Copyright (c) ZeroC, Inc.

from __future__ import annotations

import os
import shutil
from typing import TYPE_CHECKING, Any

import Component
from IceBoxUtil import IceBoxAdmin
from Util import (
    Client,
    Driver,
    Mapping,
    Process,
    ProcessFromBinDir,
    ProcessIsReleaseOnly,
    Props,
    Server,
    TestCase,
    TestSuite,
)


class IceStorm(ProcessFromBinDir, Server):
    def __init__(
        self,
        instanceName: str = "IceStorm",
        replica: int = 0,
        nreplicas: int = 0,
        transient: bool = False,
        portnum: int = 0,
        createDb: bool = True,
        cleanDb: bool = True,
        *args: Any,
        **kargs: Any,
    ):
        Server.__init__(
            self,
            exe="icebox",
            ready="IceStorm",
            mapping=Mapping.getByName("cpp"),
            *args,
            **kargs,
        )
        self.portnum = portnum
        self.replica = replica
        self.nreplicas = nreplicas
        self.transient = transient
        self.instanceName = instanceName
        self.createDb = createDb
        self.cleanDb = cleanDb
        self.desc = (
            self.instanceName if self.nreplicas == 0 else "{0} replica #{1}".format(self.instanceName, self.replica)
        )

    def getExe(self, current: Driver.Current) -> str:
        assert self.exe is not None
        return self.exe + "_32" if current.config.buildPlatform == "ppc" else self.exe

    def setup(self, current: Driver.Current) -> None:
        # Create the database directory
        self.dbdir = os.path.join(
            current.testsuite.getPath(),
            "{0}-{1}.db".format(self.instanceName, self.replica),
        )
        if self.createDb:
            if os.path.exists(self.dbdir):
                shutil.rmtree(self.dbdir)
            os.mkdir(self.dbdir)

    def teardown(self, current: Driver.Current, success: bool) -> None:
        Server.teardown(self, current, success)
        if self.cleanDb:
            # Remove the database directory tree
            try:
                shutil.rmtree(self.dbdir)
            except Exception:
                pass

    def getProps(self, current: Driver.Current) -> Props:
        props = Server.getProps(self, current)

        # Default properties
        props.update(
            {
                # We don't use IceStorm as the service name to make sure the service name is not used
                # by accident as a property prefix.
                "IceBox.Service.TestIceStorm": "IceStormService,"
                + Component.component.getSoVersion()
                + ":createIceStorm",
                "IceBox.PrintServicesReady": "IceStorm",
                "IceBox.InheritProperties": 1,
                "IceStorm.InstanceName": self.instanceName,
                "Ice.Admin.InstanceName": "IceBox",
                "Ice.Warn.Dispatch": 0,
                "Ice.Warn.Connections": 0,
                "IceStorm.LMDB.MapSize": 1,
                "IceStorm.LMDB.Path": "{testdir}/{process.instanceName}-{process.replica}.db",
            }
        )

        if self.nreplicas > 0:
            props["IceStorm.NodeId"] = self.replica

        if self.transient:
            props["IceStorm.Transient"] = 1

        #
        # Add endpoint properties here as these properties depend on the worker thread running the
        # the test case for the port number. The port number is computed by the driver based on a
        # fixed portnum index for each IceStorm endpoint (portnum = 0 for the topic manager endpoint,
        # portnum=1 for the publish endpoint, portnum=2 for the node endpoint and portnum=3 for the
        # icebox admin endpoint).
        #

        # Manager, publish, node and admin endpoints for given replica number
        def manager(replica: int) -> str:
            return current.getTestEndpoint(self.portnum + replica * 4 + 0)

        def publish(replica: int) -> str:
            return "{0}:{1}".format(
                current.getTestEndpoint(self.portnum + replica * 4 + 1),
                current.getTestEndpoint(self.portnum + replica * 4 + 1, "udp"),
            )

        def node(replica: int) -> str:
            return current.getTestEndpoint(self.portnum + replica * 4 + 2)

        def admin(replica: int) -> str:
            return current.getTestEndpoint(self.portnum + replica * 4 + 3)

        # The endpoints for the given replica
        props.update(
            {
                "IceStorm.TopicManager.Endpoints": manager(self.replica),
                "IceStorm.Publish.Endpoints": publish(self.replica),
                "Ice.Admin.Endpoints": admin(self.replica),
            }
        )

        # Compute the node and replicated endpoints to be configured for each replica
        if self.nreplicas > 0:
            props["IceStorm.Node.Endpoints"] = node(self.replica)
            for i in range(0, self.nreplicas):
                props["IceStorm.Nodes.{0}".format(i)] = "{2}/node{0}:{1}".format(i, node(i), self.instanceName)
            props["IceStorm.ReplicatedTopicManagerEndpoints"] = ":".join([manager(i) for i in range(0, self.nreplicas)])
            props["IceStorm.ReplicatedPublishEndpoints"] = ":".join([publish(i) for i in range(0, self.nreplicas)])

        return props

    def getInstanceName(self) -> str:
        return self.instanceName

    def getTopicManager(self, current: Driver.Current) -> str:
        # Return the endpoint for this IceStorm replica
        return "{1}/TopicManager:{0}".format(
            current.getTestEndpoint(self.portnum + self.replica * 4), self.instanceName
        )

    def getReplicatedTopicManager(self, current: Driver.Current) -> str:
        # Return the replicated endpoints for IceStorm
        if self.nreplicas == 0:
            return self.getTopicManager(current)

        def manager(replica: int) -> str:
            return current.getTestEndpoint(self.portnum + replica * 4)

        return "{1}/TopicManager:{0}".format(
            ":".join([manager(i) for i in range(0, self.nreplicas)]), self.instanceName
        )

    def shutdown(self, current: Driver.Current) -> None:
        # Shutdown this replica by connecting to the IceBox service manager with iceboxadmin
        endpoint = current.getTestEndpoint(self.portnum + self.replica * 4 + 3)
        props = {"IceBoxAdmin.ServiceManager.Proxy": "IceBox/admin -f IceBox.ServiceManager:" + endpoint}
        IceBoxAdmin().run(current, props=props, args=["shutdown"])


# IceStormProcess is mixed into the Client and Server specializations below rather than deriving
# from Process, so that each can pick its own parent properties. Giving it Process as a base for
# type checking only lets it use the Process API without changing the runtime method resolution
# order.
_IceStormProcessBase = Process if TYPE_CHECKING else object


class IceStormProcess(_IceStormProcessBase):
    def getParentProps(self, current: Driver.Current) -> Props:
        # Bound to the parent Client or Server getProps by each specialization below.
        raise NotImplementedError()

    def __init__(self, instanceName: str | None = None, instance: IceStorm | None = None):
        self.instanceName = instanceName
        self.instance = instance

    def getProps(self, current: Driver.Current) -> Props:
        #
        # An IceStorm client is provided with the IceStormAdmin.TopicManager.Default property set
        # to the "instance" topic manager proxy if "instance" is set. Otherwise, if a single it's
        # set to the replicated topic manager if a specific "instance name" is provided or there's
        # only one IceStorm instance name deployed. If IceStorm multiple instance names are set,
        # the client is given an IceStormAdmin.<instance name> property for each instance containing
        # the replicated topic manager proxy.
        #

        props = self.getParentProps(current)
        testcase = current.testcase
        while testcase and not isinstance(testcase, IceStormTestCase):
            testcase = testcase.parent
        assert isinstance(testcase, IceStormTestCase)
        if self.instance:
            props["IceStormAdmin.TopicManager.Default"] = self.instance.getTopicManager(current)
        else:
            instanceNames = [self.instanceName] if self.instanceName else testcase.getInstanceNames()
            if len(instanceNames) == 1:
                props["IceStormAdmin.TopicManager.Default"] = testcase.getTopicManager(current, instanceNames[0])
            else:
                for name in instanceNames:
                    props["IceStormAdmin.TopicManager.{0}".format(name)] = testcase.getTopicManager(current, name)
        return props


class IceStormAdmin(ProcessFromBinDir, ProcessIsReleaseOnly, IceStormProcess, Client):
    def __init__(
        self,
        instanceName: str | None = None,
        instance: IceStorm | None = None,
        *args: Any,
        **kargs: Any,
    ):
        Client.__init__(
            self,
            exe="icestormadmin",
            mapping=Mapping.getByName("cpp"),
            *args,
            **kargs,
        )
        IceStormProcess.__init__(self, instanceName, instance)

    def getExe(self, current: Driver.Current) -> str:
        # This used to skip the "_32" suffix when testing against a binary distribution, but binary
        # distribution testing (ICE_BIN_DIST) was removed in #3442.
        assert self.exe is not None
        return self.exe + "_32" if current.config.buildPlatform == "ppc" else self.exe

    def getParentProps(self, current: Driver.Current) -> Props:
        return Client.getProps(self, current)


class Subscriber(IceStormProcess, Server):
    processType = "subscriber"

    def __init__(
        self,
        instanceName: str | None = None,
        instance: IceStorm | None = None,
        *args: Any,
        **kargs: Any,
    ):
        Server.__init__(self, *args, **kargs)
        IceStormProcess.__init__(self, instanceName, instance)

    def getParentProps(self, current: Driver.Current) -> Props:
        return Server.getProps(self, current)


class Publisher(IceStormProcess, Client):
    processType = "publisher"

    def __init__(
        self,
        instanceName: str | None = None,
        instance: IceStorm | None = None,
        *args: Any,
        **kargs: Any,
    ):
        Client.__init__(self, *args, **kargs)
        IceStormProcess.__init__(self, instanceName, instance)

    def getParentProps(self, current: Driver.Current) -> Props:
        return Client.getProps(self, current)


class IceStormTestCase(TestCase):
    def __init__(self, name: str, icestorm: IceStorm | list[IceStorm], *args: Any, **kargs: Any):
        TestCase.__init__(self, name, *args, **kargs)
        self.icestorm = icestorm if isinstance(icestorm, list) else [icestorm]

    def init(self, mapping: Mapping, testsuite: TestSuite) -> None:
        TestCase.init(self, mapping, testsuite)

        #
        # Add icestorm servers at the beginning of the server list, IceStorm needs to be
        # started first!
        #
        self.servers = list(self.icestorm) + self.getServers()

    def runWithDriver(self, current: Driver.Current) -> None:
        current.driver.runClientServerTestCase(current)

    def startIceStorm(self, current: Driver.Current) -> None:
        for icestorm in self.icestorm:
            icestorm.start(current)

    def stopIceStorm(self, current: Driver.Current) -> None:
        self.shutdown(current)
        for icestorm in self.icestorm:
            icestorm.stop(current, True)

    def restartIceStorm(self, current: Driver.Current) -> None:
        self.stopIceStorm(current)
        self.startIceStorm(current)

    def shutdown(self, current: Driver.Current) -> None:
        for icestorm in self.icestorm:
            icestorm.shutdown(current)

    def runadmin(
        self,
        current: Driver.Current,
        cmd: str,
        instanceName: str | None = None,
        instance: IceStorm | None = None,
        exitstatus: int = 0,
        quiet: bool = False,
    ) -> str:
        admin = IceStormAdmin(instanceName, instance, args=["-e", cmd], quiet=quiet)
        admin.run(current, exitstatus=exitstatus)
        return admin.getOutput(current)

    def getTopicManager(self, current: Driver.Current, instanceName: str | None = None) -> str | None:
        if not instanceName:
            # Return the topic manager proxy from the first IceStorm server
            return self.icestorm[0].getReplicatedTopicManager(current)

        #
        # Otherwise, search for an IceStorm server with the given instance
        # name and return its replicated topic manager proxy
        #
        for s in self.icestorm:
            if s.getInstanceName() == instanceName:
                return s.getReplicatedTopicManager(current)
        return None

    def getInstanceNames(self) -> list[str]:
        # Return the different IceStorm instance names deployed with this
        # test case
        names = set()
        for s in self.icestorm:
            names.add(s.getInstanceName())
        return list(names)
