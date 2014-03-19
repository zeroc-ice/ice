#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# TODO:
#
#  - regular expressions need to be reimplemented.
#

import os, sys, time, getopt, re, platform

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise RuntimeError("can't find toplevel directory!")

sys.path.append(os.path.join(toplevel, "config"))
from scripts import *

latencyRepetitions = 10000
throughputRepetitions = 30000
period = 2 # (ms)

class Test(TestUtil.Test) :

    def __init__(self, product, wPayload, latency, nPublishers, nSubscribers):

        if latency:
            test = "latency " + str(nPublishers) + "-" + str(nSubscribers)
        else:
            test = "throughput " + str(nPublishers) + "-" + str(nSubscribers)

        if wPayload:
            TestUtil.Test.__init__(self, product, test)
        else:
            TestUtil.Test.__init__(self, product, test + " w/o payload")

        self.payload = wPayload
        self.latency = latency
        self.nSubscribers = nSubscribers
        self.nPublishers = nPublishers
        self.period = nSubscribers # (ms)

    def startPublishers(self, name, notifier, options):

        options = " " + options["publisher"]        
        if self.latency:
            options += " -p " + str(self.period) + " -r " + str(latencyRepetitions)
        else:
            options += " -r " + str(throughputRepetitions)
        if self.payload:
            options += " -w"

	publisherPipes = []
	for i in range(0, self.nPublishers):
	    p = os.popen(os.path.join(".", name) + options) 
	    if p == None:
		print "Unable to start all publishers"
		sys.exit(1)
	    publisherPipes.append(p)

        iors = ""
        for i in publisherPipes:
            iors += ' "' + i.readline().strip() + '"';
            TestUtil.getAdapterReady(i)

        pipe = os.popen(os.path.join(".", notifier) + iors)
	if pipe == None:
	    print "Unable to start notifier"
	    sys.exit(1)

        TestUtil.printOutputFromPipe(pipe)
        pipe.close()
        
        return publisherPipes

    def startSubscribers(self, name, options):

        options = " " + options["subscriber"]
        options += " -c " + str(self.nPublishers)
        if self.latency:
            options += " -r " + str(latencyRepetitions)
        else:
            options += " -r " + str(throughputRepetitions)
        if self.payload:
            options += " -w"

	subscriberPipes = []
	for i in range(0, self.nSubscribers):
	    p = os.popen(os.path.join(".", name) + options)
	    if p == None:
		print "Unable to start all subscribers"
		sys.exit(1)
	    subscriberPipes.append(p)

        for i in subscriberPipes:
            TestUtil.getAdapterReady(i)
        return subscriberPipes

    def waitForResults(self, subscribers, publishers):
        
        # Wait for the publishers to complete
        for i in range(0, self.nPublishers):
            TestUtil.printOutputFromPipe(publishers[i])
            publishers[i].close()

        # Gather results from each subscribers
        results = [ ]
        for i in range(0, self.nSubscribers):
            try:
		out = eval(subscribers[i].read())
		results.append(out)
            except KeyboardInterrupt:
                print " invalid output: " + out,
                pass
	    except SyntaxError, e:
                print e
		pass
            subscribers[i].close()

	if len(results) == 0:
	    return None 

	expected = 0
	if self.latency:
	    expected = latencyRepetitions
	else:
	    expected = throughputRepetitions

        avgLatency = 0.0
	avgThroughput = 0.0
	notCompleted = 0
        for r in results:
	    avgLatency += r['latency']
	    avgThroughput += r['throughput']
	    if r['repetitions'] < expected * 0.9:
		notCompleted += 1

        avgLatency /= len(results)
        avgThroughput /= len(results)
	return { 'latency': avgLatency, 'throughput': avgThroughput, 'data': results, 'expected': expected, 'incomplete': notCompleted}

class IceStormTest(Test):

    def run(self, name, subscriberOpts, publisherOpts, topics):

        nthreads = 4
        threadOpts = " --Ice.ThreadPool.Server.Size=" + str(nthreads) + \
                     " --Ice.ThreadPool.Server.SizeMax=" + str(nthreads) + \
                     " --Ice.ThreadPool.Server.SizeWarn=0"

        publisherOpts += threadOpts;
        subscriberOpts += threadOpts;

        return TestUtil.Test.run(self, name, { "publisher" : publisherOpts, "subscriber" : subscriberOpts }, topics)

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
        publishersPipe = self.startPublishers("publisher", "notifier", options)
        result = self.waitForResults(subscribersPipe, publishersPipe)

        # Shutdown IceStorm
        os.system(os.path.join(os.environ['ICE_HOME'], "bin", "iceboxadmin") + " --Ice.Config=config shutdown")
        servicePipe.close()

        os.chdir(cwd)
        return result

class CosEventTest(Test):

    def __init__(self, product, wPayload, latency, nPublishers, nSubscribers):

        Test.__init__(self, product, wPayload, latency, nPublishers, nSubscribers)

    def run(self, name, serviceOpts, topics):

        threadOpts = " -n 4"
        return TestUtil.Test.run(self, name, { "service" : serviceOpts, "publisher" : threadOpts, "subscriber" : threadOpts }, topics)

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
        publishersPipe = self.startPublishers("Supplier", "Notifier", options)
        result = self.waitForResults(subscribersPipe, publishersPipe)

        # Shutdown CosEvent
	os.system(os.path.join(".", "Destroyer") + " file://ec.ior")
        servicePipe.close()

        os.chdir(cwd)
        return result

def runIceStormPerfs(expr, results):

    prod = "IceStorm"
    noPayLoad = False
    payLoad = True
    latency = True
    throughput = False

    tests = [
	    (prod, noPayLoad, latency, 1, 1, [ (["latnp1"], "oneway", "", "-t"), 
		(["latnp2"], "twoway", "-o", "-t") ]),
	    (prod, payLoad, latency, 1, 1, [ (["lat1"], "oneway", "", "-t"), (["lat2"], "twoway", "-o", "-t") ]),
	    (prod, payLoad, latency, 1, 2, [ (["lat1"], "oneway", "", "-t"), (["lat2"], "twoway", "-o", "-t") ]),
	    (prod, payLoad, latency, 1, 5, [ (["lat1"], "oneway", "", "-t"), (["lat2"], "twoway", "-o", "-t") ]),
	    (prod, payLoad, latency, 1, 10, [ (["lat1"], "oneway", "", "-t"), (["lat2"], "twoway", "-o", "-t") ]),
	    (prod, payLoad, latency, 1, 20, [ (["lat1"], "oneway", "", "-t"), (["lat2"], "twoway", "-o", "-t") ]),
	    (prod, payLoad, throughput, 1, 1, [ 
		(["tpt1"], "oneway", "", "-t"), (["tpt1b"], "oneway (batch)", "-o", "-b"), (["tpt2"], "twoway", "-o", "-t")]),
	    (prod, payLoad, throughput, 1, 10, [ 
		(["tpt1"], "oneway", "", "-t"), (["tpt1b"], "oneway (batch)", "-o", "-b"), (["tpt2"], "twoway", "-o", "-t")]),
	    (prod, payLoad, throughput, 10, 1, [ 
		(["tpt1"], "oneway", "", "-t"), (["tpt1b"], "oneway (batch)", "-o", "-b"), (["tpt2"], "twoway", "-o", "-t")]),
	    (prod, payLoad, throughput, 5, 5, [ 
		(["tpt1"], "oneway", "", "-t"), (["tpt1b"], "oneway (batch)", "-o", "-b"), (["tpt2"], "twoway", "-o", "-t")]),
	    ]

    if len(expr) > 0:
	candidates = tests
	tests = []
	for prodName, withPayload, latencyTest, suppliers, consumers, cases in candidates:
	    allowedCases = []
	    for e in expr:
		for c in cases:
		    type = "throughput"
		    if latencyTest:
			type = "latency"
		    criteria = "%s %s %s" % (prodName, type, c[1])
		    if e.match(criteria):
			allowedCases.append(c)
	    if len(allowedCases) > 0:
		tests.append((prodName, withPayload, latencyTest, suppliers, consumers, allowedCases))

    for prodName, withPayload, latencyTest, suppliers, consumers, cases in tests:
	test = IceStormTest(prodName, withPayload, latencyTest, suppliers, consumers)
	for t, clientArg, opt, serverArg in cases:
	    r = test.run(clientArg, opt, serverArg, t)
	    if r != None:
		r['suppliers'] = suppliers
		r['consumers'] = consumers
		r['throughput'] = r['throughput'] * consumers
		results.append(r)

def runCosEventPerfs(expr, results):

    reactiveService = " -ORBSvcConf svc.event.reactive.conf"
    bufferedService = " -ORBSvcConf svc.event.mt.conf"
    noPayLoad = False
    payLoad = True
    latency = True
    throughput = False
    prod = "CosEvent"

    tests = [
	    (prod, noPayLoad, latency, 1, 1, 
		[(["latnp2"], "twoway", reactiveService), (["latnp2"], "twoway buffered", bufferedService)]),
	    (prod, payLoad, latency, 1, 1, 
		[(["lat2"], "twoway", reactiveService), (["lat2"], "twoway buffered", bufferedService)]),
	    (prod, payLoad, latency, 1, 2, 
		[(["lat2"], "twoway", reactiveService), (["lat2"], "twoway buffered", bufferedService)]),
	    (prod, payLoad, latency, 1, 5, 
		[(["lat2"], "twoway", reactiveService), (["lat2"], "twoway buffered", bufferedService)]),
	    (prod, payLoad, latency, 1, 10, 
		[(["lat2"], "twoway", reactiveService), (["lat2"], "twoway buffered", bufferedService)]),
	    (prod, payLoad, latency, 1, 20, 
		[(["lat2"], "twoway", reactiveService), (["lat2"], "twoway buffered", bufferedService)]),
	    (prod, payLoad, throughput, 1, 1, 
		[(["tpt2"], "twoway", reactiveService), (["tpt1b"], "twoway buffered", bufferedService)]),
	    (prod, payLoad, throughput, 1, 10, 
		[(["tpt2"], "twoway", reactiveService), (["tpt1b"], "twoway buffered", bufferedService)]),
	    (prod, payLoad, throughput, 10, 1, 
		[(["tpt2"], "twoway", reactiveService), (["tpt1b"], "twoway buffered", bufferedService)]),
	    (prod, payLoad, throughput, 5, 5, 
		[(["tpt2"], "twoway", reactiveService), (["tpt1b"], "twoway buffered", bufferedService)]),
	    ]

    if len(expr) > 0:
	candidates = tests
	tests = []
	for prodName, withPayload, latencyTest, suppliers, consumers, cases in candidates:
	    allowedCases = []
	    for e in expr:
		for c in cases:
		    type = "throughput"
		    if latencyTest:
			type = "latency"
		    criteria = "%s %s %s" % (prodName, type, c[0])
		    if e.match(criteria):
			allowedCases.append(c)
	    if len(allowedCases) > 0:
		tests.append((prodName, withPayload, latencyTest, suppliers, consumers, allowedCases))

    for prodName, withPayload, latencyTest, suppliers, consumers, cases in tests:
	test = CosEventTest(prodName, withPayload, latencyTest, suppliers, consumers)
	for t, clientArg, serverArg in cases:
	    r = test.run(clientArg, serverArg, t)
	    if r != None:
		r['suppliers'] = suppliers
		r['consumers'] = consumers
		r['throughput'] = r['throughput'] * consumers
		results.append(r)

try:
    opts, pargs = getopt.getopt(sys.argv[1:], 'hi:o:n:', ['help', 'iter=', 'output=', 'hostname='])
except getopt.GetoptError:
    usage()

niter = 1
printResults = False
filename = ""
hostname = ""
for o, a in opts:
    if o == '-i' or o == "--iter":
        niter = int(a)
    elif o == '-h' or o == "--help":
        usage()
    elif o == '-o' or o == "--output":
        filename = a
    elif o == '-n' or o == "--hostname":
        hostname = a

if not os.environ.has_key('ICE_HOME'):
    if os.path.exists(os.path.join(toplevel, "..", "ice")):
        os.environ['ICE_HOME'] = os.path.join(toplevel, "..", "ice")

if not os.environ.has_key('ICE_HOME') and not os.environ.has_key('TAO_ROOT'):
    print "You need to set at least ICE_HOME or TAO_ROOT!"
    sys.exit(1)

if filename == "":
    (system, name, ver, build, machine, processor) = platform.uname()
    if hostname == "":
        hostname = name
        if hostname.find('.'):
            hostname = hostname[0:hostname.find('.')]
    filename = ("results.icestorm." + system + "." + hostname).lower()

expr = [ ]
if len(pargs) > 0:
    for e in pargs:
        expr.append(re.compile(e))

results = []

i = 1        
while i <= niter:
    try:
        if os.environ.has_key('ICE_HOME'):
            runIceStormPerfs(expr, results)
        if os.environ.has_key('TAO_ROOT'):
            runCosEventPerfs(expr, results)
        i += 1
    except KeyboardInterrupt:
        break

print "\n"
print "All results:"
outputFile = file(filename, 'w+b')
outputFile.write(str(results))
outputFile.close()

TestUtil.PrintResults(results, filename, [('IceStorm', 'CosEvent')])
