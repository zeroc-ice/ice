#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Set protocol to "ssl" in case you want to run the tests with the SSL
# protocol. Otherwise TCP is used.
#

protocol = "ssl"
#protocol = "tcp"

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

import sys, os, re, errno

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

def isAIX():
   if sys.platform in ['aix4', 'aix5']:
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

    if isCygwin():
	print "killServers(): not implemented for cygwin python."
	return

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

def getIceBox(testdir):

    #
    # Get and return the path of the IceBox executable
    #

    iceBox = ""
    if isWin32():
	#
    	# Read the build.txt file from the test directory to figure out 
    	# how the IceBox service was built ("debug" vs. "release") and 
	# decide which icebox executable to use.
    	# 
    	build = open(os.path.join(testdir, "build.txt"), "r")
    	type = build.read().strip()
    	if type == "debug":
	    iceBox = os.path.join(toplevel, "bin", "iceboxd.exe")
        elif type == "release":
            iceBox = os.path.join(toplevel, "bin", "icebox.exe")
    else:
	iceBox = os.path.join(toplevel, "bin", "icebox")

    if iceBox == "" or not os.path.exists(iceBox):
	print "couldn't find icebox executable to run the test"
	sys.exit(0)

    return iceBox;

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

def closePipe(pipe):

    try:
	status = pipe.close()
    except IOError, ex:
	# TODO: There's a waitpid problem on CentOS, so we have to ignore ECHILD.
	if ex.errno == errno.ECHILD:
	    status = 0
	else:
	    raise

    return status

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
elif isAIX():
    os.environ["LIBPATH"] = os.path.join(toplevel, "lib") + ":" + os.getenv("LIBPATH", "")
else:
    os.environ["LD_LIBRARY_PATH"] = os.path.join(toplevel, "lib") + ":" + os.getenv("LD_LIBRARY_PATH", "")
    os.environ["LD_LIBRARY_PATH_64"] = os.path.join(toplevel, "lib") + ":" + os.getenv("LD_LIBRARY_PATH_64", "")

if protocol == "ssl":
    plugin		 = " --Ice.Plugin.IceSSL=IceSSL:create"
    clientProtocol       = plugin + " --Ice.Default.Protocol=ssl" + \
                           " --IceSSL.DefaultDir=" + os.path.join(toplevel, "certs") + \
                           " --IceSSL.CertFile=c_rsa1024_pub.pem" + \
                           " --IceSSL.KeyFile=c_rsa1024_priv.pem" + \
                           " --IceSSL.CertAuthFile=cacert.pem"
    serverProtocol       = plugin + " --Ice.Default.Protocol=ssl" + \
                           " --IceSSL.DefaultDir=" + os.path.join(toplevel, "certs") + \
                           " --IceSSL.CertFile=s_rsa1024_pub.pem" + \
                           " --IceSSL.KeyFile=s_rsa1024_priv.pem" + \
                           " --IceSSL.CertAuthFile=cacert.pem"
    clientServerProtocol = clientProtocol
else:
    clientProtocol = ""
    serverProtocol = ""
    clientServerProtocol = ""

if compress:
    clientProtocol += " --Ice.Override.Compress"
    serverProtocol += " --Ice.Override.Compress"
    clientServerProtocol += " --Ice.Override.Compress"

if threadPerConnection:
    clientProtocol += " --Ice.ThreadPerConnection"
    serverProtocol += " --Ice.ThreadPerConnection"
    clientServerProtocol += " --Ice.ThreadPerConnection"

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
    serverCmd = server + serverOptions + additionalServerOptions + " 2>&1"
    #print "serverCmd =", serverCmd
    serverPipe = os.popen(serverCmd)
    getServerPid(serverPipe)
    getAdapterReady(serverPipe)
    print "ok"
    
    print "starting " + clientName + "...",
    clientCmd = client + clientOptions + additionalClientOptions + " 2>&1"
    #print "clientCmd =", clientCmd
    clientPipe = os.popen(clientCmd)
    print "ok"

    printOutputFromPipe(clientPipe)

    clientStatus = closePipe(clientPipe)
    serverStatus = closePipe(serverPipe)

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

    clientStatus = closePipe(clientPipe)
    serverStatus = closePipe(serverPipe)

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

    collocatedStatus = closePipe(collocatedPipe)

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
