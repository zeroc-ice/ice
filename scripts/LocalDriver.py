#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import sys, os, time, threading
from Util import *
from Component import component

isPython2 = sys.version_info[0] == 2

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
        self.interrupted = False
        self.continueOnFailure = continueOnFailure
        self.lock = threading.Lock()

    def submit(self, testsuite, crossMappings, driver):
        mainThreadOnly = testsuite.isMainThreadOnly(driver) or self.workers == 0

        #
        # If the test supports workers and we are cross testing, ensure that all the cross
        # testing mappings support workers as well.
        #
        if not mainThreadOnly:
            for cross in crossMappings:
                if cross:
                    t = cross.findTestSuite(testsuite)
                    if t and t.isMainThreadOnly(driver):
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

    def isInterrupted(self):
        with self.lock:
            return self.interrupted

    def setInterrupt(self, value):
        with self.lock:
            self.interrupted = value

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
                if mainThread:
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
                self.interrupted = True
            if threads:
                print("Terminating (waiting for worker threads to terminate)...")
            else:
                print("")
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

    def getHost(self, transport, ipv6):
        if self.clientController:
            return self.clientController.getHost(transport, ipv6)
        else:
            return self.serverController.getHost(transport, ipv6)

    def filterOptions(self, options):
        if options is None:
            return None
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
        return Test.Common.Config(current.config.transport,
                                  current.config.mx,
                                  current.config.serialize,
                                  current.config.compress,
                                  current.config.ipv6,
                                  current.config.cprops,
                                  current.config.sprops)

class XmlExporter:

    def __init__(self, results, duration, failures):
        self.results = results
        self.duration = duration
        self.failures = failures

    def save(self, filename, hostname):
        with open(filename, "w") if isPython2 else open(filename, "w", encoding="utf-8") as out:
            out.write('<?xml version="1.1" encoding="UTF-8"?>\n')
            out.write('<testsuites tests="{0}" failures="{1}" time="{2:.9f}">\n'.format(len(self.results),
                                                                                        self.duration,
                                                                                        len(self.failures)))
            for r in self.results:
                r.writeAsXml(out, hostname)
            out.write('</testsuites>\n')

class LocalDriver(Driver):

    class Current(Driver.Current):

        def __init__(self, driver, testsuite, result, index, total):
            Driver.Current.__init__(self, driver, testsuite, result)
            self.index = index
            self.total = total

    @classmethod
    def getSupportedArgs(self):
        return ("", ["cross=", "workers=", "continue", "loop", "start=", "all", "all-cross", "host=",
                     "client=", "server=", "show-durations", "export-xml="])

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
        print("--export-xml=<file>   Export JUnit XML test report.")

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
        self.exportToXml = ""

        self.clientCtlPrx = ""
        self.serverCtlPrx = ""

        parseOptions(self, options, { "continue" : "continueOnFailure",
                                      "l" : "loop",
                                      "all-cross" : "allCross",
                                      "client" : "clientCtlPrx",
                                      "server" : "serverCtlPrx",
                                      "show-durations" : "showDurations",
                                      "export-xml" : "exportToXml" })

        if self.cross:
            self.cross = Mapping.getByName(self.cross)
            if not self.cross:
                raise RuntimeError("unknown mapping `{0}' for --cross option".format(self.cross))

        self.results = []
        self.threadlocal = threading.local()
        self.loopCount = 1
        self.executor = Executor(self.threadlocal, self.workers, self.continueOnFailure)

    def run(self, mappings, testSuiteIds):

        if self.clientCtlPrx or self.serverCtlPrx:
            self.initCommunicator()
            self.runner = RemoteTestCaseRunner(self.communicator, self.clientCtlPrx, self.serverCtlPrx)
        else:
            self.runner = TestCaseRunner()

        try:
            while True:
                for mapping in mappings:
                    testsuites = self.runner.getTestSuites(mapping, testSuiteIds)

                    #
                    # Sort the test suites to run tests in the following order.
                    #
                    runOrder = self.component.getRunOrder()
                    def testsuiteKey(testsuite):
                        for k in runOrder:
                            if testsuite.getId().startswith(k + '/'):
                                return testsuite.getId().replace(k, str(runOrder.index(k)))
                        return testsuite.getId()
                    testsuites = sorted(testsuites, key=testsuiteKey)

                    for testsuite in testsuites:
                        if mapping.filterTestSuite(testsuite.getId(), self.configs[mapping], self.filters, self.rfilters):
                            continue
                        if testsuite.getId() == "Ice/echo":
                            continue
                        elif (self.cross or self.allCross) and not self.component.isCross(testsuite.getId()):
                            continue
                        elif isinstance(self.runner, RemoteTestCaseRunner) and not testsuite.isMultiHost():
                            continue
                        self.executor.submit(testsuite, Mapping.getAll(self) if self.allCross else [self.cross], self)

                #
                # Run all the tests and wait for the executor to complete.
                #
                now = time.time()

                results = self.executor.runUntilCompleted(self, self.start)

                failures = [r for r in results if not r.isSuccess()]
                duration = time.time() - now

                if self.exportToXml:
                    XmlExporter(results, duration, failures).save(self.exportToXml, os.getenv("NODE_NAME", ""))

                m, s = divmod(duration, 60)
                print("")
                if m > 0:
                    print("Ran {0} tests in {1} minutes {2:02.2f} seconds".format(len(results), m, s))
                else:
                    print("Ran {0} tests in {1:02.2f} seconds".format(len(results), s))

                if self.showDurations:
                    for r in sorted(results, key = lambda r : r.getDuration()):
                        print("- {0} took {1:02.2f} seconds".format(r.testsuite, r.getDuration()))

                self.loopCount += 1

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
        finally:
            Expect.cleanup() # Cleanup processes which might still be around

    def runTestSuite(self, current):
        if self.loop:
            current.result.write("*** [{0}/{1} loop={2}] ".format(current.index, current.total, self.loopCount))
        else:
            current.result.write("*** [{0}/{1}] ".format(current.index, current.total))
        current.result.writeln("Running {0}/{1} tests ***".format(current.testsuite.getMapping(), current.testsuite))

        success = False
        try:
            try:
                current.result.started("setup")
                current.testsuite.setup(current)
                current.result.succeeded("setup")
            except Exception as ex:
                current.result.failed("setup", traceback.format_exc())
                raise

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
            try:
                current.result.started("teardown")
                current.testsuite.teardown(current, success)
                current.result.succeeded("teardown")
            except Exception as ex:
                current.result.failed("teardown", traceback.format_exc())
                raise

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
        for cross in (Mapping.getAll(self) if self.allCross else [self.cross]):

            # Only run cross tests with allCross
            if self.allCross and cross == current.testcase.getMapping():
                continue

            # Skip if the cross test server mapping is another mapping than the cross mapping
            if cross and cross != cross.getServerMapping():
                continue

            # Skip if the mapping doesn't provide the test case
            server = current.testcase.getServerTestCase(cross)
            if not server:
                continue

            current.writeln("[ running {0} test - {1} ]".format(current.testcase, time.strftime("%x %X")))
            if not self.all:
                current.config = current.config.cloneRunnable(current)
            confStr = str(current.config)
            if confStr:
                current.writeln("- Config: {0}".format(confStr))
                current.desc = confStr
            else:
                current.desc = ""
            if cross:
                current.writeln("- Mappings: {0},{1}".format(client.getMapping(), server.getMapping()))
                current.desc += (" " if current.desc else "") + "cross={0}".format(server.getMapping())
            if not current.config.canRun(current.testsuite.getId(), current) or not current.testcase.canRun(current):
                current.result.skipped(current, "not supported with this configuration")
                return

            success = False
            host = self.runner.startServerSide(server, current)
            try:
                self.runner.runClientSide(client, current, host)
                success = True
            finally:
                #
                # We start a thread to stop the servers, this ensures that stopServerSide doesn't get
                # interrupted by potential KeyboardInterrupt exceptions which could leave some servers
                # behind.
                #
                failure = []
                sem = threading.Semaphore(0)
                def stopServerSide():
                    try:
                        self.runner.stopServerSide(server, current, success)
                    except Exception as ex:
                        failure.append(ex)
                    except KeyboardInterrupt: # Potentially raised by Except.py if Ctrl-C
                        pass
                    sem.release()

                t=threading.Thread(target = stopServerSide)
                t.start()
                while True:
                    try:
                        #
                        # NOTE: we can't just use join() here because of https://bugs.python.org/issue21822
                        # We use a semaphore to wait for the servers to be stopped and return.
                        #
                        sem.acquire()
                        if failure:
                            raise failure[0]
                        t.join()
                        break
                    except KeyboardInterrupt:
                        pass # Ignore keyboard interrupts

    def runTestCase(self, current):
        if self.cross or self.allCross:
            #current.result.skipped(current, "only client/server tests are ran with cross tests")
            return

        if not current.testcase.getParent():
            current.writeln("[ running {0} test - {1} ]".format(current.testcase, time.strftime("%x %X")))
            if not self.all:
                current.config = current.config.cloneRunnable(current)
            confStr = str(current.config)
            if confStr:
                current.writeln("- Config: {0}".format(confStr))
                current.desc = confStr
            if not current.config.canRun(current.testsuite.getId(), current) or not current.testcase.canRun(current):
                current.result.skipped(current, "not supported with this configuration")
                return

        current.testcase._runClientSide(current)

    def getHost(self, transport, ipv6):
        if isinstance(self.runner, RemoteTestCaseRunner):
            return self.runner.getHost(transport, ipv6)
        else:
            return Driver.getHost(self, transport, ipv6)

    def isWorkerThread(self):
        return hasattr(self.threadlocal, "num")

    def isInterrupted(self):
        return self.executor.isInterrupted()

    def setInterrupt(self, value):
        self.executor.setInterrupt(value)

    def getTestPort(self, portnum):
        # Return a port number in the range 14100-14199 for the first thread, 14200-14299 for the
        # second thread, etc.
        assert(portnum < 100)
        baseport = 14000 + self.threadlocal.num * 100 if hasattr(self.threadlocal, "num") else 12010
        return baseport + portnum

    def getProps(self, process, current):
        props = Driver.getProps(self, process, current)
        if isinstance(process, IceProcess):
            # Ice process from the bin directory don't support Test.BasePort
            if not process.isFromBinDir() and hasattr(self.threadlocal, "num"):
                props["Test.BasePort"] = 14000 + self.threadlocal.num * 100
        return props

    def getMappings(self):
        return Mapping.getAll(self) if self.allCross else [self.cross] if self.cross else []

    def filterOptions(self, options):
        return self.runner.filterOptions(options)

Driver.add("local", LocalDriver)
