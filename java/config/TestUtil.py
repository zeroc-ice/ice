#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
#protocol = "ssl"

#
# Set compressed to 1 in case you want to run the tests with
# protocol compression.
#

compress = 0
#compress = 1

#
# Set threadPerConnection to 1 in case you want to run the tests in
# thread-per-connection mode.
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
# To print the commands that are being run.
#
debug = 0
#debug = 1

#
# Don't change anything below this line!
#

import sys, os, errno, getopt
from threading import Thread

#
# Don't change anything below this line!
#
def usage():
    print "usage: " + sys.argv[0] + " --debug --protocol protocol --compress --host host --threadPerConnection"
    sys.exit(2)
try:
    opts, args = getopt.getopt(sys.argv[1:], "", ["debug", "protocol=", "compress", "host=", "threadPerConnection"])
except getopt.GetoptError:
    usage()

for o, a in opts:
    if o == "--debug":
        debug = 1
    if o == "--protocol":
        if a not in ( "tcp", "ssl"):
            usage()
        protocol = a
    if o == "--compress":
        compress = 1
    if o == "--threadPerConnection":
        threadPerConnection = 1
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
            print "server " + str(t) + " status: " + str(status)
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

def getIceSslVersion():
    javaPipeIn, javaPipeOut = os.popen4("java IceSSL.Util")
    if not javaPipeIn or not javaPipeOut:
        print "unable to get IceSSL version!"
        sys.exit(1)
    version = javaPipeOut.readline()
    if not version:
        print "unable to get IceSSL version!"
        sys.exit(1)
    javaPipeIn.close()
    javaPipeOut.close()
    return version.strip()

def getJdkVersion():
    javaPipeIn, javaPipeOut = os.popen4("java -version")
    if not javaPipeIn or not javaPipeOut:
        print "unable to get Java version!"
        sys.exit(1)
    version = javaPipeOut.readline()
    if not version:
        print "unable to get Java version!"
        sys.exit(1)
    javaPipeIn.close()
    javaPipeOut.close()
    return version

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

os.environ["CLASSPATH"] = os.path.join(toplevel, "lib", "Ice.jar") + sep + os.getenv("CLASSPATH", "")
os.environ["CLASSPATH"] = os.path.join(toplevel, "lib") + sep + os.getenv("CLASSPATH", "")

#
# If we are using SSL as the default protocol, we need to take extra
# precautions when using JDK 1.4.
#
javaCmd = "java"
jdkVersion = None
iceSslVersion = None
if protocol == "ssl":
    jdkVersion = getJdkVersion()
    if jdkVersion.startswith("java version \"1.4"):
        #
        # To avoid the potential for long delays at startup under JDK 1.4,
        # we direct the JVM to use /dev/urandom instead of its default.
        #
        javaCmd = "java -Djava.security.egd=file:/dev/urandom"

    iceSslVersion = getIceSslVersion()
    if iceSslVersion == "1.4":
        #
        # IceSSL for JDK 1.4 requires thread-per-connection.
        #
        threadPerConnection = 1

if protocol == "ssl":
    plugin               = " --Ice.Plugin.IceSSL=IceSSL.PluginFactory"
    clientProtocol       = plugin + " --Ice.Default.Protocol=ssl" + \
                           " --IceSSL.DefaultDir=" + os.path.join(toplevel, "certs") + \
                           " --IceSSL.Keystore=client.jks" + \
                           " --IceSSL.Truststore=certs.jks" + \
                           " --IceSSL.Password=password"
    serverProtocol       = plugin + " --Ice.Default.Protocol=ssl" + \
                           " --IceSSL.DefaultDir=" + os.path.join(toplevel, "certs") + \
                           " --IceSSL.Keystore=server.jks" + \
                           " --IceSSL.Truststore=certs.jks" + \
                           " --IceSSL.Password=password"
    clientServerProtocol = clientProtocol
    cppPlugin               = " --Ice.Plugin.IceSSL=IceSSL:createIceSSL"
    cppClientProtocol       = cppPlugin + " --Ice.Default.Protocol=ssl" + \
                              " --IceSSL.DefaultDir=" + os.path.join(toplevel, "certs") + \
                              " --IceSSL.CertFile=c_rsa1024_pub.pem" + \
                              " --IceSSL.KeyFile=c_rsa1024_priv.pem" + \
                              " --IceSSL.CertAuthFile=cacert.pem"
    cppServerProtocol       = cppPlugin + " --Ice.Default.Protocol=ssl" + \
                              " --IceSSL.DefaultDir=" + os.path.join(toplevel, "certs") + \
                              " --IceSSL.CertFile=s_rsa1024_pub.pem" + \
                              " --IceSSL.KeyFile=s_rsa1024_priv.pem" + \
                              " --IceSSL.CertAuthFile=cacert.pem"
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
    cppClientProtocol += " --Ice.ThreadPerConnection"
    cppServerProtocol += " --Ice.ThreadPerConnection"
    cppClientServerProtocol += " --Ice.ThreadPerConnection"

if host != "":
    defaultHost = " --Ice.Default.Host=" + host
else:
    defaultHost = ""

commonClientOptions = " --Ice.Warn.Connections"

commonServerOptions = " --Ice.PrintAdapterReady --Ice.Warn.Connections"

if not threadPerConnection:
    commonServerOptions += " --Ice.ThreadPool.Server.Size=1 --Ice.ThreadPool.Server.SizeMax=3" + \
                           " --Ice.ThreadPool.Server.SizeWarn=0 --Ice.ServerIdleTime=30"

clientOptions = clientProtocol + defaultHost + commonClientOptions
serverOptions = serverProtocol + defaultHost + commonServerOptions
clientServerOptions = clientServerProtocol + defaultHost + commonServerOptions
collocatedOptions = clientServerProtocol + defaultHost

cppCommonClientOptions = " --Ice.NullHandleAbort --Ice.Warn.Connections"

cppCommonServerOptions = " --Ice.PrintProcessId --Ice.PrintAdapterReady --Ice.NullHandleAbort" + \
                         " --Ice.Warn.Connections --Ice.ServerIdleTime=30"

cppClientOptions = cppClientProtocol + defaultHost + cppCommonClientOptions
cppServerOptions = cppServerProtocol + defaultHost + cppCommonServerOptions
cppClientServerOptions = cppClientServerProtocol + defaultHost + cppCommonServerOptions

def clientServerTestWithOptions(additionalServerOptions, additionalClientOptions):

    server = javaCmd + " -ea Server --Ice.ProgramName=Server "
    client = javaCmd + " -ea Client --Ice.ProgramName=Client "

    print "starting server...",
    serverCmd = server + serverOptions + additionalServerOptions
    if debug:
        print "(" + serverCmd + ")",
    serverPipe = os.popen(serverCmd + " 2>&1")
    getAdapterReady(serverPipe)
    print "ok"
    
    print "starting client...",
    clientCmd = client + clientOptions + additionalClientOptions
    if debug:
        print "(" + clientCmd + ")",
    clientPipe = os.popen(clientCmd + " 2>&1")
    print "ok"

    printOutputFromPipe(clientPipe)

    clientStatus = closePipe(clientPipe)

    if clientStatus or serverStatus():
        killServers()
        sys.exit(1)

def clientServerTestWithClasspath(serverClasspath, clientClasspath):

    server = javaCmd + " -ea Server --Ice.ProgramName=Server"
    client = javaCmd + " -ea Client --Ice.ProgramName=Client"

    classpath = os.getenv("CLASSPATH", "")
    scp = serverClasspath + sep + classpath
    ccp = clientClasspath + sep + classpath

    print "starting server...",
    os.environ["CLASSPATH"] = scp
    serverCmd = server + serverOptions
    if debug:
        print "(" + serverCmd + ")",
    serverPipe = os.popen(serverCmd + " 2>&1")
    os.environ["CLASSPATH"] = classpath
    getAdapterReady(serverPipe)
    print "ok"
    
    print "starting client...",
    os.environ["CLASSPATH"] = ccp
    clientCmd = client + clientOptions
    if debug:
        print "(" + clientCmd + ")",
    clientPipe = os.popen(clientCmd + " 2>&1")
    os.environ["CLASSPATH"] = classpath
    print "ok"

    printOutputFromPipe(clientPipe)

    clientStatus = closePipe(clientPipe)

    if clientStatus or serverStatus():
        killServers()
        sys.exit(1)
    
def clientServerTest():

    clientServerTestWithOptions("", "")

def mixedClientServerTestWithOptions(additionalServerOptions, additionalClientOptions):

    server = javaCmd + " -ea Server --Ice.ProgramName=Server "
    client = javaCmd + " -ea Client --Ice.ProgramName=Client "

    print "starting server...",
    serverCmd = server + clientServerOptions + additionalServerOptions
    if debug:
        print "(" + serverCmd + ")",
    serverPipe = os.popen(serverCmd + " 2>&1")
    getAdapterReady(serverPipe)
    print "ok"
    
    print "starting client...",
    clientCmd = client + clientServerOptions + additionalClientOptions
    if debug:
        print "(" + clientCmd + ")",
    clientPipe = os.popen(clientCmd + " 2>&1")
    print "ok"

    printOutputFromPipe(clientPipe)

    clientStatus = closePipe(clientPipe)

    if clientStatus or serverStatus():
        killServers()
        sys.exit(1)

def mixedClientServerTest():

    mixedClientServerTestWithOptions("", "")

def collocatedTestWithOptions(additionalOptions):

    collocated = javaCmd + " -ea Collocated --Ice.ProgramName=Collocated "

    print "starting collocated...",
    command = collocated + collocatedOptions + additionalOptions
    if debug:
        print "(" + command + ")",
    collocatedPipe = os.popen(command + " 2>&1")
    print "ok"

    printOutputFromPipe(collocatedPipe)

    collocatedStatus = closePipe(collocatedPipe)

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
