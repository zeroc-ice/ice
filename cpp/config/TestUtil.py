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
" --Ice.PrintProcessId --Ice.PrintAdapterReady --Ice.WarnAboutServerExceptions --Ice.ServerIdleTime=30" + \
" --Ice.Security.Ssl.CertPath=TOPLEVELDIR/certs --Ice.Security.Ssl.Config=server_sslconfig.xml" + \
" --Ice.Protocol=" + protocol

clientOptions = \
" --Ice.Security.Ssl.CertPath=TOPLEVELDIR/certs --Ice.Security.Ssl.Config=client_sslconfig.xml" + \
" --Ice.Protocol=" + protocol

clientServerOptions = \
" --Ice.PrintProcessId --Ice.PrintAdapterReady --Ice.WarnAboutServerExceptions --Ice.ServerIdleTime=30" + \
" --Ice.Security.Ssl.CertPath=TOPLEVELDIR/certs --Ice.Security.Ssl.Config=sslconfig.xml --Ice.Protocol=" + protocol

collocatedOptions = \
" --Ice.Security.Ssl.CertPath=TOPLEVELDIR/certs --Ice.Security.Ssl.Config=sslconfig.xml --Ice.Protocol=" + protocol

serverPids = []

def killServers():

    global serverPids

    for pid in serverPids:
        if sys.platform == "cygwin" or sys.platform == "win32":
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
