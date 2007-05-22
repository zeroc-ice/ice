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
        protocol = a
    if o == "--compress":
        compress = 1
    if o == "--threadPerConnection":
        threadPerConnection = 1
    if o == "--host":
        host = a

#
# Check for ICE_HOME
#
ice_home = os.getenv("ICE_HOME", "")
if len(ice_home) == 0:
    print "ICE_HOME is not defined"
    sys.exit(1)

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

def isDarwin():

   if sys.platform == "darwin":
        return 1
   else:
        return 0

def isSolaris():

    if sys.platform == "sunos5":
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

        #print "started: " + str(self) + ": " + str(thread.get_ident())
        try:
            while 1:
                line = self.pipe.readline()
                if not line: break
                # Suppress "adapter ready" messages. Under windows the eol isn't \n.
                if not line.endswith(" ready\n") and not line.endswith(" ready\r\n"):
                    print "server: " + line,
        except IOError:
            pass

        self.status = closePipe(self.pipe)
        #print "terminating: " + str(self)

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
    os.environ["PATH"] = os.path.join(ice_home, "bin") + ";" + os.getenv("PATH", "")
else:
    os.environ["LD_LIBRARY_PATH"] = os.path.join(ice_home, "lib") + ":" + os.getenv("LD_LIBRARY_PATH", "")
    os.environ["LD_LIBRARY_PATH_64"] = os.path.join(ice_home, "lib") + ":" + os.getenv("LD_LIBRARY_PATH_64", "")

if protocol == "ssl":
    plugin               = " --Ice.Plugin.IceSSL=IceSSL:createIceSSL"
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

php = "php"

#
# If there is no php in the PATH, try php5:
#
if not isWin32() and not isDarwin():
    p = os.popen('php -v 2>/dev/null')
    lines = p.readlines() # This is necessary for close() to operate reliably.
    if p.close() != None:
        php = "php5"

if isWin32():
    extensionDir = os.path.abspath(os.path.join(toplevel, "bin"))
    extensionFile = "php_ice.dll"
    #
    # TODO: When we no longer support PHP 5.1.x, we can do the following:
    #
    #php = "php -d extension_dir=\"" + os.path.abspath(os.path.join(toplevel, "bin")) + "\" -d extension=php_ice.dll"
else:
    extensionDir = os.path.abspath(os.path.join(toplevel, "lib"))
    extensionFile = "IcePHP.so"
    #
    # TODO: When we no longer support PHP 5.1.x, we can do the following:
    #
    #php = "php -d extension_dir=\"" + os.path.abspath(os.path.join(toplevel, "lib")) + "\" -d extension=IcePHP.so"

def clientServerTestWithOptionsAndNames(name, additionalServerOptions, additionalClientOptions, \
                                        serverName, clientName):

    #
    # Write temporary INI file that contains the extension directives.
    # This is a workaround for a limitation in PHP 5.1.x that prevents
    # us from specifying the directives on PHP's command line.
    #
    # TODO: When we no longer support PHP 5.1.x, we can remove the extension
    #       directives. We still need to generate the temporary file to
    #       support ICE_HOME replacement.
    #
    tmpIniFile = "tmp.ini"
    testdir = os.path.join(toplevel, "test", name)
    server = os.path.join(ice_home, "test", name, serverName)
    client = php + " -c " + tmpIniFile + " -f " + clientName
    #client = php + " -c . -f " + clientName

    cwd = os.getcwd()
    os.chdir(testdir)

    #
    # Replace all occurrences of ICE_HOME with the value of the environment variable.
    #
    iniLines = open("php.ini", "r").readlines()
    for i in range(0,len(iniLines)):
        iniLines[i] = iniLines[i].replace("ICE_HOME", ice_home)

    iniFile = open(tmpIniFile, "w")
    iniFile.writelines(iniLines)
    iniFile.write("extension_dir=" + extensionDir + "\n")
    iniFile.write("extension=" + extensionFile + "\n")
    iniFile.close()

    try:
        print "starting " + serverName + "...",
        serverCmd = server + serverOptions + additionalServerOptions
        if debug:
            print "(" + serverCmd + ")",
        serverPipe = os.popen(serverCmd + " 2>&1")
        getServerPid(serverPipe)
        getAdapterReady(serverPipe)
        print "ok"
        
        print "starting " + clientName + "...",
        clientCmd = client + " -- " + clientOptions + additionalClientOptions
        if debug:
            print "(" + clientCmd + ")",
        clientPipe = os.popen(clientCmd + " 2>&1")
        print "ok"

        printOutputFromPipe(clientPipe)

        clientStatus = closePipe(clientPipe)
        if clientStatus:
            killServers()
    finally:
        os.remove(tmpIniFile)

    if clientStatus or serverStatus():
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
