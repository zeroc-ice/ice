#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, time, socket

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

hostname = socket.gethostname()
fqdn = socket.getfqdn()

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

    # On SuSE distributions, 127.0.0.2 is sometime used in /etc/hosts
    # for the hostname (apparently if no network interface was found
    # when the OS was installed). However, connecting to this IP addr
    # doesn't work (even if can be "ping").
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
                (r'', r'*', r'', '', '', ''),
                [(False, 'helloA:tcp -h %s -p 12010' % fqdn),
                (False, 'helloB:tcp -h %s -p 12010' % hostname),
                (False, 'helloC:tcp -h 127.0.0.1 -p 12010'),
                (True, 'bar @ foo')], []),
            ('testing loopback only rule',
                (r'127.0.0.1 localhost', r'', r'', '', '', ''),
                [(False, 'hello:tcp -h %s -p 12010' % fqdn),
                (False, 'hello:tcp -h %s -p 12010' % hostname),
                (False, '127.0.0.1:tcp -h %s -p 12010' % hostname),
                (False, 'localhost:tcp -h %s -p 12010' % hostname),
                (False, 'localhost/127.0.0.1:tcp -h %s -p 12010' % hostname),
                (True, 'localhost:tcp -h 127.0.0.1 -p 12010'),
                (True, 'localhost/127.0.0.1:tcp -h localhost -p 12010'),
                (True, r'hello:tcp -h 127.0.0.1 -p 12010'),
                (True, r'hello/somecat:tcp -h localhost -p 12010')], []),
            ('testing domain filter rule (accept)',
                ("*" + domainname, r'', r'', '', '', ''),
                [(True, 'hello:tcp -h %s -p 12010' % fqdn),
                (False, 'hello:tcp -h %s -p 12010' % hostname)], []),
            ('testing domain filter rule (reject)',
                (r'', "*" + domainname, r'', '', '', ''),
                [(False, 'hello:tcp -h %s -p 12010' % fqdn),
                (True, 'hello:tcp -h %s -p 12010' % hostname),
                (True, 'bar:tcp -h 127.0.0.1 -p 12010')], []),
            ('testing domain filter rule (mixed)',
                ("127.0.0.1", fqdn, r'', '', '', ''),
                [(False, 'hello:tcp -h %s -p 12010:tcp -h 127.0.0.1 -p 12010' % fqdn),
                (True, 'bar:tcp -h 127.0.0.1 -p 12010')], []),
            ('testing maximum proxy length rule',
                (r'', r'', r'40', '', '', ''),
                [(True, 'hello:tcp -h 127.0.0.1 -p 12010'),
                (False, '012345678901234567890123456789012345678901234567890123456789:tcp -h 127.0.0.1 -p 12010')], []),
            ])

if len(testcases) == 0:
    print "WARNING: You are running this test with SSL disabled and the network "
    print "         configuration for this host does not permit the other tests "
    print "         to run correctly."
    sys.exit(0)
elif len(testcases) < 6:
    print "WARNING: The network configuration for this host does not permit all "
    print "         tests to run correctly, some tests have been disabled."

router = os.path.join(toplevel, "bin", "glacier2router")

def pingProgress():
    sys.stdout.write('.')
    sys.stdout.flush()

for testcase in testcases:
    description, args, attacks, xtraConfig = testcase
    acceptFilter, rejectFilter, maxEndpoints, categoryFilter, idFilter, adapterFilter = args
    testdir = os.path.join(toplevel, 'test', 'Glacier2', 'staticFiltering')

    #
    # The test client performs multiple tests during one 'run'. We could
    # use command line arguments to pass the test cases in, but a
    # configuration file is easier.
    #
    attackcfg = file(os.path.join(testdir, 'attack.cfg'), 'w')
    accepts=0
    rejects=0
    sys.stdout.write(description)

    for expect, proxy in attacks:
        if expect:
            attackcfg.write('Accept.Proxy.' + str(accepts) + '=')
            accepts +=1
        else:
            attackcfg.write('Reject.Proxy.' + str(rejects) + '=')
            rejects +=1
        attackcfg.write(proxy + '\n')

    attackcfg.close()
    pingProgress()

    hostArg = ""
    if limitedTests:
        hostArg = " --Ice.Default.Host=127.0.0.1" 


    #
    # This test causes connections to be terminated which will cause
    # warnings if we use the default test flags. So we need to define
    # our own.
    #
    commonClientOptions = " --Ice.NullHandleAbort=1 --Ice.PrintProcessId=1" + hostArg
    commonServerOptions = r' --Ice.PrintProcessId --Ice.PrintAdapterReady --Ice.NullHandleAbort=1' + \
            r' --Ice.ServerIdleTime=600 --Ice.ThreadPool.Server.Size=2 --Ice.ThreadPool.Server.SizeMax=10' + \
            r' --Glacier2.RoutingTable.MaxSize=10 ' + hostArg

    # 
    # We cannot use the TestUtil options because they use localhost as the default host which doesn't really work for
    # these tests.
    #
    clientOptions = TestUtil.clientProtocol + commonClientOptions 
    serverOptions = TestUtil.serverProtocol + commonServerOptions

    routerCmd = router + serverOptions + " --Ice.Config=%s" % os.path.join(testdir, "router.cfg") + \
          r' --Glacier2.Client.Endpoints="default -p 12347 -t 10000"' + \
          r' --Glacier2.Admin.Endpoints="tcp -h 127.0.0.1 -p 12348 -t 10000"' + \
          r' --Glacier2.CryptPasswords="' + toplevel + r'/test/Glacier2/staticFiltering/passwords"' 

    routerConfig = file(os.path.join(testdir, "router.cfg"), "w")

    routerConfig.write("Ice.Default.Locator=locator:tcp -h %s -p 12010\n" % hostname)
    routerConfig.write("Glacier2.Client.Trace.Reject=0\n")
    routerConfig.write("#\n")

    for l in xtraConfig:
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

    routerConfig.close()

    if TestUtil.debug:
        print "(" + routerCmd + ")",
    starterPipe = os.popen(routerCmd + " 2>&1")
    TestUtil.getServerPid(starterPipe)
    TestUtil.getAdapterReady(starterPipe)
    pingProgress()

    if TestUtil.protocol != "ssl":
        serverConfig = file(os.path.join(testdir, "server.cfg"), "w")
        serverOptions = ' --Ice.Config=' + os.path.join(testdir, "server.cfg") + " " + serverOptions
        serverConfig.write("BackendAdapter.Endpoints=tcp -p 12010 -t 20000\n")
        serverConfig.close()

    serverCmd = os.path.join(testdir, 'server') + serverOptions
    if TestUtil.debug:
        print "(" + serverCmd + ")",
    serverPipe = os.popen(serverCmd + " 2>&1")
    TestUtil.getServerPid(serverPipe)
    TestUtil.getAdapterReady(serverPipe)
    pingProgress()

    sys.stdout.write(' ')
    sys.stdout.flush()

    #
    # The client is responsible for reporting success or failure. A test
    # failure will result in an assertion and the test will abort.
    #
    clientCmd = os.path.join(testdir, 'client') + clientOptions + \
                " --Ice.Config=" + os.path.join(testdir, 'attack.cfg') + " "
    if TestUtil.debug:
        print "(" + clientCmd + ")",
    clientPipe = os.popen(clientCmd + " 2>&1")
    TestUtil.ignorePid(clientPipe)

    TestUtil.printOutputFromPipe(clientPipe)

    clientStatus = TestUtil.closePipe(clientPipe)
    if clientStatus:
        TestUtil.killServers()

    if clientStatus or TestUtil.serverStatus():
        sys.exit(1)
    
sys.exit(0)
