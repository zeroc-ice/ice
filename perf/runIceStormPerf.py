#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, time

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

repetitions = 1000
period = 2 # (ms)
results = { }

# Constants
blocking = 1
reactive = 2
mt = 3

def calcResults(subscribers):

    mean1 = 0.0
    mean2 = 0.0
    maxV = 0.0
    for [l1,v1,t1,l2,v2,t2] in subscribers:
        mean1 += l1
        mean2 += l2

        if v1 > maxV:
            maxV = v1
        if v2 > maxV:
            maxV = v2
    
    return [mean1 / len(subscribers), mean2 / len(subscribers), maxV]

def printAllResults():

    print "\n"
    print "Final results"
    
    for (k, v) in results.iteritems():
        mean1 = 0.0
        mean2 = 0.0
        mean3 = 0.0
        bestl1 = max
        bestl2 = max
        bestv = max
        for [l1,l2,var] in v:
            mean1 += l1
            mean2 += l2
            mean3 += var
            if l2 < bestl2:
                bestl1 = l1
                bestl2 = l2
                bestv = var
        mean1 /= len(v)
        mean2 /= len(v)
        mean3 /= len(v)
        
        print "%-36s %04.2f %04.2f %01.3f %04.2f %04.2f %01.3f " % (k, mean1, mean2, mean3, bestl1, bestl2, bestv)
    
def runPerfIceStorm(iter, name, nSubscriber, options, publisherOptions):

    print str(iter) + ": " + name + "...",
    sys.stdout.flush()

    cwd = os.getcwd()
    os.chdir(os.path.join("src", "IceStorm"))

    servicePipe = os.popen("icebox --Ice.Config=config")
    TestUtil.getAdapterReady(servicePipe)
    TestUtil.getAdapterReady(servicePipe)
    TestUtil.getAdapterReady(servicePipe)

    subscriberPipe = [ ]
    for i in range(0, nSubscriber):
        subscriberPipe.append(os.popen("./subscriber -r " + str(repetitions) + " " + options))
        getAdapterReady(subscriberPipe[i])

    publisherPipe = os.popen("./publisher -r " + str(repetitions) + " -p " + str(period) + " " + publisherOptions)

    publisherPipe.close()
    subscribers = [ ]
    for i in range(0, nSubscriber):
        out = subscriberPipe[i].read()
        try:
            subscribers.append([float(x) for x in out.split()])
        except KeyboardInterrupt:
            print " invalid output: " + out,
            pass
        subscriberPipe[i].close()

    os.system("iceboxadmin --Ice.Config=config shutdown")
    servicePipe.close();

    os.chdir(cwd)

    if len(subscribers) > 0:
        r = calcResults(subscribers)
        addResults(name, r)
        printResults(r)
    else:
        print "failed"
             
def runPerfCosEvent(iter, name, nSubscriber, mode):

    print str(iter) + ": " + name + "...",
    sys.stdout.flush()

    os.chdir("./src/CosEvent")

    options = ""

    # Publisher options
    pOpts = ""
    if mode == blocking:
        pOpts = " -ORBSvcConf svc.blocking.conf"

    # Service options
    svcOpts = ""
    if mode == blocking:        
        svcOpts = " -ORBSvcConf svc.event.blocking.conf"
    elif mode == reactive:
        svcOpts = " -ORBSvcConf svc.event.reactive.conf"
    else:
        svcOpts = " -ORBSvcConf svc.event.mt.conf"
 
    servicePipe = os.popen("./Service " + svcOpts + " 2>&1")
    ior = servicePipe.readline().strip()
    ior = ior[14:len(ior) - 1]
    subscriberPipe = [ ]

    for i in range(0, nSubscriber):
        subscriberPipe.append(os.popen("./Consumer -r " + str(repetitions) + " " + options + " " + ior))

    publisherPipe = os.popen("./Supplier -r " + str(repetitions) + " -p " + str(period) + " " + pOpts + " " + ior)
    publisherPipe.close()
    
    subscribers = [ ]
    for i in range(0, nSubscriber):
        subscribers.append([float(x) for x in subscriberPipe[i].read().split()])
        subscriberPipe[i].close()

    os.chdir("../..")

    r = calcResults(subscribers)
    addResults(name, r)
    printResults(r)

nconsumers = 1

iter = 1
while 1:
    try:
#        runPerfCosEvent(iter, "CosEvent twoway -> twoway", nconsumers, reactive)
#        runPerfCosEvent(iter, "CosEvent twoway -> twoway (buffered)", nconsumers, mt)
        runPerfIceStorm(iter, "IceStorm twoway -> oneway", nconsumers, "", "-t")
        runPerfIceStorm(iter, "IceStorm twoway -> twoway", nconsumers, "-t", "-t")
        runPerfIceStorm(iter, "IceStorm twoway -> twoway ordered", nconsumers, "-o", "-t")
        iter += 1
    except KeyboardInterrupt:
        printAllResults()
        break
