# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
        admin.expect('>>> ')
        admin.sendline("server list")
        admin.expect('>>> ')
        admin.sendline('exit')
        admin.stop(current, True)

        defaultRouterProps = { "Ice.Default.Router" : self.glacier2router.getClientProxy(current) }

        admin.start(current, props=defaultRouterProps)
        admin.expect('>>> ')
        admin.sendline("server list")
        admin.expect('>>> ')
        admin.sendline('exit')
        admin.stop(current, True)

        current.writeln("ok")

        if current.config.protocol == "ssl":

            current.write("testing login with ssl... ")

            # Direct registry connection with SSL
            admin.start(current, args=["--ssl"])
            admin.expect('>>> ')
            admin.sendline("server list")
            admin.expect('>>> ')
            admin.sendline('exit')
            admin.stop(current, True)

            # Glacier2 connection with username/password
            admin.start(current, args=["--ssl"], props=defaultRouterProps)
            admin.expect('>>> ')
            admin.sendline("server list")
            admin.expect('>>> ')
            admin.sendline('exit')
            admin.stop(current, True)

            current.writeln("ok")

        current.write("testing commands... ")
        admin.start(current)
        try:
            serverDir = current.getBuildDir("server")

            admin.expect('>>> ')
            admin.sendline('application add application.xml server.dir=%s' % serverDir)
            admin.expect('>>> ')
            admin.sendline('application list')
            admin.expect('Test')
            admin.sendline('application describe Test')
            admin.expect('application `Test\'')
            admin.expect('\{.*\}')
            admin.expect('>>> ')
            admin.sendline('application diff application.xml server.dir=%s' % serverDir)
            admin.expect('application `Test\'\n\{.*\}')
            admin.expect('>>> ')
            admin.sendline('application update application.xml server.dir=%s' % serverDir)
            admin.expect('>>> ')
            admin.sendline('application patch Test')
            admin.expect('>>> ')
            admin.sendline('server list')
            admin.expect('server')
            admin.expect('>>> ')
            admin.sendline('server describe server')
            admin.expect('server `server\'\n\{.*\}')
            admin.expect('>>> ')
            admin.sendline('server start server')
            admin.expect('>>> ')
            admin.sendline('server state server')
            admin.expect('^active \(.*\)')
            admin.expect('>>> ')
            admin.sendline('server pid server')
            admin.expect('[0-9]+')
            admin.expect('>>> ')
            admin.sendline('server properties server')
            admin.expect('>>> ')
            admin.sendline('server property server Ice.Admin.ServerId')
            admin.expect("^server")
            admin.expect('>>> ')
            admin.sendline('server patch server')
            admin.expect('>>> ')
            admin.sendline('server disable server')
            admin.expect('>>> ')
            admin.sendline('server enable server')
            admin.expect('>>> ')
            admin.sendline('adapter list')
            admin.expect('TestAdapter')
            admin.expect('>>> ')
            admin.sendline('adapter endpoints TestAdapter')
            admin.expect(['tcp', 'ssl', 'ws', 'wss'])
            admin.expect('>>> ')
            admin.sendline('object list')
            admin.expect('test')
            admin.expect('>>> ')
            admin.sendline('object describe')
            admin.expect('proxy = `.*\' type = `.*\'')
            admin.expect('>>> ')
            admin.sendline('object find Test')
            admin.expect('test')
            admin.expect('>>> ')
            admin.sendline('server stop server')
            admin.expect('>>> ')
            admin.sendline('application remove Test')
            admin.expect('>>> ')
            admin.sendline('registry list')
            admin.expect('Master')
            admin.expect('>>> ')
            admin.sendline('registry ping Master')
            admin.expect('registry is up')
            admin.expect('>>> ')
            admin.sendline('registry describe Master')
            admin.expect('registry `Master\'\n{.*}')
            admin.expect('>>> ')
            admin.sendline('node list')
            admin.expect('localnode')
            admin.expect('>>> ')
            admin.sendline('node describe localnode')
            admin.expect('node `localnode\'\n{.*}')
            admin.expect('>>> ')
            admin.sendline('node load localnode')
            admin.expect('load average.*\n')
            admin.expect('>>> ')
            admin.sendline('node ping localnode')
            admin.expect('node is up')
            admin.expect('>>> ')
            admin.sendline('exit')
            admin.stop(current, True)
            current.writeln("ok")
        except Exception as e:
            admin.stop(current, False)
            print("error: " + str(e))
            print("failed!")

routerProps= lambda process, current: {
    'Glacier2.SessionTimeout' : 5,
    'Glacier2.SessionManager' : 'TestIceGrid/AdminSessionManager',
    'Glacier2.PermissionsVerifier' : 'Glacier2/NullPermissionsVerifier',
    'Glacier2.SSLSessionManager' : 'TestIceGrid/AdminSSLSessionManager',
    'Glacier2.SSLPermissionsVerifier' : 'Glacier2/NullSSLPermissionsVerifier',
    'Ice.Default.Locator' : current.testcase.getLocator(current),
    'IceSSL.VerifyPeer' : 1
}

TestSuite(__file__, [ IceGridAdminTestCase() ], multihost=False)