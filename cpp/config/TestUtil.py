#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003 - 2004
# ZeroC, Inc.
# North Palm Beach, FL, USA
#
# All Rights Reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************


#
# Set protocol to "ssl" in case you want to run the tests with the SSL
# protocol. Otherwise TCP is used.
#

#protocol = ""
protocol = "ssl"

#
# Set compressed to 1 in case you want to run the tests with
# protocol compression.
#

#compress = 0
compress = 1

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

def getIceVersion():

    config = open(os.path.join(toplevel, "include", "IceUtil", "Config.h"), "r")
    return re.search("ICE_STRING_VERSION \"([0-9\.]*)\"", config.read()).group(1)

def getIceSoVersion():

    config = open(os.path.join(toplevel, "include", "IceUtil", "Config.h"), "r")
    intVersion = int(re.search("ICE_INT_VERSION ([0-9]*)", config.read()).group(1))
    majorVersion = intVersion / 10000
    minorVersion = intVersion / 100 - 100 * majorVersion    
    return '%d' % (majorVersion * 10 + minorVersion)

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

def isWin9x():

    if isWin32():
        if os.environ.has_key("OS") and os.environ["OS"] == "Windows_NT":
           return 0
        return 1
    else:
        return 0

def isSolaris():

    if sys.platform == "sunos5":
        return 1
    else:
        return 0
       
def isHpUx():

   if sys.platform == "hp-ux11":
        return 1
   else:
        return 0
     
def isDarwin():

   if sys.platform == "darwin":
        return 1
   else:
        return 0
     
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

        line = pipe.readline()

        if not line:
            break

        os.write(1, line)

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
elif isHpUx():
    os.environ["SHLIB_PATH"] = os.path.join(toplevel, "lib") + ":" + os.getenv("SHLIB_PATH", "")
elif isDarwin():
    os.environ["DYLD_LIBRARY_PATH"] = os.path.join(toplevel, "lib") + ":" + os.getenv("DYLD_LIBRRARY_PATH", "")
else:
    os.environ["LD_LIBRARY_PATH"] = os.path.join(toplevel, "lib") + ":" + os.getenv("LD_LIBRARY_PATH", "")
    os.environ["LD_LIBRARY_PATH_64"] = os.path.join(toplevel, "lib") + ":" + os.getenv("LD_LIBRARY_PATH_64", "")

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

    testdir = os.path.join(toplevel, "test", name)
    server = os.path.join(testdir, serverName)
    client = os.path.join(testdir, clientName)

    print "starting " + serverName + "...",
    serverPipe = os.popen(server + serverOptions + additionalServerOptions + " 2>&1")
    getServerPid(serverPipe)
    getAdapterReady(serverPipe)
    print "ok"
    
    print "starting " + clientName + "...",
    clientPipe = os.popen(client + clientOptions + additionalClientOptions + " 2>&1")
    print "ok"

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

    testdir = os.path.join(toplevel, "test", name)
    server = os.path.join(testdir, "server")
    client = os.path.join(testdir, "client")

    print "starting server...",
    serverPipe = os.popen(server + clientServerOptions + additionalServerOptions + " 2>&1")
    getServerPid(serverPipe)
    getAdapterReady(serverPipe)
    print "ok"
    
    print "starting client...",
    clientPipe = os.popen(client + clientServerOptions + additionalClientOptions + " 2>&1")
    getServerPid(clientPipe)
    getAdapterReady(clientPipe)
    print "ok"

    printOutputFromPipe(clientPipe)

    clientStatus = clientPipe.close()
    serverStatus = serverPipe.close()

    if clientStatus or serverStatus:
	killServers()
	sys.exit(1)

def mixedClientServerTest(name):

    mixedClientServerTestWithOptions(name, "", "")

def collocatedTestWithOptions(name, additionalOptions):

    testdir = os.path.join(toplevel, "test", name)
    collocated = os.path.join(testdir, "collocated")

    print "starting collocated...",
    collocatedPipe = os.popen(collocated + collocatedOptions + additionalOptions + " 2>&1")
    print "ok"

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
