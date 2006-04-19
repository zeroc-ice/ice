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

import sys, os, re, errno

#
# Check for ICE_HOME
#
ice_home = os.environ["ICE_HOME"]
if len(ice_home) == 0:
    print "ICE_HOME is not defined"
    sys.exit(1)

def getIceVersion():

    config = open(os.path.join(ice_home, "include", "IceUtil", "Config.h"), "r")
    return re.search("ICE_STRING_VERSION \"([0-9\.]*)\"", config.read()).group(1)

def getIceSoVersion():
    config = open(os.path.join(ice_home, "include", "IceUtil", "Config.h"), "r")
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

def isSolaris():

    if sys.platform == "sunos5":
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

    while 1:
        output = pipe.readline().strip()
        if not output:
            print "failed!"
            sys.exit(1)
        if output == token + " ready":
            break

def printOutputFromPipe(pipe):

    while 1:
        line = pipe.readline()
        if not line:
            break
        os.write(1, line)

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
    os.environ["PATH"] = os.path.join(ice_home, "bin") + ";" + os.getenv("PATH", "")
else:
    os.environ["LD_LIBRARY_PATH"] = os.path.join(ice_home, "lib") + ":" + os.getenv("LD_LIBRARY_PATH", "")
    os.environ["LD_LIBRARY_PATH_64"] = os.path.join(ice_home, "lib") + ":" + os.getenv("LD_LIBRARY_PATH_64", "")

if protocol == "ssl":
    plugin		 = " --Ice.Plugin.IceSSL=IceSSL:create"
    clientProtocol       = plugin + " --Ice.Default.Protocol=ssl" + \
                           " --IceSSL.DefaultDir=" + os.path.join(ice_home, "certs") + \
                           " --IceSSL.CertFile=c_rsa1024_pub.pem" + \
                           " --IceSSL.KeyFile=c_rsa1024_priv.pem" + \
                           " --IceSSL.CertAuthFile=cacert.pem"
    serverProtocol       = plugin + " --Ice.Default.Protocol=ssl" + \
                           " --IceSSL.DefaultDir=" + os.path.join(ice_home, "certs") + \
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

if host != "":
    defaultHost = " --Ice.Default.Host=" + host
else:
    defaultHost = ""

commonServerOptions = " --Ice.PrintProcessId --Ice.PrintAdapterReady --Ice.NullHandleAbort" + \
                      " --Ice.Warn.Connections --Ice.ServerIdleTime=30" + \
                      " --Ice.ThreadPool.Server.Size=1 --Ice.ThreadPool.Server.SizeMax=3" + \
                      " --Ice.ThreadPool.Server.SizeWarn=0"

clientOptions = clientProtocol + defaultHost
serverOptions = serverProtocol + defaultHost + commonServerOptions
clientServerOptions = clientServerProtocol + defaultHost + commonServerOptions

def clientServerTestWithOptionsAndNames(name, additionalServerOptions, additionalClientOptions, \
                                        serverName, clientName):

    testdir = os.path.join(toplevel, "test", name)
    server = os.path.join(ice_home, "test", name, serverName)
    client = "php -c . -f " + clientName

    cwd = os.getcwd()
    os.chdir(testdir)

    print "starting " + serverName + "...",
    serverPipe = os.popen(server + serverOptions + additionalServerOptions + " 2>&1")
    getServerPid(serverPipe)
    getAdapterReady(serverPipe)
    print "ok"
    
    print "starting " + clientName + "...",
    clientPipe = os.popen(client + " -- " + clientOptions + additionalClientOptions + " 2>&1")
    print "ok"

    printOutputFromPipe(clientPipe)

    clientStatus = closePipe(clientPipe)
    serverStatus = closePipe(serverPipe)

    if clientStatus or serverStatus:
	killServers()
	sys.exit(1)

    os.chdir(cwd)

def clientServerTestWithOptions(name, additionalServerOptions, additionalClientOptions):

    clientServerTestWithOptionsAndNames(name, additionalServerOptions, additionalClientOptions, "server", "Client.php")

def clientServerTest(name):

    clientServerTestWithOptions(name, "", "")

def cleanDbDir(path):

    files = os.listdir(path)

    for filename in files:
        if filename != "CVS" and filename != ".dummy":
            fullpath = os.path.join(path, filename);
            os.remove(fullpath)
