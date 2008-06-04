#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# To print the commands that are being run.
#
debug = 0
#debug = 1

#
# Don't change anything below this line!
#
import sys, os, re, errno, getopt
from threading import Thread

def isCygwin():
    # The substring on sys.platform is required because some cygwin
    # versions return variations like "cygwin_nt-4.01".
    return sys.platform[:6] == "cygwin"

def isWin32():
    return sys.platform == "win32" or isCygwin()

def usage():
    print "usage: " + sys.argv[0] + " -m|--mono --debug --host host "
    sys.exit(2)

try:
    opts, args = getopt.getopt(sys.argv[1:], "m",\
         ["mono", "debug", "host="])
except getopt.GetoptError:
    usage()

mono = 0

for o, a in opts:
    if o in ( "-m", "--mono" ):
        mono = 1
    if o == "--debug":
        debug = 1
    if o == "--host":
        host = a

if not isWin32():
    mono = 1

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

class BridgeReaderThread(Thread):
    def __init__(self, pipe):
        self.pipe = pipe
        Thread.__init__(self)

    def run(self):

        try:
            while 1:
                line = self.pipe.readline()
                if not line: break
                if not line.startswith("Ice::Communicator::destroy"):
                    print "Bridge: %s" % line,
                    sys.stdout.flush()
                    pass
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

    stopBridge()
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

    while 1:
        output = pipe.readline().strip()
        if not output:
            print "failed!"
            killServers()
            sys.exit(1)
        if output.startswith("warning: "):
            continue
        break

    try:
        serverPids.append(int(output))
    except ValueError:
        print "Output is not a PID: " + output
        raise

def ignorePid(pipe):

    while 1:
        output = pipe.readline().strip()
        if not output:
            print "failed!"
            killServers()
            sys.exit(1)
        if output.startswith("warning: "):
            continue
        break

def getAdapterReady(pipe, createThread = True, count = 1):
    global serverThreads

    while count > 0: 
        output = pipe.readline().strip()
        count = count - 1

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

if not isWin32():
    os.environ["MONO_PATH"] = os.path.join(toplevel, "bin") + os.pathsep + os.getenv("MONO_PATH", "")
    os.environ["MONO_PATH"] = os.path.join(toplevel, "..", "cs", "bin") + os.pathsep + os.getenv("MONO_PATH", "")

if host != "":
    defaultHost = " --Ice.Default.Host=" + host
else:
    defaultHost = ""

commonClientOptions = " --Ice.NullHandleAbort --Ice.Warn.Connections"

commonServerOptions = " --Ice.PrintProcessId --Ice.PrintAdapterReady --Ice.NullHandleAbort" + \
                      " --Ice.Warn.Connections --Ice.ServerIdleTime=10" + \
                      " --Ice.ThreadPool.Server.Size=1 --Ice.ThreadPool.Server.SizeMax=3" + \
                      " --Ice.ThreadPool.Server.SizeWarn=0"

cppCommonClientOptions = " --Ice.NullHandleAbort --Ice.Warn.Connections"

cppCommonServerOptions = " --Ice.PrintProcessId --Ice.PrintAdapterReady --Ice.NullHandleAbort" + \
                         " --Ice.Warn.Connections --Ice.ServerIdleTime=30"

cppClientOptions = defaultHost + cppCommonClientOptions
cppServerOptions = defaultHost + cppCommonServerOptions
cppClientServerOptions = defaultHost + cppCommonServerOptions

clientOptions = defaultHost + commonClientOptions
serverOptions = defaultHost + commonServerOptions
clientServerOptions = defaultHost + commonServerOptions
collocatedOptions = defaultHost

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

bridgeThread = None
bridgeOut = None
bridgeIn = None

def startBridge(bwd = None):
    if bwd == None:
        bwd = os.path.join(toplevel, "src", "HttpBridge")

    global bridgeOut, bridgeIn, bridgeThread
    if mono:
	    print "starting bridge...",
	    sys.stdout.flush()
	    cmd = "xsp2 --root %s --applications /:." % bwd,
	    if debug:
		print "(%s)" % cmd
	    bridgeIn, bridgeOut = os.popen2("%s 2>&1" % cmd)
	    while 1:
		output = bridgeOut.readline().strip()
		if not output.startswith("Hit Return"):
		    continue
		break
	    # Start a thread for this server.
	    bridgeThread = BridgeReaderThread(bridgeOut)
	    bridgeThread.start()
	    print "ok"
    else:
	    import socket
	    try:
		import httplib
		c = httplib.HTTPConnection("127.0.0.1:8080")
		c.request("GET", "/index.html", None, {})
		return
	    except socket.error, e:
		pass

	    print "starting bridge...",
	    sys.stdout.flush()
	    bridgeIn, bridgeOut = os.popen2('WebDev.WebServer /port:8080 /path:"%s"' %(os.path.abspath(bwd)))
	    # Start a thread for this server.
	    bridgeThread = BridgeReaderThread(bridgeOut)
	    bridgeThread.start()
	    print "ok"
	

def stopBridge():
    global bridgeOut, bridgeIn, bridgeThread
    if mono:
	if bridgeThread:
	    print "stopping bridge...",
	    sys.stdout.flush()
	    bridgeIn.write("\n")
	    bridgeIn.flush()
	    bridgeThread.join()
	    print "ok"
    else:
	    print "stopping bridge...",
	    sys.stdout.flush()
	    import socket
	    try:
		import httplib
		c = httplib.HTTPConnection("127.0.0.1:8080")
		c.request("GET", "/Shutdown.ashx", None, {})
	    except socket.error, e:
		pass
	    bridgeThread.join()
	    print "ok"

def clientServerTestWithOptionsAndNames(name, additionalServerOptions, additionalClientOptions, \
                                        serverName, clientName):

    server = os.path.join(toplevel, "..", "cs", "test", "Ice", name, serverName)
    testdir = os.path.join(toplevel, "test", "IceCS", name)
    client = os.path.join(testdir, clientName)

    print createMsg(serverName),
    sys.stdout.flush()
    serverCmd = createCmd(server) + serverOptions + " " + additionalServerOptions
    if debug:
        print "(" + serverCmd + ")",
    serverPipe = os.popen(serverCmd + " 2>&1")
    getServerPid(serverPipe)
    getAdapterReady(serverPipe)
    print "ok"
    
    print createMsg(clientName),
    sys.stdout.flush()
    clientCmd = createCmd(client) + clientOptions + " " + additionalClientOptions
    if debug:
        print "(" + clientCmd + ")",
    clientPipe = os.popen(clientCmd + " 2>&1")
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

def clientTestWithOptions(name, additionalOptions):

    testdir = os.path.join(toplevel, "test", "IceCS", name)
    client = os.path.join(testdir, "client")

    print createMsg("client"),
    command = createCmd(client) + clientOptions + " " + additionalOptions
    if debug:
        print "(" + command + ")",
    clientPipe = os.popen(command + " 2>&1")
    print "ok"

    printOutputFromPipe(clientPipe)

    clientStatus = closePipe(clientPipe)

    if clientStatus:
        killServers()
        sys.exit(1)

def clientTest(name):

    clientTestWithOptions(name, "")

def cleanDbDir(path):
    for filename in [ os.path.join(path, f) for f in os.listdir(path) if f != ".gitignore"]:
	os.remove(filename)
