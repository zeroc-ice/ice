#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# TODO:
# - should add appropriate lib directories to LD_LIBRARY_PATH or PATH as
# the platform warrants.
# - convert to use the python CSV module.
#

import os, sys, getopt, re, platform, time

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
    print "defined the Ice tests will be run. If the TAO_HOME environment variable is"
    print "defined the TAO tests will be run. The results are stored in the file named"
    print "runIcePerf.results. This file get overwritten each time the performance tests"
    print "are run. If you want to print the results stored in this file, you can use"
    print "the -p or --print-results command line option."
    print ""
    print "Command line options:"
    print ""
    print " -h | --help           Print this help message."
    print " -i | --iter           Defines the number of the test will be run (default once)."
    print " -n | -hostname        Defines the hostname."
    print " -o | -output          Defines the name of the output file."
    print " -csv 		  Create a CSV format data file"
    print ""
    sys.exit(2)
    
class ClientServerTest(TestUtil.Test) :

    def __init__(self, product, test, directory = ""):
	TestUtil.Test.__init__(self, product, test, directory)

    def run(self, name, directory, clientOptions, serverOptions, topics):

        return TestUtil.Test.run(self, name, { "client" : clientOptions, "server" : serverOptions, "directory" : directory }, topics)

    def execute(self, options):
        
        cwd = os.getcwd()
        os.chdir(os.path.join(toplevel, "src", self.directory, options["directory"]))

	redirectStdErr = " 2>/dev/null"
	if TestUtil.isWin32():
	    redirectStdErr = " 2>NUL"

        serverPipe = os.popen(os.path.join(".", "server") + " " + options["server"] + redirectStdErr)
        TestUtil.getAdapterReady(serverPipe)

        clientPipe = os.popen(os.path.join(".", "client") + " " + options["client"])
	data = clientPipe.read()
	try:
	    try:
		result = eval(data)
	    finally:
		clientPipe.close()
		TestUtil.printOutputFromPipe(serverPipe);
		serverPipe.close()        
		os.chdir(cwd)
	except:
	    print data
	    raise


        return result

try:
    opts, pargs = getopt.getopt(sys.argv[1:], 'hi:o:n:', ['help', 'iter=', 'output=', 'hostname=']);
except getopt.GetoptError:
    usage()

niter = 1
hostname = ""
filename = ""
for o, a in opts:
    if o == '-i' or o == "--iter":
        niter = int(a)
    elif o == '-h' or o == "--help":
        usage()
    elif o == '-o' or o == "--output":
        filename = a
    elif o == '-n' or o == "--hostname":
        hostname = a

if filename == "":
    (system, name, ver, build, machine, processor) = platform.uname()
    if hostname == "":
        hostname = name
        if hostname.find('.'):
            hostname = hostname[0:hostname.find('.')]
    filename = ("results.ice." + system + "." + hostname).lower()

expr = [ ]
if len(pargs) > 0:
    for e in pargs:
        expr.append(re.compile(e))

if not os.environ.has_key('ICE_HOME'):
    if os.path.exists(os.path.join(toplevel, "..", "ice")):
        os.environ['ICE_HOME'] = os.path.join(toplevel, "..", "ice")

if not os.environ.has_key('ICE_HOME') and \
   not os.environ.has_key('TAO_ROOT') and \
   not os.environ.has_key('ICEE_HOME'):
    print "You need to set at least ICE_HOME, ICEE_HOME or TAO_ROOT!"
    sys.exit(1)
    
configs = [ ("IceTests", "ICE_HOME"), ("TAOTests", "TAO_ROOT"), ("IceETests", "ICEE_HOME")]

tests = []
for f, e in configs:
    if os.path.exists(f + ".py") and os.environ.has_key(e):
	m = __import__(f)
	tests.extend(m.getDefinitions())

# 
# Filter tests for those that match our pattern.
#
if len(expr) > 0:
    candidates = tests
    tests = []
    for product, group, dir, cases in candidates:
	allowedCases = []
	for e in expr:
	    for c in cases:
		criteria = "%s %s %s" % (product, group, c[0])
		if e.match(criteria):
		    allowedCases.append(c)
	if len(allowedCases) > 0:
	    tests.append((product, group, dir, allowedCases))

rawResults = []

i = 1
while i <= niter:
    try:
	for product, group, dir, cases in tests:
	    test = ClientServerTest(product, group)
	    for c in cases:
		additionalArgs = "" 
		for arg, value in c[3]:
		    additionalArgs = " %s --%s=%s" % (additionalArgs, arg, value)

		try:
		    result = test.run(c[0], dir, c[1] + additionalArgs, c[2], c[4])
		    time.sleep(2)
		    rawResults.append(result)
		except SyntaxError, e:
		    #
		    # Syntax errors may occur if something goes wrong
		    # with the test programs. If so, we'll reject the
		    # result on the basis that we can't interpet the
		    # output anyway.
		    # 
		    print e

    except KeyboardInterrupt:
	break
    i += 1

print "\n"

#
# Save results
#
outputFile = file(filename, 'w+b')
outputFile.write(str(rawResults))
outputFile.close()

TestUtil.PrintResults(rawResults, filename, [('Ice', 'TAO'), ('IceE', 'TAO'), ('IceE', 'Ice')])
