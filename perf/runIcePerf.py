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

resultNames = [ ]
results = { }

#
# Ice configuration properties.
#
threadPoolOne = " --Ice.ThreadPool.Server.Size=1 --Ice.ThreadPool.Server.SizeMax=1 --Ice.ThreadPool.Server.SizeWarn=1"
threadPoolFour = " --Ice.ThreadPool.Server.Size=4 --Ice.ThreadPool.Server.SizeMax=4 --Ice.ThreadPool.Server.SizeWarn=0"
threadPerConnection = " --Ice.ThreadPerConnection"

def addResults(name, result):
    
    if not name in resultNames:
	resultNames.append(name)

    if not results.has_key(name):
        results[name] = [ ]
    results[name].append(result)

def printAllResults():

    print "\n"
    print "Final results"
    
    for n in resultNames:
	v = results[n]
        mean = 0.0
        best = max

        for r in v:
            mean += r
            if r < best:
                best = r

        mean /= len(v)
        
        print "%-36s %f %f " % (n, mean, best)
    
def runClientServerPerf(iter, directory, name, clientOpts, serverOpts):

    cwd = os.getcwd()
    os.chdir(directory)

    print str(iter) + ": " + name + "...",
    sys.stdout.flush()
    
    serverPipe = os.popen("./server " + serverOpts)
    TestUtil.getAdapterReady(serverPipe)

    clientPipe = os.popen("./client " + clientOpts)
    result = float(clientPipe.read())
    clientPipe.close()

    os.chdir(cwd)

    addResults(name, result)
    print result

def runIcePerf(iter, test, name, clientOpts, serverOpts):
    
    dir = os.path.join(toplevel, "src", "Ice", test)
    runClientServerPerf(iter, dir, "Ice " + test + " " + name, clientOpts, serverOpts)

print "NOTE: the test suite will run as long as you don't interrupt it.\n" + \
      "      Use Ctrl-C to interrupt it, it will print the average and\n" + \
      "      best results.\n"

iter = 1
while 1:
    try:
        runIcePerf(iter, "latency", "twoway 1tp", "twoway", threadPoolOne)
        runIcePerf(iter, "latency", "twoway 4tp", "twoway", threadPoolFour)
        runIcePerf(iter, "latency", "twoway tc", "twoway " + threadPerConnection, threadPerConnection)

        runIcePerf(iter, "latency", "oneway 1tp", "oneway", threadPoolOne)
        runIcePerf(iter, "latency", "oneway 4tp", "oneway", threadPoolFour)
        runIcePerf(iter, "latency", "oneway tc", "oneway " + threadPerConnection, threadPerConnection)

        runIcePerf(iter, "latency", "batch 1tp", "batch", threadPoolOne)
        runIcePerf(iter, "latency", "batch 4tp", "batch", threadPoolFour)
        runIcePerf(iter, "latency", "batch tc", "batch " + threadPerConnection, threadPerConnection)

        runIcePerf(iter, "throughput", "byte 1tp", "byte", threadPoolOne)
        runIcePerf(iter, "throughput", "byte 4tp", "byte", threadPoolFour)
        runIcePerf(iter, "throughput", "byte tc", "byte " + threadPerConnection, threadPerConnection)

        runIcePerf(iter, "throughput", "string seq 1tp", "stringSeq", threadPoolOne)
        runIcePerf(iter, "throughput", "string seq 4tp", "stringSeq", threadPoolFour)
        runIcePerf(iter, "throughput", "string seq tc", "stringSeq " + threadPerConnection, threadPerConnection)

        runIcePerf(iter, "throughput", "long string seq 1tp", "longStringSeq", threadPoolOne)
        runIcePerf(iter, "throughput", "long string seq 4tp", "longStringSeq", threadPoolFour)
        runIcePerf(iter, "throughput", "long string seq tc", "longStringSeq " + threadPerConnection, \
		   threadPerConnection)

        runIcePerf(iter, "throughput", "struct seq 1tp", "structSeq", threadPoolOne)
        runIcePerf(iter, "throughput", "struct seq 4tp", "structSeq", threadPoolFour)
        runIcePerf(iter, "throughput", "struct seq tc", "structSeq " + threadPerConnection, threadPerConnection)

        iter += 1
    except KeyboardInterrupt:
        printAllResults()
        break
