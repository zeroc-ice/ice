#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Set blocking to 1 in case you want to run the tests in
# blocking mode.
#

blocking = 0
#blocking = 1

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

import sys, os, getopt

def usage():
    print "usage: " + sys.argv[0] + " --host host --blocking"
    sys.exit(2)
try:
    opts, args = getopt.getopt(sys.argv[1:], "", ["host=", "blocking"])
except getopt.GetoptError:
    usage()

for o, a in opts:
    if o == "--blocking":
        blocking = 1
    if o == "--host":
        host = a

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
        
# Only used for C++ programs
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
            
# Only used for C++ programs
def getServerPid(serverPipe):

    output = serverPipe.readline().strip()

    if not output:
        print "failed!"
        killServers()
        sys.exit(1)

    serverPids.append(int(output))

def getAdapterReady(serverPipe):

    output = serverPipe.readline()

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

sep = ""
if isWin32():
    sep = ";"
else:
    sep = ":"

os.environ["CLASSPATH"] = os.path.join(toplevel, "jdk", "lib", "IceE.jar") + sep + os.getenv("CLASSPATH", "")

clientProtocol = ""
serverProtocol = ""
clientServerProtocol = ""
cppClientProtocol = ""
cppServerProtocol = ""
cppClientServerProtocol = ""

if blocking:
    clientProtocol += " --Ice.Blocking"
    serverProtocol += " --Ice.Blocking"
    clientServerProtocol += " --Ice.Blocking"
    cppClientProtocol += " --Ice.Blocking"
    cppServerProtocol += " --Ice.Blocking"
    cppClientServerProtocol += " --Ice.Blocking"

if host != "":
    defaultHost = " --Ice.Default.Host=" + host
else:
    defaultHost = ""

commonClientOptions = " --Ice.Warn.Connections"

commonServerOptions = " --Ice.PrintAdapterReady" + \
                      " --Ice.Warn.Connections --Ice.ServerIdleTime=30"

clientOptions = clientProtocol + defaultHost + commonClientOptions
serverOptions = serverProtocol + defaultHost + commonServerOptions
clientServerOptions = clientServerProtocol + defaultHost + commonServerOptions
collocatedOptions = clientServerProtocol + defaultHost

cppCommonClientOptions = " --Ice.Warn.Connections"

cppCommonServerOptions = " --Ice.PrintAdapterReady" + \
                         " --Ice.Warn.Connections --Ice.ServerIdleTime=30"

cppClientOptions = cppClientProtocol + defaultHost + cppCommonClientOptions
cppServerOptions = cppServerProtocol + defaultHost + cppCommonServerOptions
cppClientServerOptions = cppClientServerProtocol + defaultHost + cppCommonServerOptions

def clientServerTestWithOptions(additionalServerOptions, additionalClientOptions):

    server = "java Server --Ice.ProgramName=Server "
    client = "java Client --Ice.ProgramName=Client "

    print "starting server...",
    serverPipe = os.popen(server + serverOptions + additionalServerOptions  + " 2>&1")
    getAdapterReady(serverPipe)
    print "ok"
    
    print "starting client...",
    clientPipe = os.popen(client + clientOptions + additionalClientOptions + " 2>&1")
    print "ok"

    printOutputFromPipe(clientPipe)

    clientStatus = clientPipe.close()
    serverStatus = serverPipe.close()

    if clientStatus or serverStatus:
	killServers()
	sys.exit(1)

def clientServerTestWithClasspath(serverClasspath, clientClasspath):

    server = "java Server --Ice.ProgramName=Server"
    client = "java Client --Ice.ProgramName=Client"

    classpath = os.getenv("CLASSPATH", "")
    scp = serverClasspath + sep + classpath
    ccp = clientClasspath + sep + classpath

    print "starting server...",
    os.environ["CLASSPATH"] = scp
    serverPipe = os.popen(server + serverOptions + " 2>&1")
    os.environ["CLASSPATH"] = classpath
    getAdapterReady(serverPipe)
    print "ok"
    
    print "starting client...",
    os.environ["CLASSPATH"] = ccp
    clientPipe = os.popen(client + clientOptions + " 2>&1")
    os.environ["CLASSPATH"] = classpath
    print "ok"

    printOutputFromPipe(clientPipe)

    clientStatus = clientPipe.close()
    serverStatus = serverPipe.close()

    if clientStatus or serverStatus:
	killServers()
	sys.exit(1)
    

def clientServerTest():

    clientServerTestWithOptions("", "")

def mixedClientServerTestWithOptions(additionalServerOptions, additionalClientOptions):

    server = "java Server --Ice.ProgramName=Server "
    client = "java Client --Ice.ProgramName=Client "

    print "starting server...",
    serverPipe = os.popen(server + clientServerOptions + additionalServerOptions + " 2>&1")
    getAdapterReady(serverPipe)
    print "ok"
    
    print "starting client...",
    clientPipe = os.popen(client + clientServerOptions + additionalClientOptions + " 2>&1")
    print "ok"

    printOutputFromPipe(clientPipe)

    clientStatus = clientPipe.close()
    serverStatus = serverPipe.close()

    if clientStatus or serverStatus:
	killServers()
	sys.exit(1)

def mixedClientServerTest():

    mixedClientServerTestWithOptions("", "")

def collocatedTestWithOptions(additionalOptions):

    collocated = "java Collocated --Ice.ProgramName=Collocated "

    print "starting collocated...",
    collocatedPipe = os.popen(collocated + collocatedOptions + additionalOptions + " 2>&1")
    print "ok"

    printOutputFromPipe(collocatedPipe)

    collocatedStatus = collocatedPipe.close()

    if collocatedStatus:
	killServers()
	sys.exit(1)

def collocatedTest():

    collocatedTestWithOptions("")

def cleanDbDir(path):

    files = os.listdir(path)

    for filename in files:
        if filename != "CVS" and filename != ".dummy":
            fullpath = os.path.join(path, filename);
            os.remove(fullpath)
