#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os, re, errno, getopt
from threading import Thread

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


#
# Set protocol to "ssl" in case you want to run the tests with the SSL
# protocol. Otherwise TCP is used.
#
protocol = ""
#protocol = "ssl"

#
# Set compressed to 1 in case you want to run the tests with
# protocol compression.
#

compress = 0
#compress = 1

#
# Set threadPerConnection to 1 in case you want to run the tests in
# thread per connection mode.
#

#threadPerConnection = 0
threadPerConnection = 1

try:
    opts, args = getopt.getopt(sys.argv[1:], "m", ["mono", "protocol=", "compress", "host=", "threadPerConnection"])
except getopt.GetoptError:
    usage()

mono = 0

for o, a in opts:
    if o in ( "-m", "--mono" ):
	mono = 1
    if o == "--protocol":
	print "setting protocol: " + a
	protocol = a
    if o == "--compress":
	compress = 1
    if o == "--threadPerConnection":
	threadPerConnection = 1
    if o == "--host":
	host = a

if protocol == "ssl":
    threadPerConnection = 1

if not isWin32():
    mono = 1

if protocol == "ssl":
    threadPerConnection = 1

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

def closePipe(pipe):

    try:
	status = pipe.close()
    except IOError, ex:
	# TODO: There's a waitpid problem on CentOS, so we have to ignore ECHILD.
	if ex.errno == errno.ECHILD:
	    status = 0
	# This happens with the C# fault tolerance test. Ignore this error.
	elif ex.errno == 0:
	    status = 0
	else:
	    raise

    return status
     
class ReaderThread(Thread):
    def __init__(self, pipe):
        self.pipe = pipe
        Thread.__init__(self)

    def run(self):

        try:
            while 1:
                line = self.pipe.readline()
                if not line: break
		# Suppress "adapter ready" messages. Under windows the eol isn't \n.
		if not line.endswith(" ready\n") and not line.endswith(" ready\r\n"):
		    print line,
        except IOError:
	    print "IOError"
            pass

	self.status = closePipe(self.pipe)

    def getStatus(self):
	return self.status

serverPids = []
serverThreads = []
allServerThreads = []

def joinServers():
    global serverThreads
    global allServerThreads
    for t in serverThreads:
	t.join()
	allServerThreads.append(t)
    serverThreads = []

def serverStatus():
    global allServerThreads
    joinServers()
    for t in allServerThreads:
    	status = t.getStatus()
    	if status:
    	    return status
    return 0

def killServers():

    global serverPids
    global serverThreads

    for pid in serverPids:

        if isWin32():
            try:
                import win32api
                handle = win32api.OpenProcess(1, 0, pid)
                win32api.TerminateProcess(handle, 0)
            except ImportError, ex:
                print "Sorry: you must install the win32all package for killServers to work."
                return
            except:
                pass # Ignore errors, such as non-existing processes.
        else:
            try:
                os.kill(pid, 9)
            except:
                pass # Ignore errors, such as non-existing processes.

    serverPids = []

    #
    # Now join with all the threads
    #
    joinServers()

def getServerPid(pipe):
    global serverPids
    global serverThreads

    output = pipe.readline().strip()

    if not output:
        print "failed!"
        killServers()
        sys.exit(1)

    try:
	serverPids.append(int(output))
    except ValueError:
	print "Output is not a PID: " + output
	raise

def ignorePid(pipe):

    output = pipe.readline().strip()

    if not output:
        print "failed!"
        killServers()
        sys.exit(1)

def getAdapterReady(pipe, createThread = True):
    global serverThreads

    output = pipe.readline().strip()

    if not output:
        print "failed!"
        killServers()
        sys.exit(1)

    # Start a thread for this server.
    if createThread:
	serverThread = ReaderThread(pipe)
	serverThread.start()
	serverThreads.append(serverThread)

def waitServiceReady(pipe, token, createThread = True):
    global serverThreads

    while 1:
        output = pipe.readline().strip()
        if not output:
            print "failed!"
            sys.exit(1)
        if output == token + " ready":
            break

    # Start a thread for this server.
    if createThread:
	serverThread = ReaderThread(pipe)
	serverThread.start()
	serverThreads.append(serverThread)

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
    plugin		 = " --Ice.Plugin.IceSSL=" + os.path.join(toplevel, "bin", "icesslcs.dll") + \
			   ":IceSSL.PluginFactory"
    clientProtocol       = plugin + " --Ice.Default.Protocol=ssl" + \
                           " --IceSSL.CertFile=" + os.path.join(toplevel, "certs", "c_rsa1024.pfx") + \
                           " --IceSSL.Password=password" + \
                           " --IceSSL.CheckCertName=0"
    serverProtocol       = plugin + " --Ice.Default.Protocol=ssl" + \
                           " --IceSSL.ImportCert.LocalMachine.AuthRoot=" + \
				os.path.join(toplevel, "certs", "cacert.pem") + \
                           " --IceSSL.CertFile=" + os.path.join(toplevel, "certs", "s_rsa1024.pfx") + \
                           " --IceSSL.Password=password"
    clientServerProtocol = plugin + " --Ice.Default.Protocol=ssl" + \
                           " --IceSSL.ImportCert.LocalMachine.AuthRoot=" + \
				os.path.join(toplevel, "certs", "cacert.pem") + \
                           " --IceSSL.CertFile=" + os.path.join(toplevel, "certs", "c_rsa1024.pfx") + \
                           " --IceSSL.Password=password" + \
                           " --IceSSL.CheckCertName=0"
    cppPlugin		    = " --Ice.Plugin.IceSSL=IceSSL:create"
    cppClientProtocol       = cppPlugin + " --Ice.Default.Protocol=ssl" + \
                              " --IceSSL.DefaultDir=" + os.path.join(toplevel, "certs") + \
                              " --IceSSL.CertAuthFile=cacert.pem" + \
                              " --IceSSL.CertFile=c_rsa1024_pub.pem" + \
                              " --IceSSL.KeyFile=c_rsa1024_priv.pem"
    cppServerProtocol       = cppPlugin + " --Ice.Default.Protocol=ssl" + \
                              " --IceSSL.DefaultDir=" + os.path.join(toplevel, "certs") + \
                              " --IceSSL.CertAuthFile=cacert.pem" + \
                              " --IceSSL.CertFile=s_rsa1024_pub.pem" + \
                              " --IceSSL.KeyFile=s_rsa1024_priv.pem"
    cppClientServerProtocol = cppClientProtocol
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

def createMsg(name):
    msg = "starting "
    if mono:
	msg += "mono "
    msg += name
    if mono:
        msg += ".exe"
    msg += "..."

    return msg

def createCmd(bin):

    if mono:
        return "mono --debug " + bin + ".exe"
    else:
        return bin

def clientServerTestWithOptionsAndNames(name, additionalServerOptions, additionalClientOptions, \
                                        serverName, clientName):

    testdir = os.path.join(toplevel, "test", name)
    server = os.path.join(testdir, serverName)
    client = os.path.join(testdir, clientName)

    print createMsg(serverName),
    serverCmd = createCmd(server) + serverOptions + " " + additionalServerOptions + " 2>&1"
    #print "serverCmd=" + serverCmd
    serverPipe = os.popen(serverCmd)
    getServerPid(serverPipe)
    getAdapterReady(serverPipe)
    print "ok"
    
    print createMsg(clientName),
    clientCmd = createCmd(client) + clientOptions + " " + additionalClientOptions + " 2>&1"
    #print "clientCmd=" + clientCmd
    clientPipe = os.popen(clientCmd)
    print "ok"

    printOutputFromPipe(clientPipe)

    clientStatus = closePipe(clientPipe)
    if clientStatus:
	killServers()

    if clientStatus or serverStatus():
        sys.exit(1)

def clientServerTestWithOptions(name, additionalServerOptions, additionalClientOptions):

    clientServerTestWithOptionsAndNames(name, additionalServerOptions, additionalClientOptions, "server", \
                                        "client")

def clientServerTest(name):

    clientServerTestWithOptions(name, "", "")

def mixedClientServerTestWithOptions(name, additionalServerOptions, additionalClientOptions):

    testdir = os.path.join(toplevel, "test", name)
    server = os.path.join(testdir, "server")
    client = os.path.join(testdir, "client")

    print createMsg("server"),
    serverCmd = createCmd(server) + clientServerOptions + " " + additionalServerOptions + " 2>&1"
    #print "serverCmd = " + serverCmd
    serverPipe = os.popen(serverCmd)
    getServerPid(serverPipe)
    getAdapterReady(serverPipe)
    print "ok"
    
    print createMsg("client"),
    clientCmd = createCmd(client) + clientServerOptions + " " + additionalClientOptions + " 2>&1"
    #print "clientCmd = " + clientCmd
    clientPipe = os.popen(clientCmd)
    ignorePid(clientPipe)
    getAdapterReady(clientPipe, False)
    print "ok"

    printOutputFromPipe(clientPipe)

    clientStatus = closePipe(clientPipe)
    if clientStatus:
	killServers()

    if clientStatus or serverStatus():
        sys.exit(1)

def mixedClientServerTest(name):

    mixedClientServerTestWithOptions(name, "", "")

def collocatedTestWithOptions(name, additionalOptions):

    testdir = os.path.join(toplevel, "test", name)
    collocated = os.path.join(testdir, "collocated")

    print createMsg("collocated"),
    collocatedPipe = os.popen(createCmd(collocated) + collocatedOptions + " " + additionalOptions + " 2>&1")
    print "ok"

    printOutputFromPipe(collocatedPipe)

    collocatedStatus = closePipe(collocatedPipe)

    if collocatedStatus:
	killServers()
	sys.exit(1)

def collocatedTest(name):

    collocatedTestWithOptions(name, "")

def clientTestWithOptions(name, additionalOptions):

    testdir = os.path.join(toplevel, "test", name)
    client = os.path.join(testdir, "client")

    print createMsg("client"),
    clientPipe = os.popen(createCmd(client) + clientOptions + " " + additionalOptions + " 2>&1")
    print "ok"

    printOutputFromPipe(clientPipe)

    clientStatus = closePipe(clientPipe)

    if clientStatus:
	killServers()
	sys.exit(1)

def clientTest(name):

    clientTestWithOptions(name, "")

def cleanDbDir(path):

    files = os.listdir(path)

    for filename in files:
        if filename != "CVS" and filename != ".dummy":
            fullpath = os.path.join(path, filename);
            os.remove(fullpath)
