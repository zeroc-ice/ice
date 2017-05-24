# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os
from Util import *

#
# The Executor class runs testsuites on multiple worker threads.
#
class Executor:

    def __init__(self, threadlocal, workers, continueOnFailure):
        self.threadlocal = threadlocal
        self.workers = workers - 1
        self.queue = []
        self.mainThreadQueue = []
        self.queueLength = 0
        self.failure = False
        self.continueOnFailure = continueOnFailure
        self.lock = threading.Lock()

    def submit(self, testsuite, crossMappings):
        mainThreadOnly = testsuite.isMainThreadOnly() or self.workers == 0

        #
        # If the test supports workers and we are cross testing, ensure that all the cross
        # testing mappings support workers as well.
        #
        if not mainThreadOnly:
            for cross in crossMappings:
                if cross:
                    t = cross.findTestSuite(testsuite)
                    if t and t.isMainThreadOnly():
                        mainThreadOnly = True
                        break

        if mainThreadOnly:
            self.mainThreadQueue.append(testsuite)
        else:
            self.queue.append(testsuite)
        self.queueLength += 1

    def get(self, total, mainThread=False):
        with self.lock:
            if self.failure:
                return None
            queue = self.mainThreadQueue if mainThread else self.queue
            if len(queue) == 0:
                return None
            self.queueLength -= 1
            return (queue.pop(0), total - self.queueLength)

    def runTestSuites(self, driver, total, results, mainThread=False):
        while True:
            item = self.get(total, mainThread)
            if not item:
                results.put(None) # Notify the main thread that there are not more tests to run
                break

            (testsuite, index) = item
            result = Result(testsuite, not driver.isWorkerThread())
            current = LocalDriver.Current(driver, testsuite, result, index, total)
            try:
                testsuite.run(current)
            except KeyboardInterrupt:
                raise
            except:
                pass
            finally:
                current.destroy()
            results.put((result, mainThread))
            if not result.isSuccess() and not self.continueOnFailure:
                with self.lock: self.failure = True

    def runUntilCompleted(self, driver, start):
        if self.queueLength == 0:
            return []

        total = self.queueLength
        if self.workers == 0 and start > 0:
            for i in range(1, start):
                if len(self.mainThreadQueue) == 0:
                    break
                self.mainThreadQueue.pop(0)
                self.queueLength -= 1

        #
        # Worker threads dequeue and run testsuites. They push results to the results
        # queue. The thread stops when there are no more testsuite to dequeue.
        #
        resultList = []
        results = queue.Queue()
        def worker(num):
            self.threadlocal.num = num
            try:
                self.runTestSuites(driver, total, results)
            except Exception as ex:
                print("unexpected exception raised from worker thread:\n" + str(ex))
                results.put(None) # Notify the main thread that we're done

        #
        # Start the worker threads
        #
        threads=[]
        for i in range(min(self.workers, total)):
             t = threading.Thread(target=worker, args=[i])
             t.start()
             threads.append(t)

        try:
            #
            # Run the main thread testsuites.
            #
            self.runTestSuites(driver, total, results, True)

            #
            # Dequeue results and print out the testuite output for each test.
            #
            count = len(threads) + 1
            while count > 0:
                try:
                    r = results.get(timeout=1)
                    if not r:
                        count -= 1
                        continue
                except queue.Empty:
                    continue

                (result, mainThread) = r
                resultList.append(result)
                if not mainThread:
                    sys.stdout.write(result.getOutput())
        except KeyboardInterrupt:
            with self.lock:
                self.failure = True
            if threads:
                print("Terminating (waiting for worker threads to terminate)...")
            raise
        finally:
            #
            # Wait for worker threads to be done.
            #
            for t in threads:
                t.join()

        #
        # Print out remaining testsuites.
        #
        try:
            while True:
                r = results.get_nowait()
                if r:
                    (result, mainThread) = r
                    resultList.append(result)
                    if not mainThread:
                        sys.stdout.write(result.getOutput())
        except queue.Empty:
            pass

        return resultList

#
# Runner to run the test cases locally.
#
class TestCaseRunner:

    def getTestSuites(self, mapping, testSuiteIds):
        return mapping.getTestSuites(testSuiteIds)

    def filterOptions(self, options):
        return options

    def startServerSide(self, testcase, current):
        return testcase._startServerSide(current)

    def stopServerSide(self, testcase, current, success):
        testcase._stopServerSide(current, success)

    def runClientSide(self, testcase, current, host):
        testcase._runClientSide(current, host)

#
# Runner to run the test cases remotely with the controller (requires IcePy)
#
class RemoteTestCaseRunner(TestCaseRunner):

    def __init__(self, communicator, clientPrx, serverPrx):
        import Test
        if clientPrx:
            self.clientController = communicator.stringToProxy(clientPrx)
            self.clientController = Test.Common.ControllerPrx.checkedCast(self.clientController)
            self.clientOptions = self.clientController.getOptionOverrides()
        else:
            self.clientController = None
            self.clientOptions = {}

        if serverPrx:
            self.serverController = communicator.stringToProxy(serverPrx)
            self.serverController = Test.Common.ControllerPrx.checkedCast(self.serverController)
            self.serverOptions = self.serverController.getOptionOverrides()
        else:
            self.serverController = None
            self.serverOptions = {}

    def getTestSuites(self, mapping, testSuiteIds):
        if self.clientController:
            clientTestSuiteIds = self.clientController.getTestSuites(str(mapping))
            if testSuiteIds:
                testSuiteIds = [ts for ts in clientTestSuiteIds if ts in testSuiteIds]
            else:
                testSuiteIds = clientTestSuiteIds
        if self.serverController:
            serverTestSuiteIds = self.serverController.getTestSuites(str(mapping))
            if testSuiteIds:
                testSuiteIds = [ts for ts in serverTestSuiteIds if ts in testSuiteIds]
            else:
                testSuiteIds = serverTestSuiteIds
        return mapping.getTestSuites(testSuiteIds)

    def filterOptions(self, options):
        import Ice
        options = options.copy()
        for (key, values) in options.items():
            for opts in [self.serverOptions, self.clientOptions]:
                if hasattr(opts, key) and getattr(opts, key) is not Ice.Unset:
                    options[key] = [v for v in values if v in getattr(opts, key)]
        return options

    def startServerSide(self, testcase, current):
        if not self.serverController:
            return TestCaseRunner.startServerSide(self, testcase, current)

        import Test
        current.serverTestCase = self.serverController.runTestCase(str(testcase.getMapping()),
                                                                   testcase.getTestSuite().getId(),
                                                                   testcase.getName(),
                                                                   str(current.driver.cross))
        try:
            try:
                return current.serverTestCase.startServerSide(self.getConfig(current))
            except Test.Common.TestCaseFailedException as ex:
                current.result.writeln(ex.output)
                raise RuntimeError("test failed:\n" + str(ex))
        except:
            current.serverTestCase.destroy()
            current.serverTestCase = None
            raise

    def stopServerSide(self, testcase, current, success):
        if not self.serverController:
            TestCaseRunner.stopServerSide(self, testcase, current, success)
            return

        import Test
        try:
            current.result.write(current.serverTestCase.stopServerSide(success))
            current.host = None
        except Test.Common.TestCaseFailedException as ex:
            current.result.writeln(ex.output)
            raise RuntimeError("test failed")
        finally:
            current.serverTestCase = None

    def runClientSide(self, testcase, current, host):
        import Test
        if not self.clientController:
            TestCaseRunner.runClientSide(self, testcase, current, host)
            return

        clientTestCase = self.clientController.runTestCase(str(testcase.getMapping()),
                                                           testcase.getTestSuite().getId(),
                                                           testcase.getName(),
                                                           str(current.driver.cross))
        try:
            current.result.write(clientTestCase.runClientSide(host, self.getConfig(current)))
        except Test.Common.TestCaseFailedException as ex:
            current.result.writeln(ex.output)
            raise RuntimeError("test failed")
        finally:
            clientTestCase.destroy()

    def getConfig(self, current):
        import Test
        return Test.Common.Config(current.config.protocol,
                                  current.config.mx,
                                  current.config.serialize,
                                  current.config.compress,
                                  current.config.ipv6,
                                  current.config.cprops,
                                  current.config.sprops)

class LocalDriver(Driver):

    class Current(Driver.Current):

        def __init__(self, driver, testsuite, result, index, total):
            Driver.Current.__init__(self, driver, testsuite, result)
            self.index = index
            self.total = total

    @classmethod
    def getSupportedArgs(self):
        return ("", ["cross=", "workers=", "continue", "loop", "start=", "all", "all-cross", "host=",
                     "client=", "server=", "show-durations"])

    @classmethod
    def usage(self):
        print("")
        print("Local driver options:")
        print("--cross=<mapping>     Run with servers from given mapping.")
        print("--workers=<count>     The number of worker threads to run the tests.")
        print("--start=<index>       Start running the tests at the given index.")
        print("--loop                Run the tests in a loop.")
        print("--continue            Don't stop on failures.")
        print("--all                 Run all sensible permutations of the tests.")
        print("--all-cross           Run all sensible permutations of cross language tests.")
        print("--client=<proxy>      The endpoint of the controller to run the client side.")
        print("--server=<proxy>      The endpoint of the controller to run the server side.")
        print("--show-durations      Print out the duration of each tests.")

    def __init__(self, options, *args, **kargs):
        Driver.__init__(self, options, *args, **kargs)

        self.cross = ""
        self.allCross = False
        self.workers = 1
        self.continueOnFailure = False
        self.loop = False
        self.start = 0
        self.all = False
        self.showDurations = False

        self.clientCtlPrx = ""
        self.serverCtlPrx = ""

        parseOptions(self, options, { "continue" : "continueOnFailure",
                                      "l" : "loop",
                                      "all-cross" : "allCross",
                                      "client" : "clientCtlPrx",
                                      "server" : "serverCtlPrx",
                                      "show-durations" : "showDurations" })

        if self.cross:
            self.cross = Mapping.getByName(self.cross)
            if not self.cross:
                raise RuntimeError("unknown mapping `{0}' for --cross option".format(self.cross))

        self.results = []
        self.threadlocal = threading.local()

    def run(self, mappings, testSuiteIds):

        if self.clientCtlPrx or self.serverCtlPrx:
            self.initCommunicator()
            self.runner = RemoteTestCaseRunner(self.communicator, self.clientCtlPrx, self.serverCtlPrx)
        else:
            self.runner = TestCaseRunner()

        while True:
            executor = Executor(self.threadlocal, self.workers, self.continueOnFailure)
            for mapping in mappings:
                testsuites = self.runner.getTestSuites(mapping, testSuiteIds)

                #
                # Sort the test suites to run tests in the following order.
                #
                runOrder = mapping.getRunOrder()
                def testsuiteKey(testsuite):
                    for k in runOrder:
                        if testsuite.getId().startswith(k + '/'):
                            return testsuite.getId().replace(k, str(runOrder.index(k)))
                    return testsuite.getId()
                testsuites = sorted(testsuites, key=testsuiteKey)

                #
                # Create the executor to run the test suites on multiple workers thread is requested.
                #
                for testsuite in testsuites:
                    if mapping.filterTestSuite(testsuite.getId(), self.configs[mapping], self.filters, self.rfilters):
                        continue
                    if testsuite.getId() == "Ice/echo":
                        continue
                    elif (self.cross or self.allCross) and not testsuite.isCross():
                        continue
                    elif isinstance(self.runner, RemoteTestCaseRunner) and not testsuite.isMultiHost():
                        continue
                    executor.submit(testsuite, Mapping.getAll() if self.allCross else [self.cross])

            #
            # Run all the tests and wait for the executor to complete.
            #
            now = time.time()

            results = executor.runUntilCompleted(self, self.start)

            failures = [r for r in results if not r.isSuccess()]
            m, s = divmod(time.time() - now, 60)
            print("")
            if m > 0:
                print("Ran {0} tests in {1} minutes {2:02.2f} seconds".format(len(results), m, s))
            else:
                print("Ran {0} tests in {1:02.2f} seconds".format(len(results), s))

            if self.showDurations:
                for r in sorted(results, key = lambda r : r.getDuration()):
                    print("- {0} took {1:02.2f} seconds".format(r.testsuite, r.getDuration()))

            if len(failures) > 0:
                print("{0} succeeded and {1} failed:".format(len(results) - len(failures), len(failures)))
                for r in failures:
                    print("- {0}".format(r.testsuite))
                    for (c, ex) in r.getFailed().items():
                        lines = r.getOutput(c).strip().split('\n')
                        for i in range(0, min(4, len(lines))):
                            print("  " + lines[i])
                        if len(lines) > 4:
                            print("  [...]")
                            for i in range(max(4, len(lines) - 8), len(lines)):
                                print("  " + lines[i])
                return 1
            else:
                print("{0} succeeded".format(len(results)))
                if not self.loop:
                    return 0

    def runTestSuite(self, current):
        current.result.writeln("*** [{0}/{1}] Running {2}/{3} tests ***".format(current.index,
                                                                                current.total,
                                                                                current.testsuite.getMapping(),
                                                                                current.testsuite))
        success = False
        try:
            current.testsuite.setup(current)
            for testcase in current.testsuite.getTestCases():
                config = current.config
                try:
                    for conf in current.config.getAll(current, testcase) if self.all else [current.config]:
                        current.config = conf
                        testcase.run(current)
                except:
                    if current.driver.debug:
                        current.result.writeln(traceback.format_exc())
                    raise
                finally:
                    current.config = config
            success = True
        finally:
            current.testsuite.teardown(current, success)

    def runClientServerTestCase(self, current):

        if current.testcase.getParent():
            success = False
            host = current.testcase._startServerSide(current)
            try:
                current.testcase._runClientSide(current, host)
                success = True
            finally:
                current.testcase._stopServerSide(current, success)
            return

        client = current.testcase.getClientTestCase()
        for cross in (Mapping.getAll() if self.allCross else [self.cross]):

            # Only run cross tests with allCross
            if self.allCross and cross == current.testcase.getMapping():
                continue

            # Skip if the mapping doesn't provide the test case
            server = current.testcase.getServerTestCase(cross)
            if not server:
                continue

            if cross and server.getMapping() != cross:
                if not self.allCross:
                    current.writeln("skipped, no server available for `{0}' mapping".format(cross))
                continue

            current.writeln("[ running {0} test ]".format(current.testcase))
            if not self.all:
                current.config = current.config.cloneRunnable(current)
            confStr = str(current.config)
            if confStr:
                current.writeln("- Config: {0}".format(confStr))
            if cross:
                current.writeln("- Mappings: {0}/{1}".format(client.getMapping(), server.getMapping()))
            if not current.config.canRun(current) or not current.testcase.canRun(current):
                current.writeln("skipped, not supported with this configuration")
                return

            success = False
            host = self.runner.startServerSide(server, current)
            try:
                self.runner.runClientSide(client, current, host)
                success = True
            finally:
                self.runner.stopServerSide(server, current, success)

    def runTestCase(self, current):
        if not self.cross and not self.allCross:
            if not current.testcase.getParent():
                current.writeln("[ running {0} test ]".format(current.testcase))
                if not self.all:
                    current.config = current.config.cloneRunnable(current)
                confStr = str(current.config)
                if confStr:
                    current.writeln("- Config: {0}".format(confStr))
                if not current.config.canRun(current) or not current.testcase.canRun(current):
                    current.writeln("skipped, not supported with this configuration")
                    return

            current.testcase._runClientSide(current)

    def isWorkerThread(self):
        return hasattr(self.threadlocal, "num")

    def getTestPort(self, portnum):
        # Return a port number in the range 14100-14199 for the first thread, 14200-14299 for the
        # second thread, etc.
        assert(portnum < 100)
        baseport = 14000 + self.threadlocal.num * 100 if hasattr(self.threadlocal, "num") else 12010
        return baseport + portnum

    def getProps(self, process, current):
        props = Driver.getProps(self, process, current)
        if isinstance(process, IceProcess):
            if current.host:
                props["Ice.Default.Host"] = current.host
            # Ice process from the bin directory don't support Test.BasePort
            if not process.isFromBinDir() and hasattr(self.threadlocal, "num"):
                props["Test.BasePort"] = 14000 + self.threadlocal.num * 100
        return props

    def getMappings(self):
        return Mapping.getAll() if self.allCross else [self.cross] if self.cross else []

    def filterOptions(self, testcase, options):
        return self.runner.filterOptions(options)

Driver.add("local", LocalDriver)
