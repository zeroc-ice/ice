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

#
# Set protocol to "ssl" in case you want to run the tests with the SSL
# protocol. Otherwise TCP is used.
#

#protocol = "ssl"
protocol = ""

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
    clientProtocol = " --Ice.Default.Protocol=ssl" + \
    " --IceSSL.Client.CertPath=TOPLEVELDIR/certs --IceSSL.Client.Config=client_sslconfig.xml"
    serverProtocol = " --Ice.Default.Protocol=ssl" + \
    " --IceSSL.Server.CertPath=TOPLEVELDIR/certs --IceSSL.Server.Config=server_sslconfig.xml"
    clientServerProtocol = " --Ice.Default.Protocol=ssl" + \
    " --IceSSL.Client.CertPath=TOPLEVELDIR/certs --IceSSL.Client.Config=sslconfig.xml" + \
    " --IceSSL.Server.CertPath=TOPLEVELDIR/certs --IceSSL.Server.Config=sslconfig.xml"
else:
    clientProtocol = ""
    serverProtocol = ""
    clientServerProtocol = ""

if host != "":
    defaultHost = " --Ice.Default.Host=" + host
else:
    defaultHost = ""

sep = ""

if sys.platform == "win32":
    sep = ";"
elif sys.platform == "cygwin":
    sep = ";"
else:
    sep = ":"

commonServerOptions = \
" --Ice.PrintAdapterReady --Ice.ConnectionWarnings --Ice.ServerIdleTime=30"

serverOptions = commonServerOptions + serverProtocol
clientOptions = clientProtocol + defaultHost
clientServerOptions = commonServerOptions + clientServerProtocol + defaultHost
collocatedOptions = clientServerProtocol

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

def clientServerTestWithOptions(toplevel, name, additionalServerOptions, additionalClientOptions):

    testdir = os.path.join(toplevel, "test", name)
    classpath = os.path.join(toplevel, "lib") + sep + os.path.join(testdir, "classes") + sep + \
	os.getenv("CLASSPATH", "")
    server = "java -ea -classpath \"" + classpath + "\" Server "
    client = "java -ea -classpath \"" + classpath + "\" Client "

    updatedServerOptions = serverOptions.replace("TOPLEVELDIR", toplevel)
    updatedClientOptions = clientOptions.replace("TOPLEVELDIR", toplevel)

    print "starting server...",
    serverPipe = os.popen(server + updatedServerOptions + additionalServerOptions)
    getAdapterReady(serverPipe)
    print "ok"
    
    print "starting client...",
    clientPipe = os.popen(client + updatedClientOptions + additionalClientOptions)
    print "ok"

    for output in clientPipe.xreadlines():
	print output,

    clientStatus = clientPipe.close()
    serverStatus = serverPipe.close()

    if clientStatus or serverStatus:
	killServers()
	sys.exit(1)

def clientServerTest(toplevel, name):

    clientServerTestWithOptions(toplevel, name, "", "")

def mixedClientServerTestWithOptions(toplevel, name, additionalServerOptions, additionalClientOptions):

    testdir = os.path.join(toplevel, "test", name)
    classpath = os.path.join(toplevel, "lib") + sep + os.path.join(testdir, "classes") + sep + \
	 os.getenv("CLASSPATH", "")
    server = "java -ea -classpath \"" + classpath + "\" Server "
    client = "java -ea -classpath \"" + classpath + "\" Client "

    updatedServerOptions = clientServerOptions.replace("TOPLEVELDIR", toplevel)
    updatedClientOptions = updatedServerOptions

    print "starting server...",
    serverPipe = os.popen(server + updatedServerOptions + additionalServerOptions)
    getAdapterReady(serverPipe)
    print "ok"
    
    print "starting client...",
    clientPipe = os.popen(client + updatedClientOptions + additionalClientOptions)
    print "ok"

    for output in clientPipe.xreadlines():
	print output,

    clientStatus = clientPipe.close()
    serverStatus = serverPipe.close()

    if clientStatus or serverStatus:
	killServers()
	sys.exit(1)

def mixedClientServerTest(toplevel, name):

    mixedClientServerTestWithOptions(toplevel, name, "", "")

def collocatedTestWithOptions(toplevel, name, additionalOptions):

    testdir = os.path.join(toplevel, "test", name)
    classpath = os.path.join(toplevel, "lib") + sep + os.path.join(testdir, "classes") + sep + \
	 os.getenv("CLASSPATH", "")
    collocated = "java -ea -classpath \"" + classpath + "\" Collocated "

    updatedCollocatedOptions = collocatedOptions.replace("TOPLEVELDIR", toplevel)

    print "starting collocated...",
    collocatedPipe = os.popen(collocated + updatedCollocatedOptions + additionalOptions)
    print "ok"

    for output in collocatedPipe.xreadlines():
	print output,

    collocatedStatus = collocatedPipe.close()

    if collocatedStatus:
	killServers()
	sys.exit(1)

def collocatedTest(toplevel, name):

    collocatedTestWithOptions(toplevel, name, "")

def cleanDbDir(path):

    files = os.listdir(path)

    for filename in files:
        if filename != "CVS" and filename != ".dummy":
            fullpath = os.path.join(path, filename);
            os.remove(fullpath)
