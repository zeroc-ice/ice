# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

class IniClient(Client):

    def __init__(self, iceOptions, iceProfile=None, *args, **kargs):
        Client.__init__(self, *args, **kargs)
        self.iceOptions = iceOptions
        self.iceProfile = iceProfile

    def setup(self, current):
        if self.iceProfile:
            current.createFile("ice.profiles", [
                "[%s]" % self.iceProfile,
                "ice.config=\"config.client\"",
                "ice.options=\"%s\"" % self.iceOptions,
            ])
        current.write("testing... ")

    def teardown(self, current, success):
        if success:
            current.writeln("ok")

    def getPhpArgs(self, current):
        if self.iceProfile:
            return ["-d", "ice.profiles=\"ice.profiles\""]
        else:
            return ["-d", "ice.options=\"{0}\"".format(self.iceOptions), "-d", "ice.config=\"config.client\""]

TestSuite(__name__, [
    ClientTestCase("php INI settings",
                   client=IniClient("--Ice.Trace.Network=1 --Ice.Warn.Connections=1")),
    ClientTestCase("php INI settings with profiles",
                   client=IniClient("--Ice.Trace.Network=1 --Ice.Warn.Connections=1",
                                    "Test",
                                    exe="ClientWithProfile"))
])
