#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

testcases = [ 
    ('Testing maximum endpoints rule',
     (r'', r'', r'1'),
     [(True, 'hello:tcp -h %s -p 12010' % hostname),
      (False, 'hello:tcp -h %s -p 12010:ssl -h %s -p 12011' % (hostname, hostname))], [])
    ]

if not limitedTests:
    testcases.extend([
	    ('Testing loopback only rule',
             (r'127.0.0.1 localhost', r'', r''),
             [(False, 'hello:tcp -h %s -p 12010' % fqdn),
              (False, 'hello:tcp -h %s -p 12010' % hostname),
              (False, '127.0.0.1:tcp -h %s -p 12010' % hostname),
              (False, 'localhost:tcp -h %s -p 12010' % hostname),
              (False, 'localhost/127.0.0.1:tcp -h %s -p 12010' % hostname),
              (True, 'localhost:tcp -h 127.0.0.1 -p 12010'),
              (True, 'localhost/127.0.0.1:tcp -h localhost -p 12010'),
              (True, r'hello:tcp -h 127.0.0.1 -p 12010'),
              (True, r'hello/somecat:tcp -h localhost -p 12010')], []),
	    ('Testing domain filter rule (accept)',
             ("*" + domainname, r'', r''),
             [(True, 'hello:tcp -h %s -p 12010' % fqdn),
              (False, 'hello:tcp -h %s -p 12010' % hostname)], []),
	    ('Testing domain filter rule (reject)',
             (r'', "*" + domainname, r''),
             [(False, 'hello:tcp -h %s -p 12010' % fqdn),
              (True, 'hello:tcp -h %s -p 12010' % hostname),
              (True, 'bar:tcp -h 127.0.0.1 -p 12010')], []),
            ('Testing domain filter rule (reject override)',
             ("127.0.0.1", fqdn, r''),
             [(False, 'hello:tcp -h %s -p 12010' % fqdn),
              (True, 'bar:tcp -h 127.0.0.1 -p 12010')], ["Glacier2.Filter.Address.AcceptOverride=0"]),
            ('Testing domain filter rule (accept override)',
             (fqdn, "127.0.0.1", r''),
             [(True, 'hello:tcp -h %s -p 12010' % fqdn),
              (False, 'bar:tcp -h 127.0.0.1 -p 12010')], ["Glacier2.Filter.Address.AcceptOverride=1"])
	    ])
else:
    print "Warning! This host's network configuration does not allow for all tests."
    print "Some tests have been disabled."

router = os.path.join(toplevel, "bin", "glacier2router")

def pingProgress():
    sys.stdout.write('.')
    sys.stdout.flush()

for testcase in testcases:
    description, args, attacks, xtraConfig = testcase
    acceptFilter, rejectFilter, maxEndpoints = args
    testdir = os.path.join(toplevel, 'test', 'Glacier2', 'address')

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
          r' --Glacier2.CryptPasswords="' + toplevel + r'/test/Glacier2/regex/passwords"' 

    routerConfig = file(os.path.join(testdir, "router.cfg"), "w")

    routerConfig.write("Ice.Default.Locator=locator:tcp -h %s -p 12010\n" % hostname)
    routerConfig.write("Glacier.Client.Trace.Reject=3\n")
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
	routerConfig.write("Glacier2.Filter.MaxProxyLength=%s\n" % maxEndpoints)

    routerConfig.close()

    starterPipe = os.popen(routerCmd + " 2>&1")
    TestUtil.getServerPid(starterPipe)
    TestUtil.getAdapterReady(starterPipe)
    pingProgress()

    serverCmd = os.path.join(testdir, 'server') + serverOptions + " 2>&1"
    serverPipe = os.popen(serverCmd)
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
    clientPipe = os.popen(clientCmd)
    TestUtil.getServerPid(clientPipe)
    TestUtil.printOutputFromPipe(clientPipe)

    clientStatus = TestUtil.closePipe(clientPipe)
    serverStatus = TestUtil.closePipe(serverPipe)
    if clientStatus or serverStatus:
	TestUtil.killServers()
	sys.exit(1)
    
    starterStatus = TestUtil.closePipe(starterPipe)

    if starterStatus:
	TestUtil.killServers()
	sys.exit(1)

sys.exit(0)
