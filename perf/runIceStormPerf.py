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

# def calcResults(subscribers):

#     mean1 = 0.0
#     mean2 = 0.0
#     maxV = 0.0
#     for [l1,v1,t1,l2,v2,t2] in subscribers:
#         mean1 += l1
#         mean2 += l2

#         if v1 > maxV:
#             maxV = v1
#         if v2 > maxV:
#             maxV = v2
    
#     return [mean1 / len(subscribers), mean2 / len(subscribers), maxV]

# def printAllResults():

#     print "\n"
#     print "Final results"
    
#     for (k, v) in results.iteritems():
#         mean1 = 0.0
#         mean2 = 0.0
#         mean3 = 0.0
#         bestl1 = max
#         bestl2 = max
#         bestv = max
#         for [l1,l2,var] in v:
#             mean1 += l1
#             mean2 += l2
#             mean3 += var
#             if l2 < bestl2:
#                 bestl1 = l1
#                 bestl2 = l2
#                 bestv = var
#         mean1 /= len(v)
#         mean2 /= len(v)
#         mean3 /= len(v)
        
#         print "%-36s %04.2f %04.2f %01.3f %04.2f %04.2f %01.3f " % (k, mean1, mean2, mean3, bestl1, bestl2, bestv)

class IceStormTest(TestUtil.Test) :

    def __init__(self, expr, results, i, product, latency, nPublishers, nSubscribers):

        if latency:
            test = "latency " + str(nPublishers) + "-" + str(nSubscribers)
        else:
            test = "throughput " + str(nPublishers) + "-" + str(nSubscribers)
            
        TestUtil.Test.__init__(self, expr, results, i, product, test)

        self.latency = latency
        self.nSubscribers = nSubscribers
        self.nPublishers = nPublishers

    def run(self, name, subscriberOpts, publisherOpts):

        TestUtil.Test.run(self, name, { "publisher" : publisherOpts, "subscriber" : subscriberOpts })

    def execute(self, options):
        
        cwd = os.getcwd()
        os.chdir(os.path.join(toplevel, "src", self.product))

        # Start IceStorm
        servicePipe = os.popen(os.path.join(os.environ['ICE_HOME'], "bin", "icebox") + " --Ice.Config=config")
        TestUtil.getAdapterReady(servicePipe)
        TestUtil.getAdapterReady(servicePipe)
        TestUtil.getAdapterReady(servicePipe)

        # Start the subscribers
        subscribersPipe = [ ]
        if self.latency:
            options["publisher"] += " -r " + str(latencyRepetitions)
        else:
            options["publisher"] += " -r " + str(throughputRepetitions)
        for i in range(0, self.nSubscribers):
            subscribersPipe.append(os.popen("./subscriber -c " + str(self.nPublishers) + " " + options["subscriber"]))
            TestUtil.getAdapterReady(subscribersPipe[i])

        # Start the publishers
        publishersPipe = [ ]
        if self.latency:
            options["publisher"] += " -p " + str(period) + " -r " + str(latencyRepetitions)
        else:
            options["publisher"] += " -r " + str(throughputRepetitions)
            
        for i in range(0, self.nPublishers):
            publishersPipe.append(os.popen("./publisher " + options["publisher"]))

        # Wait for the publishers to complete
        for i in range(0, self.nPublishers):
            TestUtil.printOutputFromPipe(publishersPipe[i])
            publishersPipe[i].close()

        # Gather results from each subscribers
        results = [ ]
        for i in range(0, self.nSubscribers):
            out = subscribersPipe[i].read()
            try:
                r = [float(x) for x in out.split()]
                if self.latency:
                    results.append((r[0], r[3])) # Latency w/o payload & w/ payload
                    if r[1] > 3:
                        print "high deviation: " + r[1] + " ",
                    if r[4] > 3:
                        print "high deviation: " + r[4] + " ",
                else:
                    results.append((r[2], r[5])) # Throughput w/o payload & w/ payload
            except KeyboardInterrupt:
                print " invalid output: " + out,
                pass
            subscribersPipe[i].close()

        # Shutdown IceStorm
        os.system(os.path.join(os.environ['ICE_HOME'], "bin", "iceboxadmin") + " --Ice.Config=config shutdown")
        servicePipe.close();

        os.chdir(cwd)

        return results
    
             
# def runPerfCosEvent(iter, name, nSubscriber, mode):

#     print str(iter) + ": " + name + "...",
#     sys.stdout.flush()

#     os.chdir("./src/CosEvent")

#     options = ""

#     # Publisher options
#     pOpts = ""
#     if mode == blocking:
#         pOpts = " -ORBSvcConf svc.blocking.conf"

#     # Service options
#     svcOpts = ""
#     if mode == blocking:        
#         svcOpts = " -ORBSvcConf svc.event.blocking.conf"
#     elif mode == reactive:
#         svcOpts = " -ORBSvcConf svc.event.reactive.conf"
#     else:
#         svcOpts = " -ORBSvcConf svc.event.mt.conf"
 
#     servicePipe = os.popen("./Service " + svcOpts + " 2>&1")
#     ior = servicePipe.readline().strip()
#     ior = ior[14:len(ior) - 1]
#     subscriberPipe = [ ]

#     for i in range(0, nSubscriber):
#         subscriberPipe.append(os.popen("./Consumer -r " + str(repetitions) + " " + options + " " + ior))

#     publisherPipe = os.popen("./Supplier -r " + str(repetitions) + " -p " + str(period) + " " + pOpts + " " + ior)
#     publisherPipe.close()
    
#     subscribers = [ ]
#     for i in range(0, nSubscriber):
#         subscribers.append([float(x) for x in subscriberPipe[i].read().split()])
#         subscriberPipe[i].close()

#     os.chdir("../..")

#     r = calcResults(subscribers)
#     addResults(name, r)
#     printResults(r)

def runIceStormPerfs(expr, results, i):

    test = IceStormTest(expr, results, i, "IceStorm", True, 1, 1)
    test.run("oneway", "", "-t")
    test.run("twoway", "-t", "-t")
    test.run("twoway ordered", "-o", "-t")

    test = IceStormTest(expr, results, i, "IceStorm", False, 1, 1)
    test.run("oneway", "", "-t")
    test.run("twoway", "-t", "-t")
    test.run("twoway ordered", "-o", "-t")

    test = IceStormTest(expr, results, i, "IceStorm", True, 1, 25)
    test.run("oneway", "", "-t")
    test.run("twoway", "-t", "-t")
    test.run("twoway ordered", "-o", "-t")

    test = IceStormTest(expr, results, i, "IceStorm", False, 1, 25)
    test.run("oneway", "", "-t")
    test.run("twoway", "-t", "-t")
    test.run("twoway ordered", "-o", "-t")


def runCosEventPerfs(expr, results, i):
    return
#     test = CosEventTest(expr, results, i, "CosEvent", "latency 1-1")
#     test.run("twoway", );
#     test.run("twoway buffered", );
#        runPerfCosEvent(iter, "CosEvent twoway -> twoway", nconsumers, reactive)
#        runPerfCosEvent(iter, "CosEvent twoway -> twoway (buffered)", nconsumers, mt)

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
