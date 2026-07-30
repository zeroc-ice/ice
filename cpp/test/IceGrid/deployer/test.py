# Copyright (c) ZeroC, Inc.


from __future__ import annotations

import os

from IceGridUtil import IceGridClient, IceGridNode, IceGridTestCase
from Util import Driver, Process, Props, TestSuite, Windows, platform


def clientProps(process: Process, current: Driver.Current) -> Props:
    return {"TestDir": current.getBuildDir("server")}


if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(
        __file__,
        [
            IceGridTestCase(
                "without targets",
                icegridnode=IceGridNode(envs={"MY_FOO": 12}),  # noqa: F821
                client=IceGridClient(props=clientProps),
            ),
            IceGridTestCase(
                "with targets",
                icegridnode=IceGridNode(envs={"MY_FOO": 12}),
                client=IceGridClient(props=clientProps),
                targets=["moreservers", "moreservices", "moreproperties"],
            ),
        ],
        libDirs=["testservice"],
        multihost=False,
    )
