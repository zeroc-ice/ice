# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

class Glacier2StaticFilteringTestCase(ClientServerTestCase):

    def __init__(self, testcase, hostname):
        self.hostname = hostname
        description, self.tcArgs, self.attacks, self.xtraConfig = testcase

        clientProps = { "Ice.Config" : "{testdir}/client.cfg", "Ice.Warn.Connections" : 0 }
        serverProps = { "Ice.Config" : "{testdir}/server.cfg", "Ice.Warn.Connections" : 0  }
        routerProps = { "Ice.Config" : "{testdir}/router.cfg", "Glacier2.RoutingTable.MaxSize" : 10 }

        # Override the server/router default host property, we don't want to use the loopback
        serverProps["Ice.Default.Host"] = ""
        routerProps["Ice.Default.Host"] = ""

        ClientServerTestCase.__init__(self,
                                      description,
                                      desc=description,
                                      servers=[Glacier2Router(props=routerProps), Server(props=serverProps)],
                                      client=Client(props=clientProps))

    def setupClientSide(self, current):
        current.write("testing {0}... ".format(self))

    def setupServerSide(self, current):
        acceptFilter, rejectFilter, maxEndpoints, categoryFilter, idFilter, adapterFilter = self.tcArgs

        #
        # The test client performs multiple tests during one 'run'. We could
        # use command line arguments to pass the test cases in, but a
        # configuration file is easier.
        #
        with open(os.path.join(self.getTestSuite().getPath(), 'client.cfg'), 'w') as clientConfig:
            accepts=0
            rejects=0
            for expect, proxy in self.attacks:
                if expect:
                    clientConfig.write('Accept.Proxy.' + str(accepts) + '=')
                    accepts +=1
                else:
                    clientConfig.write('Reject.Proxy.' + str(rejects) + '=')
                    rejects +=1
                clientConfig.write(proxy + '\n')

        with open(os.path.join(self.getTestSuite().getPath(), 'server.cfg'), 'w') as serverConfig:
            if current.config.protocol != "ssl":
                serverConfig.write("BackendAdapter.Endpoints=tcp -p 12010\n")

        with open(os.path.join(self.getTestSuite().getPath(), "router.cfg"), "w") as routerConfig:
            routerConfig.write("Ice.Default.Locator=locator:tcp -h %s -p 12010\n" % self.hostname)
            routerConfig.write("Glacier2.Client.Trace.Reject=0\n")
            routerConfig.write("#\n")

            for l in self.xtraConfig:
                routerConfig.write(l)
                routerConfig.write("\n")

            #
            # We configure the AddProxy constraints as a configuration file.
            # Regular expression syntax can easily confuse the command line.
            #
            if not len(acceptFilter) == 0:
                routerConfig.write("Glacier2.Filter.Address.Accept=%s\n" % acceptFilter)
            if not len(rejectFilter) == 0:
                routerConfig.write("Glacier2.Filter.Address.Reject=%s\n" % rejectFilter)
            if not len(maxEndpoints) == 0:
                routerConfig.write("Glacier2.Filter.ProxySizeMax=%s\n" % maxEndpoints)
            if not len(categoryFilter) == 0:
                routerConfig.write("Glacier2.Filter.Category.Accept=%s\n" % categoryFilter)
            if not len(idFilter) == 0:
                routerConfig.write("Glacier2.Filter.Identity.Accept=%s\n" % idFilter)
            if not len(adapterFilter) == 0:
                routerConfig.write("Glacier2.Filter.AdapterId.Accept=%s\n" % adapterFilter)

class Glacier2StaticFilteringTestSuite(Glacier2TestSuite):

    def setup(self, current):

        Glacier2TestSuite.setup(self, current)

        import socket

        hostname = socket.gethostname().lower()
        fqdn = socket.getfqdn().lower()

        limitedTests = False

        #
        # Try and figure out what tests are reasonable with this host's
        # configuration.
        #
        if fqdn.endswith("localdomain") or fqdn.endswith("local") or fqdn.endswith("domain"):
            #
            # No real configured domain name, this means that anything that
            # requires a domain name isn't likely going to work. Furthermore, it
            # might be the case that the hostname contains this suffix, so we
            # should just toss it and pretend that there is no 'hostname'
            # configured for this box.
            #
            hostname = "127.0.0.1"
            fqdn = ""
            domainname = ""
            limitedTests = True
        elif hostname.startswith("localhost"):
            #
            # No configured host name (and possibly no domain name), minimal
            # tests.
            #
            hostname = "127.0.0.1"
            fqdn = ""
            domainname = ""
            limitedTests = True
        elif fqdn.find(".") == -1:
            #
            # No real configured domain.
            #
            hostname = "127.0.0.1"
            fqdn = ""
            domainname = ""
            limitedTests = True
        else:
            dot = fqdn.find(".")
            domainname = fqdn[dot+1:]
            #
            # Some Python installs are going to return a FQDN for gethostname().
            # This invalidates the tests that need to differentiate between the
            # hostname and the FQDN. If these tests fail because of a DNS error,
            # it may be that an alias is not present for the host partition of
            # the FQDN.
            #
            if fqdn == hostname:
                hostname = hostname[:dot]
            if domainname == "":
                limitedTests = True
        try:
            testaddr1 = socket.gethostbyname(fqdn)
            testaddr2 = socket.gethostbyname(hostname)

            # On SUSE distributions, 127.0.0.2 is sometimes used in /etc/hosts
            # for the hostname (apparently if no network interface was found
            # when the OS was installed). However, connecting to this IP address
            # doesn't work (even if it can be pinged).
            if testaddr1 == "127.0.0.2" or testaddr2 == "127.0.0.2":
                limitedTests = True
                hostname = "127.0.0.1"
                fqdn = ""
                domainname = ""
        except:
            limitedTests = True
            hostname = "127.0.0.1"
            fqdn = ""
            domainname = ""

        testcases = [
                ('testing category filter',
                        ('', '', '', 'foo "a cat with spaces"', '', ''),
                        [(True, 'foo/helloA:tcp -h 127.0.0.1 -p 12010'),
                        (True, '"a cat with spaces/helloB":tcp -h 127.0.0.1 -p 12010'),
                        (False, 'nocat/helloC:tcp -h 127.0.0.1 -p 12010'),
                        (False, 'cat/helloD:tcp -h 127.0.0.1 -p 12010')], []),
                ('testing adapter id filter',
                        ('', '*', '', '', '', 'foo "an adapter with spaces"'),
                        [(False, 'foo/helloA:tcp -h 127.0.0.1 -p 12010'),
                        (False, '"a cat with spaces/helloB":tcp -h 127.0.0.1 -p 12010'),
                        (False, 'nocat/helloC:tcp -h 127.0.0.1 -p 12010'),
                        (False, 'cat/helloD:tcp -h 127.0.0.1 -p 12010'),
                        (False, 'helloE @ bar'),
                        (True, 'helloF1 @ "an adapter with spaces"'),
                        (True, 'helloF @ foo')], []),
                ('test identity filters',
                        ('', '', '', '', 'myident cata/fooa "a funny id/that might mess it up"', ''),
                        [(False, '"a cat with spaces/helloB":tcp -h 127.0.0.1 -p 12010'),
                        (False, 'nocat/helloC:tcp -h 127.0.0.1 -p 12010'),
                        (False, 'cat/helloD:tcp -h 127.0.0.1 -p 12010'),
                        (False, 'baz/myident:tcp -h 127.0.0.1 -p 12010'),
                        (False, 'cata/foo:tcp -h 127.0.0.1 -p 12010'),
                        (False, 'cat/fooa:tcp -h 127.0.0.1 -p 12010'),
                        (True, 'myident:tcp -h 127.0.0.1 -p 12010'),
                        (True, 'cata/fooa:tcp -h 127.0.0.1 -p 12010'),
                        (True, '"a funny id/that might mess it up":tcp -h 127.0.0.1 -p 12010')], []),
                ('test mixing filters',
                        ('', '', '', 'mycat "a sec cat"', 'myident cata/fooa "a funny id/that might mess it up" "a\\"nother"',
                            'myadapter'),
                        [(False, '"a cat with spaces/helloB":tcp -h 127.0.0.1 -p 12010'),
                        (False, 'nocat/helloC:tcp -h 127.0.0.1 -p 12010'),
                        (False, 'cat/helloD:tcp -h 127.0.0.1 -p 12010'),
                        (False, 'baz/myident:tcp -h 127.0.0.1 -p 12010'),
                        (False, 'cata/foo:tcp -h 127.0.0.1 -p 12010'),
                        (False, 'cat/fooa:tcp -h 127.0.0.1 -p 12010'),
                        (True, 'mycat/fooa:tcp -h 127.0.0.1 -p 12010'),
                        (True, '"a sec cat/fooa":tcp -h 127.0.0.1 -p 12010'),
                        (True, 'mycat/foo @ jimbo'),
                        (False, 'hiscatA @ jimbo'),
                        (True, 'hiscat @ myadapter'),
                        (True, 'a\"nother @ jimbo'),
                        (True, 'myident:tcp -h 127.0.0.1 -p 12010'),
                        (True, 'cata/fooa:tcp -h 127.0.0.1 -p 12010'),
                        (True, '"a funny id/that might mess it up":tcp -h 127.0.0.1 -p 12010')], []),
                ('test mixing filters (indirect only)',
                        ('', '*', '', 'mycat "a sec cat"', 'myident cata/fooa "a funny id/that might mess it up"', 'myadapter'),
                        [(False, '"a cat with spaces/helloB":tcp -h 127.0.0.1 -p 12010'),
                        (False, 'nocat/helloC:tcp -h 127.0.0.1 -p 12010'),
                        (False, 'cat/helloD:tcp -h 127.0.0.1 -p 12010'),
                        (False, 'baz/myident:tcp -h 127.0.0.1 -p 12010'),
                        (False, 'cata/foo:tcp -h 127.0.0.1 -p 12010'),
                        (False, 'cat/fooa:tcp -h 127.0.0.1 -p 12010'),
                        (False, 'mycat/fooa:tcp -h 127.0.0.1 -p 12010'),
                        (False, '"a sec cat/fooa":tcp -h 127.0.0.1 -p 12010'),
                        (True, 'mycat/foo @ jimbo'),
                        (False, 'hiscatA @ jimbo'),
                        (True, 'hiscat @ myadapter'),
                        (False, 'myident:tcp -h 127.0.0.1 -p 12010'),
                        (False, 'cata/fooa:tcp -h 127.0.0.1 -p 12010'),
                        (True, '"a funny id/that might mess it up" @ myadapter'),
                        (False, '"a funny id/that might mess it up":tcp -h 127.0.0.1 -p 12010')], []),
                ]

        if not limitedTests:
            testcases.extend([
                    ('testing reject all',
                        ('', '*', '', '', '', ''),
                        [(False, 'helloA:tcp -h %s -p 12010' % fqdn),
                        (False, 'helloB:tcp -h %s -p 12010' % hostname),
                        (False, 'helloC:tcp -h 127.0.0.1 -p 12010'),
                        (True, 'bar @ foo')], []),
                    ('testing loopback only rule',
                        ('127.0.0.1 localhost', '', '', '', '', ''),
                        [(False, 'hello:tcp -h %s -p 12010' % fqdn),
                        (False, 'hello:tcp -h %s -p 12010' % hostname),
                        (False, '127.0.0.1:tcp -h %s -p 12010' % hostname),
                        (False, 'localhost:tcp -h %s -p 12010' % hostname),
                        (False, 'localhost/127.0.0.1:tcp -h %s -p 12010' % hostname),
                        (True, 'localhost:tcp -h 127.0.0.1 -p 12010'),
                        (True, 'localhost/127.0.0.1:tcp -h localhost -p 12010'),
                        (True, 'hello:tcp -h 127.0.0.1 -p 12010'),
                        (True, 'hello/somecat:tcp -h localhost -p 12010')], []),
                    ('testing port filter rule',
                        ('127.0.0.1:12010 localhost:12010', '', '', '', '', ''),
                        [(False, 'hello1:tcp -h 127.0.0.1 -p 12011'),
                         (False, 'hello2:tcp -h localhost -p 12011'),
                         (False, 'hello5:tcp -h %s -p 12010' % hostname),
                         (True, 'hello3:tcp -h 127.0.0.1 -p 12010'),
                         (True, 'hello4:tcp -h localhost -p 12010')], []),
                    ('testing reject port filter rule',
                        ('', '127.0.0.1:[0-12009,12011-65535] localhost:[0-12009,12011-65535]', '', '', '', ''),
                        [(False, 'hello1:tcp -h 127.0.0.1 -p 12011'),
                         (False, 'hello2:tcp -h localhost -p 12011'),
                         (True, 'hello5:tcp -h %s -p 12010' % hostname),
                         (True, 'hello3:tcp -h 127.0.0.1 -p 12010'),
                         (True, 'hello4:tcp -h localhost -p 12010')], []),
                    ('testing port filter rule with wildcard address rule',
                        ('*:12010', '', '', '', '', ''),
                        [(False, 'hello1:tcp -h 127.0.0.1 -p 12011'),
                         (False, 'hello2:tcp -h localhost -p 12011'),
                         (True, 'hello5:tcp -h %s -p 12010' % hostname),
                         (True, 'hello3:tcp -h 127.0.0.1 -p 12010'),
                         (True, 'hello4:tcp -h localhost -p 12010')], []),
                    ('testing domain filter rule (accept)',
                        ("*" + domainname, '', '', '', '', ''),
                        [(True, 'hello:tcp -h %s -p 12010' % fqdn),
                        (False, 'hello:tcp -h %s -p 12010' % hostname)], []),
                    ('testing domain filter rule (reject)',
                        ('', "*" + domainname, '', '', '', ''),
                        [(False, 'hello:tcp -h %s -p 12010' % fqdn),
                        (True, 'hello:tcp -h %s -p 12010' % hostname),
                        (True, 'bar:tcp -h 127.0.0.1 -p 12010')], []),
                    ('testing domain filter rule (mixed)',
                        ("127.0.0.1", fqdn, '', '', '', ''),
                        [(False, 'hello:tcp -h %s -p 12010:tcp -h 127.0.0.1 -p 12010' % fqdn),
                        (True, 'bar:tcp -h 127.0.0.1 -p 12010')], []),
                    ('testing maximum proxy length rule',
                        ('', '', '53', '', '', ''),
                        [(True, 'hello:tcp -h 127.0.0.1 -p 12010 -t infinite'),
                        (False, '012345678901234567890123456789012345678901234567890123456789:tcp -h 127.0.0.1 -p 12010')], []),
                    ])

        if len(testcases) == 0:
            current.writeln("WARNING: You are running this test with SSL disabled and the network ")
            current.writeln("         configuration for this host does not permit the other tests ")
            current.writeln("         to run correctly.")
        elif len(testcases) < 6:
            current.writeln("WARNING: The network configuration for this host does not permit all ")
            current.writeln("         tests to run correctly, some tests have been disabled.")

        self.testcases = {}
        for testcase in testcases:
            self.addTestCase(Glacier2StaticFilteringTestCase(testcase, hostname))

Glacier2StaticFilteringTestSuite(__name__, testcases=[], runOnMainThread=True, options={ "ipv6" : [False] }, multihost=False)
