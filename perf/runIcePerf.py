#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, time, pickle, getopt, re

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

def usage():
    print "usage: " + sys.argv[0] + " [-h|--help] [-i|--iter N] [-p|--print-results]"
    print ""
    print "Run the Ice performance test suite. If the ICE_HOME environment variable is"
    print "defined the Ice tests will be run. If the TAO_ROOT environment variable is"
    print "defined the TAO tests will be run. The results are stored in the file named"
    print "runIcePerf.results. This file get overwritten each time the performance tests"
    print "are run. If you want to print the results stored in this file, you can use"
    print "the -p or --print-results command line option."
    print ""
    print "Command line options:"
    print ""
    print " -h | --help           Print this help message."
    print " -i | --iter           Defines the number of the test will be run."
    print " -p | --print-results  Print the stored results."
    print ""
    sys.exit(2)
    

#
# Ice configuration properties.
#
threadPoolOne = " --Ice.ThreadPool.Server.Size=1 --Ice.ThreadPool.Server.SizeMax=1 --Ice.ThreadPool.Server.SizeWarn=1"
threadPoolFour = " --Ice.ThreadPool.Server.Size=4 --Ice.ThreadPool.Server.SizeMax=4 --Ice.ThreadPool.Server.SizeWarn=0"
threadPerConnection = " --Ice.ThreadPerConnection"
resultsFile = "runIcePerf.results"

class Results :

    def __init__(self):
        self.results = { }
        self.resultNames = [ ]

    def add(self, name, result):

        if not name in self.resultNames:
            self.resultNames.append(name)

        if not self.results.has_key(name):
            self.results[name] = [ ]

        self.results[name].append(result)

        f = file(resultsFile, 'w')
        pickle.dump(self, f);
        f.close()
        
    def printAll(self):

        for n in self.resultNames:
            v = self.results[n]
            mean = 0.0
            best = max
            for r in v:
                mean += r
                if r < best:
                    best = r
            mean /= len(v)
        
            print "%-36s %f %f " % (n, mean, best)

results = Results()

def runClientServerPerf(expr, iter, directory, name, clientOpts, serverOpts):

    match = len(expr) == 0
    for e in expr:
        if e.match(name):
            match = True
            break;

    if not match:
        return
    
    cwd = os.getcwd()
    os.chdir(directory)

    print str(iter) + ": " + name + "...",
    sys.stdout.flush()
    
    serverPipe = os.popen(os.path.join(".", "server") + " " + serverOpts + " 2> /dev/null")
    TestUtil.getAdapterReady(serverPipe)

    clientPipe = os.popen(os.path.join(".", "client") + " " + clientOpts + " 2> /dev/null")
    result = float(clientPipe.read())
    clientPipe.close()

    os.chdir(cwd)

    results.add(name, result)
    
    print result

def runIcePerf(expr, iter, test, name, clientOpts, serverOpts):

    dir = os.path.join(toplevel, "src", "Ice", test)
    runClientServerPerf(expr, iter, dir, "Ice " + name, clientOpts, serverOpts)

def runTAOPerf(expr, iter, test, name, clientOpts, serverOpts):
    
    dir = os.path.join(toplevel, "src", "TAO", test)
    runClientServerPerf(expr, iter, dir, "TAO " + name, clientOpts, serverOpts)

def runIcePerfs(expr, i):
    
    runIcePerf(expr, i, "latency", "latency twoway 1tp", "twoway", threadPoolOne)
    runIcePerf(expr, i, "latency", "latency twoway 4tp", "twoway", threadPoolFour)
    runIcePerf(expr, i, "latency", "latency twoway tc", "twoway " + threadPerConnection, threadPerConnection)
    
    runIcePerf(expr, i, "latency", "latency twoway AMI 1tp", "twoway ami", threadPoolOne)
    runIcePerf(expr, i, "latency", "latency twoway AMI 4tp", "twoway ami", threadPoolFour)
    runIcePerf(expr, i, "latency", "latency twoway AMI tc", "twoway ami " + threadPerConnection, threadPerConnection)

    runIcePerf(expr, i, "latency", "latency oneway 1tp", "oneway", threadPoolOne)
    runIcePerf(expr, i, "latency", "latency oneway 4tp", "oneway", threadPoolFour)
    runIcePerf(expr, i, "latency", "latency oneway tc", "oneway " + threadPerConnection, threadPerConnection)
    
    runIcePerf(expr, i, "latency", "latency batch 1tp", "batch", threadPoolOne)
    runIcePerf(expr, i, "latency", "latency batch 4tp", "batch", threadPoolFour)
    runIcePerf(expr, i, "latency", "latency batch tc", "batch " + threadPerConnection, threadPerConnection)
    
    runIcePerf(expr, i, "throughput", "throughput byte 1tp", "byte", threadPoolOne)
    runIcePerf(expr, i, "throughput", "throughput byte 4tp", "byte", threadPoolFour)
    runIcePerf(expr, i, "throughput", "throughput byte tc", "byte " + threadPerConnection, threadPerConnection)
    
    runIcePerf(expr, i, "throughput", "throughput string seq 1tp", "stringSeq", threadPoolOne)
    runIcePerf(expr, i, "throughput", "throughput string seq 4tp", "stringSeq", threadPoolFour)
    runIcePerf(expr, i, "throughput", "throughput string seq tc", "stringSeq " + threadPerConnection, \
               threadPerConnection)
    
    runIcePerf(expr, i, "throughput", "throughput long string seq 1tp", "longStringSeq", threadPoolOne)
    runIcePerf(expr, i, "throughput", "throughput long string seq 4tp", "longStringSeq", threadPoolFour)
    runIcePerf(expr, i, "throughput", "throughput long string seq tc", "longStringSeq " + threadPerConnection, \
               threadPerConnection)
    
    runIcePerf(expr, i, "throughput", "throughput struct seq 1tp", "structSeq", threadPoolOne)
    runIcePerf(expr, i, "throughput", "throughput struct seq 4tp", "structSeq", threadPoolFour)
    runIcePerf(expr, i, "throughput", "throughput struct seq tc", "structSeq " + threadPerConnection, \
               threadPerConnection)

def runTAOPerfs(expr, i):
    
    runTAOPerf(expr, i, "Thread_Pool", "latency twoway 1tp", "latency twoway", "1")
    runTAOPerf(expr, i, "Thread_Pool", "latency twoway 4tp", "latency twoway", "4")
    runTAOPerf(expr, i, "Thread_Per_Connection", "latency twoway tc", "latency twoway", "")

    runTAOPerf(expr, i, "Thread_Pool", "latency oneway 1tp", "latency oneway", "1")
    runTAOPerf(expr, i, "Thread_Pool", "latency oneway 4tp", "latency oneway", "4")
    runTAOPerf(expr, i, "Thread_Per_Connection", "latency oneway tc", "latency oneway", "")
    
    runTAOPerf(expr, i, "Thread_Pool", "throughput byte 1tp", "throughput byte", "1")
    runTAOPerf(expr, i, "Thread_Pool", "throughput byte 4tp", "throughput byte", "4")
    runTAOPerf(expr, i, "Thread_Per_Connection", "throughput byte tc", "throughput byte", "")
    
    runTAOPerf(expr, i, "Thread_Pool", "throughput string seq 1tp", "throughput string", "1")
    runTAOPerf(expr, i, "Thread_Pool", "throughput string seq 4tp", "throughput string", "4")
    runTAOPerf(expr, i, "Thread_Per_Connection", "throughput string seq tc", "throughput string", "")
    
    runTAOPerf(expr, i, "Thread_Pool", "throughput long string seq 1tp", "throughput longString", "1")
    runTAOPerf(expr, i, "Thread_Pool", "throughput long string seq 4tp", "throughput longString", "4")
    runTAOPerf(expr, i, "Thread_Per_Connection", "throughput long string seq tc", "throughput longString", "")
    
    runTAOPerf(expr, i, "Thread_Pool", "throughput struct seq 1tp", "throughput struct", "1")
    runTAOPerf(expr, i, "Thread_Pool", "throughput struct seq 4tp", "throughput struct", "4")
    runTAOPerf(expr, i, "Thread_Per_Connection", "throughput struct seq tc", "throughput struct", "")

try:
    opts, pargs = getopt.getopt(sys.argv[1:], 'hi:p', ['help', 'iter=', 'print-results']);
except getopt.GetoptError:
    usage()

niter = max
for o, a in opts:
    if o == '-i' or o == "--iter":
        niter = int(a)
    elif o == '-h' or o == "--help":
        usage()
    elif o == '-p' or o == "--print-results":
        f = file(resultsFile)
        results = pickle.load(f);
        f.close()
        results.printAll()
        sys.exit(0)

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

i = 1        
while i < niter:
    try:
        if os.environ.has_key('ICE_HOME'):
            runIcePerfs(expr, i)
        if os.environ.has_key('TAO_ROOT'):
            runTAOPerfs(expr, i)
        i += 1
    except KeyboardInterrupt:
        break

print "\n"
print "All results:"
results.printAll()
