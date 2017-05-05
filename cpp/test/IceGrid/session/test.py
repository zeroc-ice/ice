# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

class IceGridSessionTestCase(IceGridTestCase):

    def setupClientSide(self, current):
        IceGridTestCase.setupClientSide(self, current)
        current.mkdirs("db/node-1")

    def setupServerSide(self, current):
        self.verifier = Server(exe="verifier", waitForShutdown=False, props={
            "PermissionsVerifier.Endpoints" : "tcp -p 12002"
        })
        current.write("starting permission verifier... ")
        self.verifier.start(current)
        current.writeln("ok")

    def teardownServerSide(self, current, success):
        self.verifier.stop(current, success)
        self.verifier = None

registryProps = {
    'Ice.Warn.Dispatch' : '0',
    'IceGrid.Registry.DynamicRegistration' : True,
    'IceGrid.Registry.SessionFilters' : True,
    'IceGrid.Registry.AdminSessionFilters' : True,
    'IceGrid.Registry.PermissionsVerifier' : 'ClientPermissionsVerifier',
    'IceGrid.Registry.AdminPermissionsVerifier' : 'AdminPermissionsVerifier:tcp -p 12002',
    'IceGrid.Registry.SSLPermissionsVerifier' : 'SSLPermissionsVerifier',
    'IceGrid.Registry.AdminSSLPermissionsVerifier' : 'SSLPermissionsVerifier',
}

clientProps = lambda process, current: {
    "IceGridNodeExe" : IceGridNode().getCommandLine(current),
    "ServerDir" : current.getBuildDir("server"),
    "TestDir" : "{testdir}",
}

clientProps10 = lambda process, current: {
    "IceGridNodeExe" : IceGridNode().getCommandLine(current),
    "ServerDir" : current.getBuildDir("server"),
    "TestDir" : "{testdir}",
    "Ice.Default.EncodingVersion" : "1.0"
}

icegridregistry = [IceGridRegistryMaster(props=registryProps)]

if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(__file__,
          [ IceGridSessionTestCase("with default encoding", icegridregistry=icegridregistry,
                                   client=IceGridClient(props=clientProps)),
            IceGridSessionTestCase("with 1.0 encoding", icegridregistry=icegridregistry,
                                   client=IceGridClient(props=clientProps10))],
            runOnMainThread=True, multihost=False)
