#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2002
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
#
# **********************************************************************

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

import sys, os

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
        
# Only used for C++ programs
serverPids = []
def killServers():

    global serverPids

    for pid in serverPids:
        if isCygwin():
            print "killServers(): not implemented for cygwin python."
            sys.exit(1)
        elif isWin32():
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
            
# Only used for C++ programs
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

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sep = ""
if isWin32():
    sep = ";"
else:
    sep = ":"

os.environ["CLASSPATH"] = os.path.join(toplevel, "lib") + sep + os.environ["CLASSPATH"]

clientProtocol = ""
serverProtocol = ""
clientServerProtocol = ""

if host != "":
    defaultHost = " --Ice.Default.Host=" + host
else:
    defaultHost = ""

commonServerOptions = " --Ice.PrintAdapterReady --Ice.ServerThreadPool.Size=3" + \
                      " --Ice.Warn.Connection --Ice.ServerIdleTime=30"

clientOptions = clientProtocol + defaultHost
serverOptions = serverProtocol + defaultHost + commonServerOptions
clientServerOptions = clientServerProtocol + defaultHost + commonServerOptions
collocatedOptions = clientServerProtocol + defaultHost

def clientServerTestWithOptions(name, additionalServerOptions, additionalClientOptions):

    testdir = os.path.join(toplevel, "test", name)
    server = "java -ea Server --Ice.ProgramName=Server "
    client = "java -ea Client --Ice.ProgramName=Client "

    print "starting server...",
    serverPipe = os.popen(server + serverOptions + additionalServerOptions)
    getAdapterReady(serverPipe)
    print "ok"
    
    print "starting client...",
    clientPipe = os.popen(client + clientOptions + additionalClientOptions)
    print "ok"

    for output in clientPipe.xreadlines():
	print output,

    clientStatus = clientPipe.close()
    serverStatus = serverPipe.close()

    if clientStatus or serverStatus:
	killServers()
	sys.exit(1)

def clientServerTest(name):

    clientServerTestWithOptions(name, "", "")

def mixedClientServerTestWithOptions(name, additionalServerOptions, additionalClientOptions):

    testdir = os.path.join(toplevel, "test", name)
    server = "java -ea Server --Ice.ProgramName=Server "
    client = "java -ea Client --Ice.ProgramName=Client "

    print "starting server...",
    serverPipe = os.popen(server + serverOptions + additionalServerOptions)
    getAdapterReady(serverPipe)
    print "ok"
    
    print "starting client...",
    clientPipe = os.popen(client + clientOptions + additionalClientOptions)
    print "ok"

    for output in clientPipe.xreadlines():
	print output,

    clientStatus = clientPipe.close()
    serverStatus = serverPipe.close()

    if clientStatus or serverStatus:
	killServers()
	sys.exit(1)

def mixedClientServerTest(name):

    mixedClientServerTestWithOptions(name, "", "")

def collocatedTestWithOptions(name, additionalOptions):

    testdir = os.path.join(toplevel, "test", name)
    collocated = "java -ea Collocated --Ice.ProgramName=Collocated "

    print "starting collocated...",
    collocatedPipe = os.popen(collocated + collocatedOptions + additionalOptions)
    print "ok"

    for output in collocatedPipe.xreadlines():
	print output,

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
