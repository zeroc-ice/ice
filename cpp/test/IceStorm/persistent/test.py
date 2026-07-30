# Copyright (c) ZeroC, Inc.

from __future__ import annotations

from typing import Any

from IceStormUtil import IceStorm, IceStormAdmin, IceStormProcess
from Util import Client, ClientTestCase, Driver, Mapping, Props, TestCase, TestSuite

icestorm1 = IceStorm(createDb=True, cleanDb=False)
icestorm2 = IceStorm(createDb=False, cleanDb=True)


def test(value: object) -> None:
    if not value:
        raise RuntimeError("test failed")


class IceStormPersistentTestCase(TestCase):
    def __init__(self, name: str, icestorm: IceStorm, *args: Any, **kargs: Any):
        TestCase.__init__(self, name, *args, **kargs)
        self.icestorm = icestorm

    def init(self, mapping: Mapping, testsuite: TestSuite) -> None:
        TestCase.init(self, mapping, testsuite)
        self.servers = [self.icestorm]

    def runWithDriver(self, current: Driver.Current) -> None:
        current.driver.runClientServerTestCase(current)

    def teardownClientSide(self, current: Driver.Current, success: bool) -> None:
        admin = IceStormAdmin(
            instance=self.icestorm,
            quiet=True,
            args=["-e", "topics {}".format(self.icestorm.getInstanceName())],
        )
        admin.run(current, exitstatus=0)
        #
        # Ensure all topics have been restored from the storage
        #
        topics = admin.getOutput(current).split()
        test(len(topics) == 10)
        for i in range(0, 10):
            topic = topics[i]
            admin = IceStormAdmin(
                instance=self.icestorm,
                quiet=True,
                args=[
                    "-e",
                    "current {0};subscribers {1}".format(self.icestorm.getInstanceName(), topic),
                ],
            )
            admin.run(current, exitstatus=0)
            subscribers = admin.getOutput(current).split()[2:]
            test("subscriber{0}".format(i) in subscribers)
            if i > 0:
                test("IceStorm/topic.topic{0}".format(i - 1) in subscribers)

        self.icestorm.shutdown(current)


class PersistentClient(IceStormProcess, Client):
    processType = "client"

    def __init__(self, instanceName: str | None = None, instance: IceStorm | None = None, *args: Any, **kargs: Any):
        Client.__init__(self, *args, **kargs)
        IceStormProcess.__init__(self, instanceName, instance)

    def getParentProps(self, current: Driver.Current) -> Props:
        # IceStormProcess.getProps calls this to reach the Client props rather than its own entry
        # in the MRO.
        return Client.getProps(self, current)


TestSuite(
    __file__,
    [
        IceStormPersistentTestCase(
            "persistent create",
            icestorm1,
            client=ClientTestCase(client=PersistentClient(instance=icestorm1, args=["create"])),
        ),
        IceStormPersistentTestCase(
            "persistent check",
            icestorm2,
            client=ClientTestCase(client=PersistentClient(instance=icestorm2, args=["check"])),
        ),
    ],
    multihost=False,
)
