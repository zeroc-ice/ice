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

import sys, os, string

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

def clientServerTest(toplevel, name):

    testdir = os.path.join(toplevel, "test", name)
    server = os.path.join(testdir, "server")
    client = os.path.join(testdir, "client")

    print "starting server...",
    serverPipe = os.popen(os.path.join(testdir, "server --pid"))
    output = serverPipe.readline().strip()
    if not output:
        print "failed!"
        sys.exit(0)
    serverPids.append(int(output))
    print "ok"
    
    print "starting client...",
    clientPipe = os.popen(os.path.join(testdir, "client"))
    output = clientPipe.readline()
    if not output:
	print "failed!"
	TestUtil.killServers()
	sys.exit(0)
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

    print "starting collocated...",
    collocatedPipe = os.popen(os.path.join(testdir, "collocated"))
    output = collocatedPipe.read().strip()
    if not output:
        print "failed!"
        sys.exit(0)
    print "ok"
    print output
