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

import sys, os

protocol = "tcp"
serverOptions = " --Ice.Protocol=" + protocol + " --Ice.Trace.Security=0 --Ice.PrintProcessId --Ice.PrintAdapterReady --Ice.ServerIdleTime=60 --Ice.Ssl.Config=TOPLEVELDIR/Certs/server_sslconfig.xml"
clientOptions = " --Ice.Protocol=" + protocol + " --Ice.Trace.Security=0 --Ice.Ssl.Config=TOPLEVELDIR/Certs/client_sslconfig.xml"
collocatedOptions = " --Ice.Protocol=" + protocol + " --Ice.Trace.Security=0 --Ice.PrintProcessId --Ice.PrintAdapterReady --Ice.ServerIdleTime=60 --Ice.Ssl.Config=TOPLEVELDIR/Certs/sslconfig.xml"

serverPids = []

def killServers():

    global serverPids

    for pid in serverPids:
        if os.name == "nt":
            import win32api
            handle = win32api.OpenProcess(1, 0, pid)
            return (0 != win32api.TerminateProcess(handle, 0))
        else:
            os.kill(pid, 9)

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

    testdir = os.path.normpath(toplevel + "/test/" + name)
    server = os.path.normpath(testdir + "/server")
    client = os.path.normpath(testdir + "/client")

    updatedServerOptions = serverOptions
    updatedServerOptions = updatedServerOptions.replace("TOPLEVELDIR", toplevel)
    print "starting server...",
    serverPipe = os.popen(server + updatedServerOptions)
    getServerPid(serverPipe)
    getAdapterReady(serverPipe)
    print "ok"
    
    updatedClientOptions = clientOptions
    updatedClientOptions = updatedClientOptions.replace("TOPLEVELDIR", toplevel)
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

    testdir = os.path.normpath(toplevel + "/test/" + name)
    collocated = os.path.normpath(testdir + "/collocated")

    updatedCollocatedOptions = collocatedOptions
    updatedCollocatedOptions = updatedCollocatedOptions.replace("TOPLEVELDIR", toplevel)

    command = collocated + " " + updatedCollocatedOptions
    print "starting collocated...",
    collocatedPipe = os.popen(command)
    output = collocatedPipe.read().strip()
    if not output:
        print "failed!"
        sys.exit(1)
    print "ok"
    print output
