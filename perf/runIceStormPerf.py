#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, time, getopt, re

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

latencyRepetitions = 10000
throughputRepetitions = 50000
period = 2 # (ms)

class Test(TestUtil.Test) :

    def __init__(self, expr, results, i, product, wPayload, latency, nPublishers, nSubscribers):

        if latency:
            test = "latency " + str(nPublishers) + "-" + str(nSubscribers)
        else:
            test = "throughput " + str(nPublishers) + "-" + str(nSubscribers)

        if wPayload:
            TestUtil.Test.__init__(self, expr, results, i, product, test)
        else:
            TestUtil.Test.__init__(self, expr, results, i, product + " w/o payload", test)

        self.payload = wPayload
        self.latency = latency
        self.nSubscribers = nSubscribers
        self.nPublishers = nPublishers

    def startPublishers(self, name, options):

        options = " " + options["publisher"]        
        if self.latency:
            options += " -p " + str(period) + " -r " + str(latencyRepetitions)
        else:
            options += " -r " + str(throughputRepetitions)
        if self.payload:
            options += " -w"
        return [os.popen(os.path.join(".", name) + options) for i in range(0, self.nPublishers)]

    def startSubscribers(self, name, options):

        options = " " + options["subscriber"]
        options += " -c " + str(self.nPublishers)
        if self.latency:
            options += " -p " + str(period) + " -r " + str(latencyRepetitions)
        else:
            options += " -r " + str(throughputRepetitions)
        if self.payload:
            options += " -w"
        subscribersPipe = [os.popen(os.path.join(".", name) + options) for i in range(0, self.nSubscribers)]
        for i in subscribersPipe:
            TestUtil.getAdapterReady(i)
        return subscribersPipe

    def waitForResults(self, subscribers, publishers):
        
        # Wait for the publishers to complete
        for i in range(0, self.nPublishers):
            TestUtil.printOutputFromPipe(publishers[i])
            publishers[i].close()

        # Gather results from each subscribers
        results = [ ]
        for i in range(0, self.nSubscribers):
            out = subscribers[i].read()
            try:
                r = [float(x) for x in out.split()]
                if self.latency:
                    results.append(r[0]) # Latency
                    if r[1] > 3:
                        print "high deviation: " + str(r[1]) + " ", # Standard deviation
                else:
                    results.append(r[2]) # Throughput
            except KeyboardInterrupt:
                print " invalid output: " + out,
                pass
            subscribers[i].close()

        mean = 0.0
        for r in results:
            mean += r
        mean /= len(results)
        return mean

class IceStormTest(Test):

    def run(self, name, subscriberOpts, publisherOpts):

        TestUtil.Test.run(self, name, { "publisher" : publisherOpts, "subscriber" : subscriberOpts })

    def execute(self, options):
        
        cwd = os.getcwd()
        os.chdir(os.path.join(toplevel, "src", "IceStorm"))

        # Start IceStorm
        servicePipe = os.popen(os.path.join(os.environ['ICE_HOME'], "bin", "icebox") + " --Ice.Config=config")
        TestUtil.getAdapterReady(servicePipe)
        TestUtil.getAdapterReady(servicePipe)
        TestUtil.getAdapterReady(servicePipe)

        # Start subscribers and publishers
        subscribersPipe = self.startSubscribers("subscriber", options)
        publishersPipe = self.startPublishers("publisher", options)
        result = self.waitForResults(subscribersPipe, publishersPipe)

        # Shutdown IceStorm
        os.system(os.path.join(os.environ['ICE_HOME'], "bin", "iceboxadmin") + " --Ice.Config=config shutdown")
        servicePipe.close();

        os.chdir(cwd)
        return result

class CosEventTest(Test):

    def run(self, name, serviceOpts):

        TestUtil.Test.run(self, name, { "service" : serviceOpts })

    def execute(self, options):
        
        cwd = os.getcwd()
        os.chdir(os.path.join(toplevel, "src", "CosEvent"))

        # Start the CosEvent service
        servicePipe = os.popen(os.path.join(".", "Service") + " " + options["service"] + " 2>&1")
        ior = servicePipe.readline().strip()
        ior = ior[14:len(ior) - 1]
        options["publisher"] = ior
        options["subscriber"] = ior

        # Start subscribers and publishers
        subscribersPipe = self.startSubscribers("Consumer", options)
        publishersPipe = self.startPublishers("Supplier", options)
        result = self.waitForResults(subscribersPipe, publishersPipe)

        os.chdir(cwd)
        return result

def runIceStormPerfs(expr, results, i):

    test = IceStormTest(expr, results, i, "IceStorm", False, True, 1, 1) # w/o payload, latency, 1-1
    test.run("oneway", "", "-t")
    test.run("twoway", "-o", "-t")

    test = IceStormTest(expr, results, i, "IceStorm", True, True, 1, 1) # w/ payload, latency, 1-1
    test.run("oneway", "", "-t")
    test.run("twoway", "-o", "-t")

    test = IceStormTest(expr, results, i, "IceStorm", True, False, 1, 1) # w/ payload, throughput 1-1
    test.run("oneway", "", "-t")
    test.run("twoway", "-o", "-t")

#     test = IceStormTest(expr, results, i, "IceStorm", True, True, 1, 10)
#     test.run("oneway", "", "-t")
#     test.run("twoway", "-o", "-t")

#     test = IceStormTest(expr, results, i, "IceStorm", True, False, 1, 10)
#     test.run("oneway", "", "-t")
#     test.run("twoway", "-o", "-t")


def runCosEventPerfs(expr, results, i):

    reactiveService = " -ORBSvcConf svc.event.reactive.conf"
    bufferedService = " -ORBSvcConf svc.event.mt.conf"

    test = CosEventTest(expr, results, i, "CosEvent", False, True, 1, 1)  # w/o payload, latency, 1-1
    test.run("twoway", reactiveService);
    test.run("twoway buffered", bufferedService)

    test = CosEventTest(expr, results, i, "CosEvent", True, True, 1, 1)  # w/ payload, latency, 1-1
    test.run("twoway", reactiveService);
    test.run("twoway buffered", bufferedService)

    test = CosEventTest(expr, results, i, "CosEvent", True, False, 1, 1)  # w/ payload, throughput, 1-1
    test.run("twoway", reactiveService);
    test.run("twoway buffered", bufferedService)

try:
    opts, pargs = getopt.getopt(sys.argv[1:], 'hi:o:n:', ['help', 'iter=', 'output=', 'hostname=']);
except getopt.GetoptError:
    usage()

niter = max
printResults = False
hostname = ""
outputFile = ""
for o, a in opts:
    if o == '-i' or o == "--iter":
        niter = int(a)
    elif o == '-h' or o == "--help":
        usage()
    elif o == '-o' or o == "--output":
        outputFile = a
    elif o == '-n' or o == "--hostname":
        hostname = a

if outputFile == "":
    if hostname == "":
        import socket
        hostname = socket.gethostname()
    outputFile = "results.icestorm." + sys.platform + "." + hostname

expr = [ ]
if len(pargs) > 0:
    for e in pargs:
        expr.append(re.compile(e))

if not os.environ.has_key('ICE_HOME'):
    if os.path.exists(os.path.join(toplevel, "..", "ice")):
        os.environ['ICE_HOME'] = os.path.join(toplevel, "..", "ice")

if not os.environ.has_key('ICE_HOME') and not os.environ.has_key('TAO_ROOT'):
    print "You need to set at least ICE_HOME or TAO_ROOT!"
    sys.exit(1)

results = TestUtil.HostResults(hostname, outputFile)

i = 1        
while i <= niter:
    try:
        if os.environ.has_key('ICE_HOME'):
            runIceStormPerfs(expr, results, i)
        if os.environ.has_key('TAO_ROOT'):
            runCosEventPerfs(expr, results, i)
        i += 1
    except KeyboardInterrupt:
        break

print "\n"
print "All results:"
all = TestUtil.AllResults()
all.add(results)
all.printAll(TestUtil.ValuesMeanAndBest(), False)
