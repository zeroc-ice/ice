#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Set protocol to "ssl" in case you want to run the tests with the SSL
# protocol. Otherwise TCP is used.
#

protocol = ""
#protocol = "ssl"	// TODO: Not implemented yet

#
# Set compressed to 1 in case you want to run the tests with
# protocol compression.
#

compress = 0
#compress = 1		// TODO: Not implemented yet

#
# Set the host to the host name the test servers are running on. If
# not set, Ice will try to find out the IP address for the
# hostname. If you DNS isn't set up propertly, you should therefore
# use "localhost".
#

#host = "someotherhost"
host = "localhost"

#
# Don't change anything below this line!
#

import sys, os, re

def isCygwin():

    # The substring on sys.platform is required because some cygwin
    # versions return variations like "cygwin_nt-4.01".
    if sys.platform[:6] == "cygwin":
        return 1
    else:
        return 0

def isWin32():

    if sys.platform == "win32" or isCygwin():
        return 1
    else:
        return 0

serverPids = []
def killServers():

    global serverPids

    if isCygwin():
	print "killServers(): not implemented for cygwin python."

	#
	# TODO: Michi: Not sure why exit(1) was here. This means that, when
	# we run the test suite with allTests.py under Cygwin, the first sub-test that
	# calls killServers will return non-zero exit status and, therefore,
	# terminate allTests.py, so the subsequence tests are never run.
	#
	#sys.exit(1)

    for pid in serverPids:
        if isWin32():
            try:
                import win32api
                handle = win32api.OpenProcess(1, 0, pid)
                win32api.TerminateProcess(handle, 0)
            except:
                pass # Ignore errors, such as non-existing processes.
        else:
            try:
                os.kill(pid, 9)
            except:
                pass # Ignore errors, such as non-existing processes.

    serverPids = []

def getServerPid(serverPipe):

    output = serverPipe.readline().strip()

    if not output:
        print "failed!"
        killServers()
        sys.exit(1)

    serverPids.append(int(output))

def getAdapterReady(serverPipe):

    output = serverPipe.readline().strip()

    if not output:
        print "failed!"
        killServers()
        sys.exit(1)

def waitServiceReady(pipe, token):

    while True:

        output = pipe.readline().strip()

        if not output:
            print "failed!"
            sys.exit(1)

        if output == token + " ready":
            break

def printOutputFromPipe(pipe):

    while True:

        c = pipe.read(1)

        if c == "":
            break

        os.write(1, c)

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

if isWin32():
    if isCygwin():
	os.environ["PATH"] = os.path.join(toplevel, "bin") + ":" + os.getenv("PATH", "")
    else:
	os.environ["PATH"] = os.path.join(toplevel, "bin") + ";" + os.getenv("PATH", "")

if protocol == "ssl":
    plugin		 = " --Ice.Plugin.IceSSL=IceSSL:create"
    clientProtocol       = plugin + " --Ice.Default.Protocol=ssl" + \
                           " --IceSSL.Client.CertPath=" + os.path.join(toplevel, "certs") + \
                           " --IceSSL.Client.Config=client_sslconfig.xml"
    serverProtocol       = plugin + " --Ice.Default.Protocol=ssl" + \
                           " --IceSSL.Server.CertPath=" + os.path.join(toplevel, "certs") + \
                           " --IceSSL.Server.Config=server_sslconfig.xml"
    clientServerProtocol = plugin + " --Ice.Default.Protocol=ssl" + \
                           " --IceSSL.Client.CertPath=" + os.path.join(toplevel, "certs") + \
                           " --IceSSL.Client.Config=sslconfig.xml" + \
                           " --IceSSL.Server.CertPath=" + os.path.join(toplevel, "certs") + \
                           " --IceSSL.Server.Config=sslconfig.xml"
else:
    clientProtocol = ""
    serverProtocol = ""
    clientServerProtocol = ""

if compress:
    clientProtocol += " --Ice.Override.Compress"
    serverProtocol += " --Ice.Override.Compress"
    clientServerProtocol += " --Ice.Override.Compress"

if host != "":
    defaultHost = " --Ice.Default.Host=" + host
else:
    defaultHost = ""

commonClientOptions = " --Ice.NullHandleAbort --Ice.Warn.Connections"

commonServerOptions = " --Ice.PrintProcessId --Ice.PrintAdapterReady --Ice.NullHandleAbort" + \
                      " --Ice.Warn.Connections --Ice.ServerIdleTime=30" + \
                      " --Ice.ThreadPool.Server.Size=1 --Ice.ThreadPool.Server.SizeMax=3" + \
                      " --Ice.ThreadPool.Server.SizeWarn=0"

clientOptions = clientProtocol + defaultHost + commonClientOptions
serverOptions = serverProtocol + defaultHost + commonServerOptions
clientServerOptions = clientServerProtocol + defaultHost + commonServerOptions
collocatedOptions = clientServerProtocol + defaultHost

def clientServerTestWithOptionsAndNames(name, additionalServerOptions, additionalClientOptions, \
                                        serverName, clientName):

    testDir = os.path.join(toplevel, "test", name)
    server = os.path.basename(serverName)
    serverDir = os.path.dirname(os.path.join(testDir, serverName))
    client = os.path.basename(clientName)
    clientDir = os.path.dirname(os.path.join(testDir, clientName))

    cwd = os.getcwd()
    
    print "starting " + serverName + "...",
    os.chdir(serverDir)
    serverPipe = os.popen(os.path.join(".", server) + serverOptions + " " + additionalServerOptions)
    getServerPid(serverPipe)
    getAdapterReady(serverPipe)
    print "ok"
    
    os.chdir(cwd)
    
    print "starting " + clientName + "...",
    os.chdir(clientDir);
    clientPipe = os.popen(os.path.join(".", client) + clientOptions + " " + additionalClientOptions)
    print "ok"

    os.chdir(cwd)

    printOutputFromPipe(clientPipe)

    clientStatus = clientPipe.close()
    serverStatus = serverPipe.close()

    if clientStatus or serverStatus:
	killServers()
	sys.exit(1)

def clientServerTestWithOptions(name, additionalServerOptions, additionalClientOptions):

    clientServerTestWithOptionsAndNames(name, additionalServerOptions, additionalClientOptions, "server", "client")

def clientServerTest(name):

    clientServerTestWithOptions(name, "", "")

def mixedClientServerTestWithOptions(name, additionalServerOptions, additionalClientOptions):

    testDir = os.path.join(toplevel, "test", name)

    cwd = os.getcwd()

    print "starting server...",
    os.chdir(testDir);
    serverPipe = os.popen(os.path.join(".", "server") + clientServerOptions + " " + additionalServerOptions)
    getServerPid(serverPipe)
    getAdapterReady(serverPipe)
    print "ok"
    
    os.chdir(cwd)

    print "starting client...",
    os.chdir(testDir);
    clientPipe = os.popen(os.path.join(".", "client") + clientServerOptions + " " + additionalClientOptions)
    getServerPid(clientPipe)
    getAdapterReady(clientPipe)
    print "ok"

    os.chdir(cwd)

    printOutputFromPipe(clientPipe)

    clientStatus = clientPipe.close()
    serverStatus = serverPipe.close()

    if clientStatus or serverStatus:
	killServers()
	sys.exit(1)

def mixedClientServerTest(name):

    mixedClientServerTestWithOptions(name, "", "")

def collocatedTestWithOptions(name, additionalOptions):

    testDir = os.path.join(toplevel, "test", name)
    collocatedDir = os.path.dirname(os.path.join(testDir, "collocated"))

    cwd = os.getcwd()

    os.chdir(collocatedDir)

    print "starting collocated...",
    collocatedPipe = os.popen(os.path.join(".", "collocated") + collocatedOptions + " " + additionalOptions)
    print "ok"

    os.chdir(cwd)

    printOutputFromPipe(collocatedPipe)

    collocatedStatus = collocatedPipe.close()

    if collocatedStatus:
	killServers()
	sys.exit(1)

def collocatedTest(name):

    collocatedTestWithOptions(name, "")

def cleanDbDir(path):

    files = os.listdir(path)

    for filename in files:
        if filename != "CVS" and filename != ".dummy":
            fullpath = os.path.join(path, filename);
            os.remove(fullpath)
