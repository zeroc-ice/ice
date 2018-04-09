# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

class IceGridAdminTestCase(IceGridTestCase):

    def __init__(self):
        self.glacier2router = Glacier2Router(props=routerProps, waitForShutdown=False)
        IceGridTestCase.__init__(self, application=None, server=self.glacier2router)

    def runClientSide(self, current):

        current.write("testing login with username/password... ")

        admin=IceGridAdmin()

        admin.start(current)
        admin.expect(current, '>>> ')
        admin.sendline(current, "server list")
        admin.expect(current, '>>> ')
        admin.sendline(current, 'exit')
        admin.stop(current, True)

        defaultRouterProps = { "Ice.Default.Router" : self.glacier2router.getClientProxy(current) }

        admin.start(current, props=defaultRouterProps)
        admin.expect(current, '>>> ')
        admin.sendline(current, "server list")
        admin.expect(current, '>>> ')
        admin.sendline(current, 'exit')
        admin.stop(current, True)

        current.writeln("ok")

        if current.config.protocol == "ssl":

            current.write("testing login with ssl... ")

            # Direct registry connection with SSL
            admin.start(current, args=["--ssl"])
            admin.expect(current, '>>> ')
            admin.sendline(current, "server list")
            admin.expect(current, '>>> ')
            admin.sendline(current, 'exit')
            admin.stop(current, True)

            # Glacier2 connection with username/password
            admin.start(current, args=["--ssl"], props=defaultRouterProps)
            admin.expect(current, '>>> ')
            admin.sendline(current, "server list")
            admin.expect(current, '>>> ')
            admin.sendline(current, 'exit')
            admin.stop(current, True)

            current.writeln("ok")

        current.write("testing commands... ")
        admin.start(current)
        try:
            serverDir = current.getBuildDir("server")

            admin.expect(current, '>>> ')
            admin.sendline(current, 'application add application.xml server.dir=%s' % serverDir)
            admin.expect(current, '>>> ')
            admin.sendline(current, 'application list')
            admin.expect(current, 'Test')
            admin.sendline(current, 'application describe Test')
            admin.expect(current, 'application `Test\'')
            admin.expect(current, '\{.*\}')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'application diff application.xml server.dir=%s' % serverDir)
            admin.expect(current, 'application `Test\'\n\{.*\}')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'application update application.xml server.dir=%s' % serverDir)
            admin.expect(current, '>>> ')
            admin.sendline(current, 'application patch Test')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'server list')
            admin.expect(current, 'server')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'server describe server')
            admin.expect(current, 'server `server\'\n\{.*\}')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'server start server')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'server state server')
            admin.expect(current, '^active \(.*\)')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'server pid server')
            admin.expect(current, '[0-9]+')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'server properties server')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'server property server Ice.Admin.ServerId')
            admin.expect(current, "^server")
            admin.expect(current, '>>> ')
            admin.sendline(current, 'server patch server')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'server disable server')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'server enable server')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'adapter list')
            admin.expect(current, 'TestAdapter')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'adapter endpoints TestAdapter')
            admin.expect(current, ['tcp', 'ssl', 'ws', 'wss'])
            admin.expect(current, '>>> ')
            admin.sendline(current, 'object list')
            admin.expect(current, 'test')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'object describe')
            admin.expect(current, 'proxy = `.*\' type = `.*\'')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'object find Test')
            admin.expect(current, 'test')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'server stop server')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'application remove Test')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'registry list')
            admin.expect(current, 'Master')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'registry ping Master')
            admin.expect(current, 'registry is up')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'registry describe Master')
            admin.expect(current, 'registry `Master\'\n{.*}')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'node list')
            admin.expect(current, 'localnode')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'node describe localnode')
            admin.expect(current, 'node `localnode\'\n{.*}')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'node load localnode')
            admin.expect(current, 'load average.*\n')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'node ping localnode')
            admin.expect(current, 'node is up')
            admin.expect(current, '>>> ')
            admin.sendline(current, 'exit')
            admin.stop(current, True)
            current.writeln("ok")
        except Exception as e:
            admin.stop(current, False)
            raise RuntimeError("failed!\n" + str(e))

routerProps= lambda process, current: {
    'Glacier2.SessionTimeout' : 5,
    'Glacier2.SessionManager' : 'TestIceGrid/AdminSessionManager',
    'Glacier2.PermissionsVerifier' : 'Glacier2/NullPermissionsVerifier',
    'Glacier2.SSLSessionManager' : 'TestIceGrid/AdminSSLSessionManager',
    'Glacier2.SSLPermissionsVerifier' : 'Glacier2/NullSSLPermissionsVerifier',
    'Ice.Default.Locator' : current.testcase.getLocator(current),
    'IceSSL.VerifyPeer' : 1
}

if os.getuid() != 0:
    TestSuite(__file__, [ IceGridAdminTestCase() ], multihost=False)
