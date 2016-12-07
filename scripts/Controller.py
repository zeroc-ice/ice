#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys
from Util import *

class ControllerDriver(Driver):

    class Current(Driver.Current):

        def __init__(self, driver, testsuite, testcase, cross, protocol, host, args):
            Driver.Current.__init__(self, driver, testsuite, Result(testsuite, driver.debug))
            self.testcase = testcase
            self.serverTestCase = self.testcase.getServerTestCase(cross)
            self.clientTestCase = self.testcase.getClientTestCase()
            self.cross = cross
            self.host = host
            self.args = args
            self.config.protocol = protocol

    @classmethod
    def getOptions(self):
        return ("", ["clean", "id=", "endpoints=", "interface="])

    @classmethod
    def usage(self):
        print("")
        print("Controller driver options:")
        print("--id=<identity>       The identify of the controller object.")
        print("--endpoints=<endpts>  The endpoints to listen on.")
        print("--interface=<IP>      The interface to use to discover the controller.")
        print("--clean               Remove trust settings (OS X).")

    def __init__(self, options, *args, **kargs):
        Driver.__init__(self, options, *args, **kargs)
        self.id = "controller"
        self.endpoints = "tcp"
        self.interface = ""
        self.clean = False
        parseOptions(self, options, { "clean" : "clean" })

    def run(self, mappings, testSuiteIds):

        if isinstance(platform, Darwin):
            #
            # On OS X, we set the trust settings on the certificate to prevent
            # the Web browsers from prompting the user about the untrusted
            # certificate. Some browsers such as Chrome don't provide the
            # option to set this trust settings.
            #
            serverCert = os.path.join(toplevel, "certs", "server.pem")
            if self.clean:
                if os.system("security verify-cert -c " + serverCert + " >& /dev/null") == 0:
                    sys.stdout.write("removing trust settings for the HTTP server certificate... ")
                    sys.stdout.flush()
                    if os.system("security remove-trusted-cert " + serverCert) != 0:
                        print("\nerror: couldn't remove trust settings for the HTTP server certificate")
                    else:
                        print("ok")
                else:
                    print("trust settings already removed")
                return
            else:
                if os.system("security verify-cert -c " + serverCert + " >& /dev/null") != 0:
                    sys.stdout.write("adding trust settings for the HTTP server certificate... ")
                    sys.stdout.flush()
                    if os.system("security add-trusted-cert -r trustAsRoot " + serverCert) != 0:
                        print("error: couldn't add trust settings for the HTTP server certificate")
                    print("ok")
                    print("run " + sys.argv[0] + " --clean to remove the trust setting")

        import Ice
        Ice.loadSlice(os.path.join(toplevel, "scripts", "Controller.ice"))
        import Test

        class ServerI(Test.Common.Server):
            def __init__(self, driver, current):
                self.driver = driver
                self.current = current

            def waitForServer(self, c):
                if not self.current:
                    raise Test.Common.ServerFailedException("couldn't find test suite")

                try:
                    self.current.serverTestCase._startServerSide(self.current)
                except Exception as ex:
                    raise Test.Common.ServerFailedException(str(ex))

            def waitTestSuccess(self, c):
                c.adapter.remove(c.id)
                if self.current:
                    self.current.serverTestCase._stopServerSide(self.current, True)

            def terminate(self, c):
                c.adapter.remove(c.id)
                if self.current:
                    self.current.serverTestCase._stopServerSide(self.current, False)

        class TestCaseI(Test.Common.TestCase):
            def __init__(self, driver, current):
                self.driver = driver
                self.current = current
                self.serverSideRunning = False

            def startServerSide(self, config, c):
                self.updateCurrent(config)

                # Depending on the configuration, either use an IPv4, IPv6 or BT address for Ice.Default.Host
                if self.current.config.ipv6:
                    if not self.driver.hostIPv6:
                        raise Test.Common.TestCaseFailedException("no IPv6 address set with --host-ipv6")
                    self.current.host = self.driver.hostIPv6
                elif self.current.config.protocol == "bt":
                    if not self.driver.hostBT:
                        raise Test.Common.TestCaseFailedException("no Bluetooth address set with --host-bt")
                    self.current.host = self.driver.hostBT
                else:
                    self.current.host = self.driver.host if self.driver.host else self.driver.interface

                try:
                    self.current.serverTestCase._startServerSide(self.current)
                    self.serverSideRunning = True
                    return self.current.host
                except Exception as ex:
                    raise Test.Common.TestCaseFailedException(self.current.result.getOutput() + "\n" + str(ex))

            def stopServerSide(self, success, c):
                if self.serverSideRunning:
                    try:
                        self.current.serverTestCase._stopServerSide(self.current, success)
                        return self.current.result.getOutput()
                    except Exception as ex:
                        raise Test.Common.TestCaseFailedException(self.current.result.getOutput() + "\n" + str(ex))

            def runClientSide(self, host, config, c):
                self.updateCurrent(config)
                if host:
                    current.host = host
                try:
                    self.current.clientTestCase._runClientSide(self.current)
                    return self.current.result.getOutput()
                except Exception as ex:
                    raise Test.Common.TestCaseFailedException(self.current.result.getOutput() + "\n" + str(ex))

            def destroy(self, c):
                if self.serverSideRunning:
                    try:
                        self.current.serverTestCase._stopServerSide(self.current, False)
                    except:
                        pass
                c.adapter.remove(c.id)

            def updateCurrent(self, config):
                attrs = ["protocol", "mx", "serialize", "compress", "ipv6"]
                for a in attrs:
                    v = getattr(config, a)
                    if v is not Ice.Unset:
                        setattr(self.current.config, a, v)

        class ControllerI(Test.Common.Controller):
            def __init__(self, driver):
                self.driver = driver
                self.server = None
                self.testcase = None

            def runServer(self, mapping, testsuite, protocol, host, winrt, testcase, args, c):
                self.terminate()
                try:
                    current = self.driver.getCurrent(mapping, testsuite, testcase, "", protocol, host, args)
                except Test.Common.TestCaseNotExistException:
                    current = None
                self.server = Test.Common.ServerPrx.uncheckedCast(c.adapter.addWithUUID(ServerI(self.driver, current)))
                return self.server

            def runTestCase(self, mapping, testsuite, testcase, cross, c):
                self.terminate()
                current = self.driver.getCurrent(mapping, testsuite, testcase, cross)
                self.testcase = Test.Common.TestCasePrx.uncheckedCast(c.adapter.addWithUUID(TestCaseI(self.driver, current)))
                return self.testcase

            def getOptionOverrides(self, c):
                return Test.Common.OptionOverrides(ipv6=([False] if not self.driver.hostIPv6 else [False, True]))

            def terminate(self):
                if self.server:
                    try:
                        self.server.terminate()
                    except:
                        pass
                    self.server = None

                if self.testcase:
                    try:
                        self.testcase.destroy()
                    except:
                        pass
                    self.testcase = None

        initData = Ice.InitializationData()
        initData.properties = Ice.createProperties();
        initData.properties.setProperty("Ice.Plugin.IceSSL", "IceSSL:createIceSSL")
        initData.properties.setProperty("IceSSL.DefaultDir", os.path.join(toplevel, "certs"))
        initData.properties.setProperty("IceSSL.CertFile", "server.p12")
        initData.properties.setProperty("IceSSL.Password", "password")
        initData.properties.setProperty("IceSSL.Keychain", "test.keychain")
        initData.properties.setProperty("IceSSL.KeychainPassword", "password")
        initData.properties.setProperty("IceSSL.VerifyPeer", "0");
        initData.properties.setProperty("Ice.ThreadPool.Server.SizeMax", "10")
        initData.properties.setProperty("Ice.Plugin.IceDiscovery", "IceDiscovery:createIceDiscovery")
        initData.properties.setProperty("IceDiscovery.DomainId", "TestController")
        if self.interface:
            initData.properties.setProperty("IceDiscovery.Interface", self.interface)
            initData.properties.setProperty("Ice.Default.Host", self.interface)
        #initData.properties.setProperty("Ice.Trace.Network", "3")
        #initData.properties.setProperty("Ice.Trace.Protocol", "1")
        initData.properties.setProperty("ControllerAdapter.Endpoints", self.endpoints)
        initData.properties.setProperty("ControllerAdapter.AdapterId", Ice.generateUUID())

        communicator = Ice.initialize(initData)
        ctrlCHandler = Ice.CtrlCHandler()
        ctrlCHandler.setCallback(lambda sig: communicator.shutdown())
        try:
            adapter = communicator.createObjectAdapter("ControllerAdapter")
            adapter.add(ControllerI(self), communicator.stringToIdentity(self.id))
            adapter.activate()
            communicator.waitForShutdown()
        finally:
            communicator.destroy()
            ctrlCHandler.destroy()

    def getCurrent(self, mapping, testsuite, testcase, cross, protocol=None, host=None, args=[]):
        import Test
        mapping = Mapping.getByName(mapping)
        if not mapping:
            raise Test.Common.TestCaseNotExistException("unknown mapping {0}".format(mapping))

        if cross:
            cross = Mapping.getByName(cross)
            if not cross:
                raise Test.Common.TestCaseNotExistException("unknown mapping {0} for cross testing".format(cross))

        ts = mapping.findTestSuite(testsuite)
        if not ts:
            raise Test.Common.TestCaseNotExistException("unknown testsuite {0}".format(testsuite))

        tc = ts.findTestCase("server" if ts.getId() == "Ice/echo" else (testcase or "client/server"))
        if not tc or not tc.getServerTestCase():
            raise Test.Common.TestCaseNotExistException("unknown testcase {0}".format(testcase))

        return ControllerDriver.Current(self, ts, tc, cross, protocol, host, args)

    def getProps(self, process, current):
        props = Driver.getProps(self, process, current)
        if isinstance(process, IceProcess) and current.host:
            props["Ice.Default.Host"] = current.host
        return props

    def getArgs(self, process, current):
        return current.args

    def isWorkerThread(self):
        return True

Driver.add("controller", ControllerDriver, default=True)

runTests()
