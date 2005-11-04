#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

#compress = 0
compress = 1

#
# Set threadPerConnection to 1 in case you want to run the tests in
# thread per connection mode.
#

threadPerConnection = 0
#threadPerConnection = 1

#
# If you don't set "host" below, then the Ice library will try to find
# out the IP address of this host. For the Ice test suite, it's best
# to set the IP address explicitly to 127.0.0.1. This avoid problems
# with incorrect DNS or hostname setups.
#

host = "127.0.0.1"

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
        return;

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

    while 1:

        output = pipe.readline().strip()

        if not output:
            print "failed!"
            sys.exit(1)

        if output == token + " ready":
            break

def printOutputFromPipe(pipe):

    while 1:

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
    cppPlugin		    = " --Ice.Plugin.IceSSL=IceSSL:create"
    cppClientProtocol       = cppPlugin + " --Ice.Default.Protocol=ssl" + \
                              " --IceSSL.Client.CertPath=" + os.path.join(toplevel, "certs") + \
                              " --IceSSL.Client.Config=client_sslconfig.xml"
    cppServerProtocol       = cppPlugin + " --Ice.Default.Protocol=ssl" + \
                              " --IceSSL.Server.CertPath=" + os.path.join(toplevel, "certs") + \
                              " --IceSSL.Server.Config=server_sslconfig.xml"
    cppClientServerProtocol = cppPlugin + " --Ice.Default.Protocol=ssl" + \
                           " --IceSSL.Client.CertPath=" + os.path.join(toplevel, "certs") + \
                           " --IceSSL.Client.Config=sslconfig.xml" + \
                           " --IceSSL.Server.CertPath=" + os.path.join(toplevel, "certs") + \
                           " --IceSSL.Server.Config=sslconfig.xml"
else:
    clientProtocol = ""
    serverProtocol = ""
    clientServerProtocol = ""
    cppClientProtocol = ""
    cppServerProtocol = ""
    cppClientServerProtocol = ""

if compress:
    clientProtocol += " --Ice.Override.Compress"
    serverProtocol += " --Ice.Override.Compress"
    clientServerProtocol += " --Ice.Override.Compress"
    cppClientProtocol += " --Ice.Override.Compress"
    cppServerProtocol += " --Ice.Override.Compress"
    cppClientServerProtocol += " --Ice.Override.Compress"

if threadPerConnection:
    clientProtocol += " --Ice.ThreadPerConnection"
    serverProtocol += " --Ice.ThreadPerConnection"
    clientServerProtocol += " --Ice.ThreadPerConnection"
    cppClientProtocol += " --Ice.ThreadPerConnection"
    cppServerProtocol += " --Ice.ThreadPerConnection"
    cppClientServerProtocol += " --Ice.ThreadPerConnection"

if host != "":
    defaultHost = " --Ice.Default.Host=" + host
else:
    defaultHost = ""

commonClientOptions = " --Ice.NullHandleAbort --Ice.Warn.Connections"

commonServerOptions = " --Ice.PrintProcessId --Ice.PrintAdapterReady --Ice.NullHandleAbort" + \
                      " --Ice.Warn.Connections --Ice.ServerIdleTime=10" + \
                      " --Ice.ThreadPool.Server.Size=1 --Ice.ThreadPool.Server.SizeMax=3" + \
                      " --Ice.ThreadPool.Server.SizeWarn=0"

cppCommonClientOptions = " --Ice.Warn.Connections"

cppCommonServerOptions = " --Ice.PrintAdapterReady" + \
                         " --Ice.Warn.Connections --Ice.ServerIdleTime=30" + \
                         " --Ice.ThreadPool.Server.Size=1 --Ice.ThreadPool.Server.SizeMax=3" + \
                         " --Ice.ThreadPool.Server.SizeWarn=0"

cppClientOptions = cppClientProtocol + defaultHost + cppCommonClientOptions
cppServerOptions = cppServerProtocol + defaultHost + cppCommonServerOptions
cppClientServerOptions = cppClientServerProtocol + defaultHost + cppCommonServerOptions

clientOptions = clientProtocol + defaultHost + commonClientOptions
serverOptions = serverProtocol + defaultHost + commonServerOptions
clientServerOptions = clientServerProtocol + defaultHost + commonServerOptions
collocatedOptions = clientServerProtocol + defaultHost

def createMsg(mono, name):
    
    msg = "starting "
    if mono:
	msg += "mono "
    msg += name
    if mono:
        msg += ".exe"
    msg += "..."

    return msg

def createCmd(mono, bin):

    if mono:
        return "mono --debug " + bin + ".exe"
    else:
        return bin

def clientServerTestWithOptionsAndNames(mono, name, additionalServerOptions, additionalClientOptions, \
                                        serverName, clientName):

    testdir = os.path.join(toplevel, "test", name)
    server = os.path.join(testdir, serverName)
    client = os.path.join(testdir, clientName)

    print createMsg(mono, serverName),

    serverPipe = os.popen(createCmd(mono, server) + serverOptions + " " + additionalServerOptions)
    getServerPid(serverPipe)
    getAdapterReady(serverPipe)
    print "ok"
    
    print createMsg(mono, clientName),
    clientPipe = os.popen(createCmd(mono, client) + clientOptions + " " + additionalClientOptions)
    print "ok"

    printOutputFromPipe(clientPipe)

    clientStatus = clientPipe.close()
    serverStatus = serverPipe.close()

    if clientStatus or serverStatus:
	killServers()
	sys.exit(1)

def clientServerTestWithOptions(mono, name, additionalServerOptions, additionalClientOptions):

    clientServerTestWithOptionsAndNames(mono, name, additionalServerOptions, additionalClientOptions, "server", "client")

def clientServerTest(mono, name):

    clientServerTestWithOptions(mono, name, "", "")

def mixedClientServerTestWithOptions(mono, name, additionalServerOptions, additionalClientOptions):

    testdir = os.path.join(toplevel, "test", name)
    server = os.path.join(testdir, "server")
    client = os.path.join(testdir, "client")

    print createMsg(mono, "server"),
    serverPipe = os.popen(createCmd(mono, server) + clientServerOptions + " " + additionalServerOptions)
    getServerPid(serverPipe)
    getAdapterReady(serverPipe)
    print "ok"
    
    print createMsg(mono, "client"),
    clientPipe = os.popen(createCmd(mono, client) + clientServerOptions + " " + additionalClientOptions)
    getServerPid(clientPipe)
    getAdapterReady(clientPipe)
    print "ok"

    printOutputFromPipe(clientPipe)

    clientStatus = clientPipe.close()
    serverStatus = serverPipe.close()

    if clientStatus or serverStatus:
	killServers()
	sys.exit(1)

def mixedClientServerTest(mono, name):

    mixedClientServerTestWithOptions(mono, name, "", "")

def collocatedTestWithOptions(mono, name, additionalOptions):

    testdir = os.path.join(toplevel, "test", name)
    collocated = os.path.join(testdir, "collocated")

    print createMsg(mono, "collocated"),
    collocatedPipe = os.popen(createCmd(mono, collocated) + collocatedOptions + " " + additionalOptions)
    print "ok"

    printOutputFromPipe(collocatedPipe)

    collocatedStatus = collocatedPipe.close()

    if collocatedStatus:
	killServers()
	sys.exit(1)

def collocatedTest(mono, name):

    collocatedTestWithOptions(mono, name, "")

def clientTestWithOptions(mono, name, additionalOptions):

    testdir = os.path.join(toplevel, "test", name)
    client = os.path.join(testdir, "client")

    print createMsg(mono, "client"),
    clientPipe = os.popen(createCmd(mono, client) + clientOptions + " " + additionalOptions)
    print "ok"

    printOutputFromPipe(clientPipe)

    clientStatus = clientPipe.close()

    if clientStatus:
	killServers()
	sys.exit(1)

def clientTest(mono, name):

    clientTestWithOptions(mono, name, "")

def cleanDbDir(path):

    files = os.listdir(path)

    for filename in files:
        if filename != "CVS" and filename != ".dummy":
            fullpath = os.path.join(path, filename);
            os.remove(fullpath)
