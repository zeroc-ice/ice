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

serverOptions = \
" --Ice.PrintProcessId --Ice.PrintAdapterReady --Ice.WarnAboutServerExceptions=1 --Ice.ServerIdleTime=30" + \
" --Ice.Ssl.CertPath=TOPLEVELDIR/certs --Ice.Ssl.Config=server_sslconfig.xml --Ice.Protocol=" + protocol

clientOptions = \
" --Ice.Ssl.CertPath=TOPLEVELDIR/certs --Ice.Ssl.Config=client_sslconfig.xml --Ice.Protocol=" + protocol

clientServerOptions = \
" --Ice.PrintProcessId --Ice.PrintAdapterReady --Ice.WarnAboutServerExceptions=1 --Ice.ServerIdleTime=30" + \
" --Ice.Ssl.CertPath=TOPLEVELDIR/certs --Ice.Ssl.Config=sslconfig.xml --Ice.Protocol=" + protocol

collocatedOptions = \
" --Ice.Ssl.CertPath=TOPLEVELDIR/certs --Ice.Ssl.Config=sslconfig.xml --Ice.Protocol=" + protocol

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

    testdir = os.path.normpath(toplevel + "/test/" + name)
    collocated = os.path.normpath(testdir + "/collocated")

    updatedCollocatedOptions = collocatedOptions.replace("TOPLEVELDIR", toplevel)

    print "starting collocated...",
    collocatedPipe = os.popen(collocated + updatedCollocatedOptions)
    output = collocatedPipe.read().strip()
    if not output:
        print "failed!"
        sys.exit(1)
    print "ok"
    print output
