# Copyright (c) ZeroC, Inc.

from __future__ import annotations

from typing import Any

from Util import Client, ClientTestCase, Driver, TestSuite


class IniClient(Client):
    def __init__(self, iceOptions: str, iceProfile: str | None = None, *args: Any, **kargs: Any):
        Client.__init__(self, *args, **kargs)
        self.iceOptions = iceOptions
        self.iceProfile = iceProfile

    def setup(self, current: Driver.Current) -> None:
        if self.iceProfile:
            current.createFile(
                "ice.profiles",
                [
                    "[%s]" % self.iceProfile,
                    'ice.config="config.client"',
                    'ice.options="%s"' % self.iceOptions,
                ],
            )
        current.write("testing... ")

    def teardown(self, current: Driver.Current, success: bool) -> None:
        if success:
            current.writeln("ok")

    def getPhpArgs(self, current: Driver.Current) -> list[str]:
        if self.iceProfile:
            return ["-d", 'ice.profiles="ice.profiles"']
        else:
            return [
                "-d",
                'ice.options="{0}"'.format(self.iceOptions),
                "-d",
                'ice.config="config.client"',
            ]


TestSuite(
    __name__,
    [
        ClientTestCase(
            "php INI settings",
            client=IniClient("--Ice.Trace.Network=1 --Ice.Warn.Connections=1"),
        ),
        ClientTestCase(
            "php INI settings with profiles",
            client=IniClient(
                "--Ice.Trace.Network=1 --Ice.Warn.Connections=1",
                "Test",
                exe="ClientWithProfile",
            ),
        ),
    ],
)
