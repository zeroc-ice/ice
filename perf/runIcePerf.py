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
    print "usage: " + sys.argv[0] + " [-h|--help] [-i|--iter N] [-n|--hostname HOSTNAME] [-o|--output FILENAME]"
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
    print " -n | -hostname        Defines the hostname."
    print " -o | -output          Defines the name of the output file."
    print ""
    sys.exit(2)
    

#
# Ice configuration properties.
#
threadPoolOne = " --Ice.ThreadPool.Server.Size=1 --Ice.ThreadPool.Server.SizeMax=1 --Ice.ThreadPool.Server.SizeWarn=1"
threadPoolFour = " --Ice.ThreadPool.Server.Size=4 --Ice.ThreadPool.Server.SizeMax=4 --Ice.ThreadPool.Server.SizeWarn=0"
threadPerConnection = " --Ice.ThreadPerConnection"

def runIcePerfs(expr, results, i):

    test = TestUtil.Test(expr, results, i, "Ice", "latency twoway")
    test.run("1tp", "latency", "twoway", threadPoolOne)
    test.run("4tp", "latency", "twoway", threadPoolFour)
    test.run("tpc", "latency", "twoway " + threadPerConnection, threadPerConnection)
    
    test = TestUtil.Test(expr, results, i, "Ice", "latency oneway")
    test.run("1tp", "latency", "oneway", threadPoolOne)
    test.run("4tp", "latency", "oneway", threadPoolFour)
    test.run("tpc", "latency", "oneway " + threadPerConnection, threadPerConnection)
    
    test = TestUtil.Test(expr, results, i, "Ice", "latency oneway")
    test.run("1tp (batch)", "latency", "batch", threadPoolOne)
    test.run("4tp (batch)", "latency", "batch", threadPoolFour)
    test.run("tpc (batch)", "latency", "batch " + threadPerConnection, threadPerConnection)

    test = TestUtil.Test(expr, results, i, "Ice", "latency twoway AMI")
    test.run("1tp", "latency", "twoway ami", threadPoolOne)
    test.run("4tp", "latency", "twoway ami", threadPoolFour)
    test.run("tpc", "latency", "twoway ami " + threadPerConnection, threadPerConnection)

    test = TestUtil.Test(expr, results, i, "Ice", "throughput byte")
    test.run("1tp", "throughput", "byte", threadPoolOne)
    test.run("4tp", "throughput", "byte", threadPoolFour)
    test.run("tpc", "throughput", "byte " + threadPerConnection, threadPerConnection)
    
    test = TestUtil.Test(expr, results, i, "Ice", "throughput string seq")
    test.run("1tp", "throughput", "stringSeq", threadPoolOne)
    test.run("4tp", "throughput", "stringSeq", threadPoolFour)
    test.run("tpc", "throughput", "stringSeq " + threadPerConnection, threadPerConnection)
    
    test = TestUtil.Test(expr, results, i, "Ice", "throughput long string seq")
    test.run("1tp", "throughput", "longStringSeq", threadPoolOne)
    test.run("4tp", "throughput", "longStringSeq", threadPoolFour)
    test.run("tpc", "throughput", "longStringSeq " + threadPerConnection, threadPerConnection)
    
    test = TestUtil.Test(expr, results, i, "Ice", "throughput struct seq")
    test.run("1tp", "throughput", "structSeq", threadPoolOne)
    test.run("4tp", "throughput", "structSeq", threadPoolFour)
    test.run("tpc", "throughput", "structSeq " + threadPerConnection, threadPerConnection)

def runTAOPerfs(expr, results, i):

    test = TestUtil.Test(expr, results, i, "TAO", "latency twoway")
    test.run("1tp", "Thread_Pool", "latency twoway", "1")
    test.run("4tp", "Thread_Pool", "latency twoway", "4")
    test.run("tpc", "Thread_Per_Connection", "latency twoway", "")

    test = TestUtil.Test(expr, results, i, "TAO", "latency oneway")
    test.run("1tp", "Thread_Pool", "latency oneway", "1")
    test.run("4tp", "Thread_Pool", "latency oneway", "4")
    test.run("tpc", "Thread_Per_Connection", "latency oneway", "")
    
    test = TestUtil.Test(expr, results, i, "TAO", "throughput byte")
    test.run("1tp", "Thread_Pool", "throughput byte", "1")
    test.run("4tp", "Thread_Pool", "throughput byte", "4")
    test.run("tpc", "Thread_Per_Connection", "throughput byte", "")
    
    test = TestUtil.Test(expr, results, i, "TAO", "throughput string seq")
    test.run("1tp", "Thread_Pool", "throughput string", "1")
    test.run("4tp", "Thread_Pool", "throughput string", "4")
    test.run("tpc", "Thread_Per_Connection", "throughput string", "")
    
    test = TestUtil.Test(expr, results, i, "TAO", "throughput long string seq")
    test.run("1tp", "Thread_Pool", "throughput longString", "1")
    test.run("4tp", "Thread_Pool", "throughput longString", "4")
    test.run("tpc", "Thread_Per_Connection", "throughput longString", "")
    
    test = TestUtil.Test(expr, results, i, "TAO", "throughput struct seq")
    test.run("1tp", "Thread_Pool", "throughput struct", "1")
    test.run("4tp", "Thread_Pool", "throughput struct", "4")
    test.run("tpc", "Thread_Per_Connection", "throughput struct", "")

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
    outputFile = "results." + sys.platform + "." + hostname

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
while i < niter:
    try:
        if os.environ.has_key('ICE_HOME'):
            runIcePerfs(expr, results, i)
        if os.environ.has_key('TAO_ROOT'):
            runTAOPerfs(expr, results, i)
        i += 1
    except KeyboardInterrupt:
        break

print "\n"
print "All results:"
all = TestUtil.AllResults()
all.add(results)
all.printAll(False)
