#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2001
# MutableRealms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

#
# Set protocol to "ssl" in case you want to run the tests with the SSL
# protocol. Otherwise TCP is used.
#

protocol = "ssl"
#protocol = ""

#
# Set the host to the host name the test servers are running on. If not
# set, the local host is used.
#

#host = "someotherhost"
host = ""

#
# Don't change anything below this line!
#

if protocol == "ssl":
    clientProtocol = " --Ice.DefaultProtocol=ssl" + \
    " --Ice.SSL.Client.CertPath=TOPLEVELDIR/certs --Ice.SSL.Client.Config=client_sslconfig.xml"
    serverProtocol = " --Ice.DefaultProtocol=ssl" + \
    " --Ice.SSL.Server.CertPath=TOPLEVELDIR/certs --Ice.SSL.Server.Config=server_sslconfig.xml"
    clientServerProtocol = " --Ice.DefaultProtocol=ssl" + \
    " --Ice.SSL.Client.CertPath=TOPLEVELDIR/certs --Ice.SSL.Client.Config=sslconfig.xml" + \
    " --Ice.SSL.Server.CertPath=TOPLEVELDIR/certs --Ice.SSL.Server.Config=sslconfig.xml"
else:
    clientProtocol = ""
    serverProtocol = ""
    clientServerProtocol = ""

if host != "":
    defaultHost = " --Ice.DefaultHost=" + host
else:
    defaultHost = ""

commonServerOptions = \
" --Ice.PrintProcessId --Ice.PrintAdapterReady --Ice.ConnectionWarnings --Ice.ServerIdleTime=30"

serverOptions = commonServerOptions + serverProtocol
clientOptions = clientProtocol + defaultHost
clientServerOptions = commonServerOptions + clientServerProtocol + defaultHost
collocatedOptions = clientServerProtocol

import sys, os

serverPids = []

def killServers():

    global serverPids

    for pid in serverPids:
        if sys.platform == "cygwin":
            print "killServers(): not implemented for cygwin python."
            sys.exit(1)
        elif sys.platform == "win32":
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

def clientServerTest(toplevel, name):

    testdir = os.path.join(toplevel, "test", name)
    server = os.path.join(testdir, "server")
    client = os.path.join(testdir, "client")

    updatedServerOptions = serverOptions.replace("TOPLEVELDIR", toplevel)
    updatedClientOptions = clientOptions.replace("TOPLEVELDIR", toplevel)

    print "starting server...",
    serverPipe = os.popen(server + updatedServerOptions)
    getServerPid(serverPipe)
    getAdapterReady(serverPipe)
    print "ok"
    
    print "starting client...",
    clientPipe = os.popen(client + updatedClientOptions)
    output = clientPipe.readline()
    if not output:
	print "failed!"
	killServers()
	sys.exit(1)
    print "ok"
    print output,
    while 1:
	output = clientPipe.readline()
	if not output:
	    break;
	print output,

def collocatedTest(toplevel, name):

    testdir = os.path.join(toplevel, "test", name)
    collocated = os.path.join(testdir, "collocated")

    updatedCollocatedOptions = collocatedOptions.replace("TOPLEVELDIR", toplevel)

    print "starting collocated...",
    collocatedPipe = os.popen(collocated + updatedCollocatedOptions)
    output = collocatedPipe.read().strip()
    if not output:
        print "failed!"
        sys.exit(1)
    print "ok"
    print output

def cleanDbDir(path):

    files = os.listdir(path)

    for filename in files:
        if filename != "CVS" and filename != ".dummy":
            fullpath = os.path.join(path, filename);
            os.remove(fullpath)
