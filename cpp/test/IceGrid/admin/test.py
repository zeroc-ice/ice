# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

class IceGridAdminTestCase(IceGridTestCase):

    def __init__(self):
        self.glacier2router = Glacier2Router(props=routerProps, waitForShutdown=False)
        IceGridTestCase.__init__(self, application=None, server=self.glacier2router)

    def runClientSide(self, current):

        def expect(admin, msg):
            if admin.expect(current, ['error:', msg] if isinstance(msg, str) else ['error:'] + msg) == 0:
                raise RuntimeError(admin.getOutput(current))

        current.write("testing login with username/password... ")

        admin=IceGridAdmin()

        admin.start(current)
        expect(admin, '>>> ')
        admin.sendline(current, "server list")
        expect(admin, '>>> ')
        admin.sendline(current, 'exit')
        admin.stop(current, True)

        defaultRouterProps = { "Ice.Default.Router" : self.glacier2router.getClientProxy(current) }

        admin.start(current, props=defaultRouterProps)
        expect(admin, '>>> ')
        admin.sendline(current, "server list")
        expect(admin, '>>> ')
        admin.sendline(current, 'exit')
        admin.stop(current, True)

        current.writeln("ok")

        if current.config.protocol == "ssl":

            current.write("testing login with ssl... ")

            # Direct registry connection with SSL
            admin.start(current, args=["--ssl"])
            expect(admin, '>>> ')
            admin.sendline(current, "server list")
            expect(admin, '>>> ')
            admin.sendline(current, 'exit')
            admin.stop(current, True)

            # Glacier2 connection with username/password
            admin.start(current, args=["--ssl"], props=defaultRouterProps)
            expect(admin, '>>> ')
            admin.sendline(current, "server list")
            expect(admin, '>>> ')
            admin.sendline(current, 'exit')
            admin.stop(current, True)

            current.writeln("ok")

        current.write("testing commands... ")
        admin.start(current)
        try:
            serverDir = current.getBuildDir("server")

            expect(admin, '>>> ')
            admin.sendline(current, 'application add application.xml server.dir=%s' % serverDir)
            expect(admin, '>>> ')
            admin.sendline(current, 'application list')
            expect(admin, 'Test')
            admin.sendline(current, 'application describe Test')
            expect(admin, 'application `Test\'')
            expect(admin, '\{.*\}')
            expect(admin, '>>> ')
            admin.sendline(current, 'application diff application.xml server.dir=%s' % serverDir)
            expect(admin, 'application `Test\'\n\{.*\}')
            expect(admin, '>>> ')
            admin.sendline(current, 'application update application.xml server.dir=%s' % serverDir)
            expect(admin, '>>> ')
            admin.sendline(current, 'application patch Test')
            expect(admin, '>>> ')
            admin.sendline(current, 'server list')
            expect(admin, 'server')
            expect(admin, '>>> ')
            admin.sendline(current, 'server describe server')
            expect(admin, 'server `server\'\n\{.*\}')
            expect(admin, '>>> ')
            admin.sendline(current, 'server start server')
            expect(admin, '>>> ')
            admin.sendline(current, 'server state server')
            expect(admin, '^active \(.*\)')
            expect(admin, '>>> ')
            admin.sendline(current, 'server pid server')
            expect(admin, '[0-9]+')
            expect(admin, '>>> ')
            admin.sendline(current, 'server properties server')
            expect(admin, '>>> ')
            admin.sendline(current, 'server property server Ice.Admin.ServerId')
            expect(admin, "^server")
            expect(admin, '>>> ')
            admin.sendline(current, 'server patch server')
            expect(admin, '>>> ')
            admin.sendline(current, 'server disable server')
            expect(admin, '>>> ')
            admin.sendline(current, 'server enable server')
            expect(admin, '>>> ')
            admin.sendline(current, 'adapter list')
            expect(admin, 'TestAdapter')
            expect(admin, '>>> ')
            admin.sendline(current, 'adapter endpoints TestAdapter')
            expect(admin, ['tcp', 'ssl', 'ws', 'wss'])
            expect(admin, '>>> ')
            admin.sendline(current, 'object list')
            expect(admin, 'test')
            expect(admin, '>>> ')
            admin.sendline(current, 'object describe')
            expect(admin, 'proxy = `.*\' type = `.*\'')
            expect(admin, '>>> ')
            admin.sendline(current, 'object find Test')
            expect(admin, 'test')
            expect(admin, '>>> ')
            admin.sendline(current, 'server stop server')
            expect(admin, '>>> ')
            admin.sendline(current, 'application remove Test')
            expect(admin, '>>> ')
            admin.sendline(current, 'registry list')
            expect(admin, 'Master')
            expect(admin, '>>> ')
            admin.sendline(current, 'registry ping Master')
            expect(admin, 'registry is up')
            expect(admin, '>>> ')
            admin.sendline(current, 'registry describe Master')
            expect(admin, 'registry `Master\'\n{.*}')
            expect(admin, '>>> ')
            admin.sendline(current, 'node list')
            expect(admin, 'localnode')
            expect(admin, '>>> ')
            admin.sendline(current, 'node describe localnode')
            expect(admin, 'node `localnode\'\n{.*}')
            expect(admin, '>>> ')
            admin.sendline(current, 'node load localnode')
            expect(admin, 'load average.*\n')
            expect(admin, '>>> ')
            admin.sendline(current, 'node ping localnode')
            expect(admin, 'node is up')
            expect(admin, '>>> ')
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

if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(__file__, [ IceGridAdminTestCase() ], multihost=False)
