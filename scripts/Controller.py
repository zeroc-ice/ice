#!/usr/bin/env python3
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

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
            self.config.transport = protocol

    @classmethod
    def getSupportedArgs(self):
        return ("", ["clean", "id=", "endpoints="])

    @classmethod
    def usage(self):
        print("")
        print("Controller driver options:")
        print("--id=<identity>       The identify of the controller object.")
        print("--endpoints=<endpts>  The endpoints to listen on.")
        print("--clean               Remove trust settings (macOS).")

    def __init__(self, options, *args, **kargs):
        Driver.__init__(self, options, *args, **kargs)
        self.id = "controller"
        self.endpoints = ""
        self.clean = False
        parseOptions(self, options, { "clean" : "clean" })

        if not self.endpoints:
            self.endpoints = "tcp -h {}".format(self.interface)

    def run(self, mappings, testSuiteIds):

        if isinstance(platform, Darwin):
            #
            # On macOS, we set the trust settings on the certificate to prevent
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

        self.initCommunicator()
        import Ice
        Ice.loadSlice(os.path.join(toplevel, "scripts", "Controller.ice"))
        import Test

        class TestCaseI(Test.Common.TestCase):
            def __init__(self, driver, current):
                self.driver = driver
                self.current = current
                self.serverSideRunning = False

            def startServerSide(self, config, c):
                self.updateCurrent(config)
                try:
                    self.serverSideRunning = True
                    return self.current.serverTestCase._startServerSide(self.current)
                except Exception as ex:
                    self.serverSideRunning = False
                    raise Test.Common.TestCaseFailedException(self.current.result.getOutput() + "\n" + traceback.format_exc())

            def stopServerSide(self, success, c):
                if self.serverSideRunning:
                    try:
                        self.current.serverTestCase._stopServerSide(self.current, success)
                        return self.current.result.getOutput()
                    except Exception as ex:
                        raise Test.Common.TestCaseFailedException(self.current.result.getOutput() + "\n" + str(ex))

            def runClientSide(self, host, config, c):
                self.updateCurrent(config)
                try:
                    self.current.clientTestCase._runClientSide(self.current, host)
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
                attrs = ["transport", "mx", "serialize", "compress", "ipv6", "cprops", "sprops"]
                for a in attrs:
                    v = getattr(config, a)
                    if v is not Ice.Unset:
                        if a not in self.current.config.parsedOptions:
                            self.current.config.parsedOptions.append(a)
                        setattr(self.current.config, a, v)

        class ControllerI(Test.Common.Controller):
            def __init__(self, driver):
                self.driver = driver
                self.testcase = None

            def runTestCase(self, mapping, testsuite, testcase, cross, c):
                if self.testcase:
                    try:
                        self.testcase.destroy()
                    except:
                        pass
                    self.testcase = None

                current = self.driver.getCurrent(mapping, testsuite, testcase, cross)
                prx = c.adapter.addWithUUID(TestCaseI(self.driver, current))
                self.testcase = Test.Common.TestCasePrx.uncheckedCast(c.adapter.createDirectProxy(prx.ice_getIdentity()))
                return self.testcase

            def getTestSuites(self, mapping, c):
                mapping = Mapping.getByName(mapping)
                config = self.driver.configs[mapping]
                return [str(t) for t in mapping.getTestSuites() if not mapping.filterTestSuite(t.getId(), config)]

            def getOptionOverrides(self, c):
                return Test.Common.OptionOverrides(ipv6=([False] if not self.driver.hostIPv6 else [False, True]))

        self.initCommunicator()
        self.communicator.getProperties().setProperty("ControllerAdapter.Endpoints", self.endpoints)
        self.communicator.getProperties().setProperty("ControllerAdapter.AdapterId", Ice.generateUUID())
        adapter = self.communicator.createObjectAdapter("ControllerAdapter")
        adapter.add(ControllerI(self), self.communicator.stringToIdentity(self.id))
        adapter.activate()
        self.communicator.waitForShutdown()

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

    def isInterrupted(self):
        return False

Driver.add("controller", ControllerDriver, default=True)

runTests()
