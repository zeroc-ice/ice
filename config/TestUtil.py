#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os, re, errno, getopt, time, StringIO, string
from threading import Thread

# Global flags and their default values.
protocol = ""                   # If unset, default to TCP. Valid values are "tcp" or "ssl".
compress = False                # Set to True to enable bzip2 compression.
serialize = False               # Set to True to have tests use connection serialization
host = "127.0.0.1"              # Default to loopback.
debug = False                   # Set to True to enable test suite debugging.
mono = False                    # Set to True when not on Windows
keepGoing = False               # Set to True to have the tests continue on failure.
ipv6 = False                    # Default to use IPv4 only
ice_home = None                 # Binary distribution to use (None to use binaries from source distribution)
x64 = False                     # Binary distribution is 64-bit
javaCmd = "java"                # Default java loader

#
# The PHP interpreter is called "php5" on some platforms (e.g., SLES).
#
phpCmd = "php"
for path in string.split(os.environ["PATH"], os.pathsep):
    #
    # Stop if we find "php" in the PATH first.
    #
    if os.path.exists(os.path.join(path, "php")):
        break
    elif os.path.exists(os.path.join(path, "php5")):
        phpCmd = "php5"
        break

#
# This is set by the choice of init method. If not set, before it is
# used, it indicates a bug and things should terminate.
#
defaultMapping = None

testErrors = []

def configurePaths():
    toplevel = findTopLevel()

    if ice_home:
        print "*** using Ice installation from " + ice_home,
        if x64:
            print "(64bit)",
        print

    #
    # If Ice is installed from RPMs, just set the CLASSPATH for Java.
    #
    if ice_home == "/usr":
        javaDir = os.path.join("/", "usr", "share", "java")
        os.environ["CLASSPATH"] = os.path.join(javaDir, "Ice.jar") + os.pathsep + os.getenv("CLASSPATH", "")
        return # That's it, we're done!
    
    if isWin32():
        os.environ["PATH"] = getCppBinDir() + os.pathsep + os.getenv("PATH", "")
    else:
        libDir = os.path.join(getIceDir("cpp"), "lib")
        if isHpUx():
            if x64:
                if ice_home:
                    libDir = os.path.join(libDir, "pa20_64")
                os.environ["LD_LIBRARY_PATH"] = libDir + os.pathsep + os.getenv("LD_LIBRARY_PATH", "")
            else:
                os.environ["SHLIB_PATH"] = libDir + os.pathsep + os.getenv("SHLIB_PATH", "")
        elif isDarwin():
            os.environ["DYLD_LIBRARY_PATH"] = libDir + os.pathsep + os.getenv("DYLD_LIBRARY_PATH", "")
        elif isAIX():
            os.environ["LIBPATH"] = libDir + os.pathsep + os.getenv("LIBPATH", "")
        elif isSolaris():
            if x64:
                if ice_home:
                    if isSparc():
                        libDir = os.path.join(libDir, "sparcv9")
                    else:
                        libDir = os.path.join(libDir, "amd64")
                os.environ["LD_LIBRARY_PATH_64"] = libDir + os.pathsep + os.getenv("LD_LIBRARY_PATH_64", "")
            else:
                os.environ["LD_LIBRARY_PATH"] = libDir + os.pathsep + os.getenv("LD_LIBRARY_PATH", "")
        else:
            if x64:
                if ice_home:
                    libDir = libDir + "64"
                os.environ["LD_LIBRARY_PATH_64"] = libDir + os.pathsep + os.getenv("LD_LIBRARY_PATH_64", "")
            else:
                os.environ["LD_LIBRARY_PATH"] = libDir + os.pathsep + os.getenv("LD_LIBRARY_PATH", "")

    javaDir = os.path.join(getIceDir("java"), "lib")
    os.environ["CLASSPATH"] = os.path.join(javaDir, "Ice.jar") + os.pathsep + os.getenv("CLASSPATH", "")
    os.environ["CLASSPATH"] = os.path.join(javaDir) + os.pathsep + os.getenv("CLASSPATH", "")
    
    # 
    # On Windows, C# assemblies are found thanks to the .exe.config files.
    #
    if not isWin32():
        os.environ["MONO_PATH"] = os.path.join(getIceDir("cs"), "bin") + os.pathsep + os.getenv("MONO_PATH", "")
        
    #
    # On Windows x64, set PYTHONPATH to python/x64.
    #
    pythonDir = os.path.join(getIceDir("py"), "python")
    if isWin32() and x64:
        os.environ["PYTHONPATH"] = os.path.join(pythonDir, "x64") + os.pathsep + os.getenv("PYTHONPATH", "")
    else:
        os.environ["PYTHONPATH"] = pythonDir + os.pathsep + os.getenv("PYTHONPATH", "")

    rubyDir = os.path.join(getIceDir("rb"), "ruby")
    os.environ["RUBYLIB"] = rubyDir + os.pathsep + os.getenv("RUBYLIB", "")

def addLdPath(libpath):
    if isWin32():
        os.environ["PATH"] = libpath + os.pathsep + os.getenv("PATH", "")
    elif isHpUx():
        os.environ["SHLIB_PATH"] = libpath + os.pathsep + os.getenv("SHLIB_PATH", "")
        os.environ["LD_LIBRARY_PATH"] = libpath + os.pathsep + os.getenv("LD_LIBRARY_PATH", "")
    elif isDarwin():
        os.environ["DYLD_LIBRARY_PATH"] = libpath + os.pathsep + os.getenv("DYLD_LIBRARY_PATH", "")
    elif isAIX():
        os.environ["LIBPATH"] = libpath + os.pathsep + os.getenv("LIBPATH", "")
    else:
        os.environ["LD_LIBRARY_PATH"] = libpath + os.pathsep + os.getenv("LD_LIBRARY_PATH", "")
        os.environ["LD_LIBRARY_PATH_64"] = libpath + os.pathsep + os.getenv("LD_LIBRARY_PATH_64", "")
    
def isCygwin():
    # The substring on sys.platform is required because some cygwin
    # versions return variations like "cygwin_nt-4.01".
    return sys.platform[:6] == "cygwin"

def isWin32():
    return sys.platform == "win32" or isCygwin()

def isVista():
    return isWin32() and sys.getwindowsversion()[0] == 6

def isWin9x():
    if isWin32():
        return not (os.environ.has_key("OS") and os.environ["OS"] == "Windows_NT")
    else:
        return 0

def isSolaris():
    return sys.platform == "sunos5"
       
def isSparc():
    p = os.popen("uname -m")
    l = p.readline().strip()
    p.close()
    if l == "sun4u":
        return True
    else:
        return False

def isHpUx():
   return sys.platform == "hp-ux11"

def isAIX():
   return sys.platform in ['aix4', 'aix5']
  
def isDarwin():
   return sys.platform == "darwin"

def isLinux():
    return sys.platform.startswith("linux")

def index(l, re):
    """Find the index of the first item in the list that matches the given re"""
    for i in range(0, len(l)):
        if re.search(l[i]):
            return i
    return -1

def run(tests, root = False):
    def usage():
        print "usage: " + sys.argv[0] + """
          --all                   Run all sensible permutations of the tests.
          --start=index           Start running the tests at the given index.
          --loop                  Run the tests in a loop.
          --filter=<regex>        Run all the tests that match the given regex.
          --rfilter=<regex>       Run all the tests that do not match the given regex.
          --debug                 Display debugging information on each test.
          --protocol=tcp|ssl      Run with the given protocol.
          --compress              Run the tests with protocol compression.
          --host=host             Set --Ice.Default.Host=<host>.
          --serialize             Run with connection serialization.
          --continue              Keep running when a test fails
          --ipv6                  Use IPv6 addresses.
          --ice-home=<path>       Use the binary distribution from the given path.
          --x64                   Binary distribution is 64-bit.
          --script                Generate a script to run the tests.
        """
        sys.exit(2)

    try:
        opts, args = getopt.getopt(sys.argv[1:], "lr:R:",
                                   ["start=", "start-after=", "filter=", "rfilter=", "all", "loop", "debug",
                                    "protocol=", "compress", "host=", "serialize", "continue", "ipv6",
                                    "ice-home=", "x64", "script"])
    except getopt.GetoptError:
        usage()

    if args:
        usage()

    start = 0
    loop = False
    all = False
    arg = ""
    script = False

    filters = []
    for o, a in opts:
        if o == "--continue":
            keepGoing = True
        elif o in ("-l", "--loop"):
            loop = True
        elif o in ("-r", "-R", "--filter", '--rfilter'):
            testFilter = re.compile(a)
            if o in ("--rfilter", "-R"):
                filters.append((testFilter, True))
            else:
                filters.append((testFilter, False))
        elif o == "--all" :
            all = True
        elif o in '--start':
            start = int(a)
        elif o == "--script":
            script = True
        elif o == "--protocol":
            if a not in ( "ssl", "tcp"):
                usage()
            if mono and getDefaultMapping() == "cs" and a == "ssl":
                print "SSL is not supported with mono"
                sys.exit(1)

        if o in ( "--protocol", "--host", "--debug", "--compress", "--serialize", "--ipv6", \
                  "--ice-home", "--x64"):
            arg += " " + o
            if len(a) > 0:
                arg += " " + a

    for testFilter, removeFilter in filters:
        if removeFilter:
            tests = [ (x, y) for x,y in tests if not testFilter.search(x) ]
        else:
            tests = [ (x, y) for x,y in tests if testFilter.search(x) ]

    if not root:
        tests = [ (os.path.join(getDefaultMapping(), "test", x), y) for x, y in tests ]

    # Expand all the tests and argument combinations.
    expanded = []
    if all:
        expanded.append([(test, arg, config) for test,config in tests if "once" in config ])

        a = '--protocol=tcp %s'  % arg
        expanded.append([ (test, a, config) for test,config in tests if "core" in config])

        a = '--protocol=ssl %s'  % arg
        expanded.append([ (test, a, config) for test,config in tests if "core" in config])

        a = '--protocol=tcp --compress %s'  % arg
        expanded.append([ (test, a, config) for test,config in tests if "core" in config])

        a = "--ipv6 --protocol=tcp %s" % arg
        expanded.append([ (test, a, config) for test,config in tests if "core" in config])

        a = "--ipv6 --protocol=ssl %s" % arg
        expanded.append([ (test, a, config) for test,config in tests if "core" in config])

        a = "--protocol=tcp %s" % arg
        expanded.append([ (test, a, config) for test,config in tests if "service" in config])

        a = "--protocol=ssl --ipv6 %s" % arg
        expanded.append([ (test, a, config) for test,config in tests if "service" in config])

        a = "--protocol=tcp --serialize %s" % arg
        expanded.append([ (test, a, config) for test,config in tests if "stress" in config])
    else:
        expanded.append([ (test, arg, config) for test,config in tests])

    if loop:
        num = 1
        while 1:
            runTests(start, expanded, num, script = script)
            num += 1
    else:
        runTests(start, expanded, script = script)

    global testErrors
    if len(testErrors) > 0:
        print "The following errors occurred:"
        for x in testErrors:
            print x

if not isWin32():
    mono = True
    
toplevel = None 

def findTopLevel():
    global toplevel

    if toplevel != None:
        return toplevel

    for toplevel in [".", "..", "../..", "../../..", "../../../..", "../../../../.."]:
        toplevel = os.path.abspath(toplevel)
        if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
            break
    else:
        toplevel = None
        raise "can't find toplevel directory!"

    return toplevel

def getIceDir(subdir = None):
    #
    # If ICE_HOME is set we're running the test against a binary distribution. Otherwise,
    # we're running the test against a source distribution.
    # 
    global ice_home
    if ice_home:
        return ice_home
    elif subdir:
        return os.path.join(findTopLevel(), subdir)
    else: 
        return findTopLevel()

findTopLevel()

serverPids = []
serverThreads = []
allServerThreads = []

def writePhpIni(src, dst):
    extDir = None
    ext = None

    #
    # TODO
    #
    # When we no longer support PHP 5.1.x, we can use the following PHP
    # command-line options:
    #
    # -d extension_dir=...
    # -d extension=[php_ice.dll|IcePHP.so]
    #
    if isWin32():
        ext = "php_ice.dll"
        extDir = os.path.abspath(os.path.join(getIceDir("php"), "bin"))
    else:
        ext = "IcePHP.so"
        if not ice_home:
            extDir = os.path.abspath(os.path.join(findTopLevel(), "php", "lib"))
        else:
            #
            # If ICE_HOME points to the installation directory of a source build, the
            # PHP extension will be located in $ICE_HOME/lib or $ICE_HOME/lib64.
            # For an RPM installation, PHP is already configured to load the extension.
            # We could also execute "php -m" and check if the output includes "ice".
            #
            if x64:
                extDir = os.path.join(ice_home, "lib64")
            else:
                extDir = os.path.join(ice_home, "lib")

            if not os.path.exists(os.path.join(extDir, ext)):
                if ice_home == "/usr":
                    extDir = None # Assume PHP is already configured to load the extension.
                else:
                    print "unable to find IcePHP extension!"
                    sys.exit(1)

    ini = open(src, "r").readlines()
    for i in range(0, len(ini)):
        ini[i] = ini[i].replace("ICE_HOME", os.path.join(findTopLevel()))
    tmpini = open(dst, "w")
    tmpini.writelines(ini)
    if extDir:
        tmpini.write("extension_dir=%s\n" % extDir)
        tmpini.write("extension=%s\n" % ext)
    tmpini.close()

def getIceSoVersion():

    config = open(os.path.join(findTopLevel(), "cpp", "include", "IceUtil", "Config.h"), "r")
    intVersion = int(re.search("ICE_INT_VERSION ([0-9]*)", config.read()).group(1))
    majorVersion = intVersion / 10000
    minorVersion = intVersion / 100 - 100 * majorVersion    
    patchVersion = intVersion % 100
    if patchVersion > 50:
        if patchVersion >= 52:
            return '%db%d' % (majorVersion * 10 + minorVersion, patchVersion - 50)
        else:
            return '%db' % (majorVersion * 10 + minorVersion)
    else:
        return '%d' % (majorVersion * 10 + minorVersion)

def getIceSSLVersion():
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

def closePipe(pipe):
    try:
        status = pipe.close()
    except IOError, ex:
        # TODO: There's a waitpid problem on CentOS, so we have to ignore ECHILD.
        if ex.errno == errno.ECHILD:
            status = 0
        # This happens with the test/IceGrid/simple test on AIX, and the
        # C# fault tolerance test.
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
                    sys.stdout.flush()
                    print line,
        except IOError:
            pass

        self.status = closePipe(self.pipe)

    def getPipe(self):
        return self.pipe

    def getStatus(self):
        return self.status

def joinServers():
    global serverThreads
    global allServerThreads
    for t in serverThreads:
        t.join()
        allServerThreads.append(t)
    serverThreads = []

# This joins with all servers and if any of them failed then
# it returns the failure status.
def serverStatus():
    global allServerThreads
    joinServers()
    for t in allServerThreads:
        status = t.getStatus()
        if status:
            print "server ", str(t), " status: ", str(status)
            return status
    return 0

# This joins with a specific server (the one started with the given pipe)
# returns its exit status. If the server cannot be found an exception
# is raised.
def specificServerStatus(pipe, timeout = None):
    global serverThreads
    for t in serverThreads:
        if t.getPipe() == pipe:
            serverThreads.remove(t)
	    if isWin32() and timeout != None:
 		#
	        # BUGFIX: On Windows x64 with python 2.5 join with
	        # a timeout doesn't work (it hangs for the duration
		# of the timeout if the thread is alive at the time
		# of the join call).
		#
	        while timeout >= 0 and t.isAlive():
		    time.sleep(1)
   	   	    timeout -= 1
		if not t.isAlive():
		    t.join()
            else:
	        t.join(timeout)
            if t.isAlive():
                raise "server with pipe " + str(pipe) + " did not exit within the timeout period."
            status = t.getStatus()
            return status
    raise "can't find server with pipe: " + str(pipe)

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

    output = ignorePid(pipe)

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
        return output

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

def getIceBox(testdir):

    #
    # Get and return the path of the IceBox executable
    #
    lang = getDefaultMapping()
    if lang == "cpp":
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
                iceBox = os.path.join(getCppBinDir(), "iceboxd.exe")
            elif type == "release":
                iceBox = os.path.join(getCppBinDir(), "icebox.exe")
        else:
            iceBox = os.path.join(getCppBinDir(), "icebox")

        if not os.path.exists(iceBox):
            print "couldn't find icebox executable to run the test"
            sys.exit(0)
    elif lang == "java":
        iceBox = "IceBox.Server"
    elif lang == "cs":
        iceBox = os.path.join(getIceDir("cs"), "bin", "iceboxnet")
                
    if iceBox == "":
        print "couldn't find icebox executable to run the test"
        sys.exit(0)
    
    return iceBox;

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

class InvalidSelectorString(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

sslConfigTree = { 
        "cpp" : { 
            "plugin" : " --Ice.Plugin.IceSSL=IceSSL:createIceSSL --Ice.Default.Protocol=ssl " +
            "--IceSSL.DefaultDir=%(certsdir)s --IceSSL.CertAuthFile=cacert.pem",
            "client" : " --IceSSL.CertFile=c_rsa1024_pub.pem --IceSSL.KeyFile=c_rsa1024_priv.pem",
            "server" : " --IceSSL.CertFile=s_rsa1024_pub.pem --IceSSL.KeyFile=s_rsa1024_priv.pem",
            "colloc" : " --IceSSL.CertFile=c_rsa1024_pub.pem --IceSSL.KeyFile=c_rsa1024_priv.pem"
            },
        "java" : { 
            "plugin" : " --Ice.Plugin.IceSSL=IceSSL.PluginFactory --Ice.Default.Protocol=ssl " +
            "--IceSSL.DefaultDir=%(certsdir)s --IceSSL.Truststore=certs.jks --IceSSL.Password=password",
            "client" : " --IceSSL.Keystore=client.jks",
            "server" : " --IceSSL.Keystore=server.jks",
            "colloc" : " --IceSSL.Keystore=client.jks"
            },
        "cs" : {
            "plugin" : " --Ice.Plugin.IceSSL=IceSSL:IceSSL.PluginFactory --Ice.Default.Protocol=ssl" +
            " --IceSSL.Password=password --IceSSL.DefaultDir=%(certsdir)s",
            "client" : " --IceSSL.CertFile=c_rsa1024.pfx --IceSSL.CheckCertName=0",
            "server" : " --IceSSL.CertFile=s_rsa1024.pfx --IceSSL.ImportCert.CurrentUser.Root=cacert.pem",
            "colloc" : " --IceSSL.CertFile=c_rsa1024.pfx --IceSSL.ImportCert.CurrentUser.Root=cacert.pem --IceSSL.CheckCertName=0"
            },
        }
sslConfigTree["py"] = sslConfigTree["cpp"]
sslConfigTree["rb"] = sslConfigTree["cpp"]
sslConfigTree["php"] = sslConfigTree["cpp"]

def getDefaultMapping(currentDir = ""):
    """Try and guess the language mapping out of the current path"""

    if currentDir != "":
        # Caller has specified the current path to use as a base. 
        scriptPath = os.path.abspath(currentDir).split(os.sep)
        scriptPath.reverse()
        for p in scriptPath: 
            if p in ["cpp", "cs", "java", "php", "py", "rb", "tmp"]:
                return p

    scriptPath = os.path.abspath(sys.argv[0]).split(os.sep)
    scriptPath.reverse()
    for p in scriptPath: 
        if p in ["cpp", "cs", "java", "php", "py", "rb", "tmp"]:
            return p

    scriptPath = os.path.abspath(os.getcwd()).split(os.sep)
    scriptPath.reverse()
    for p in scriptPath: 
        if p in ["cpp", "cs", "java", "php", "py", "rb", "tmp"]:
            return p

    #  Default to C++
    return "cpp"

def getTestEnv():
    env = {}
    env["certsdir"] = os.path.abspath(os.path.join(findTopLevel(), "certs"))
    return env 

class DriverConfig:
    lang = None
    protocol = None 
    compress = 0
    serialize = 0
    host = None 
    mono = False
    type = None
    overrides = None
    ipv6 = False
    x64 = False

    def __init__(self, type = None):
        global protocol
        global compress
        global serialize
        global host 
        global mono
        global ipv6
        global x64
        self.lang = getDefaultMapping()
        self.protocol = protocol
        self.compress = compress
        self.serialize = serialize
        self.host = host
        self.mono = mono
        self.type = type
        self.ipv6 = ipv6
        self.x64 = x64
        
def argsToDict(argumentString, results):
    """Converts an argument string to dictionary"""
    args = argumentString.strip()
    while len(args) > 0:
        end = args.find(" --")
        if end == -1:
            current = args.strip()
            args = ""
        else:
            current = args[:end].strip()
            args = args[end:].strip()

        value = current.find("=")
        if value != -1:
            results[current[:value]] = current[value+1:]
        else:
            results[current] = None
    return results
            
def getCommandLine(exe, config, env=None):

    if not env:
        env = getTestEnv()
        
    #
    # Command lines are built up from the items in the components
    # sequence, which is initialized with command line options common to
    # all test drivers.
    #
    components = ["--Ice.NullHandleAbort=1", "--Ice.Warn.Connections=1"]

    #
    # Turn on network tracing.
    #
    # components.append("--Ice.Trace.Network=3")

    #
    # Now we add additional components dependent on the desired
    # configuration.
    #
    if config.protocol == "ssl":
        components.append(sslConfigTree[config.lang]["plugin"] % env)
        components.append(sslConfigTree[config.lang][config.type] % env)

    if config.compress:
        components.append("--Ice.Override.Compress=1")

    if config.serialize:
        components.append("--Ice.ThreadPool.Server.Serialize=1")
        
    if config.type == "server" or config.type == "colloc" and config.lang == "py":
        components.append("--Ice.ThreadPool.Server.Size=1 --Ice.ThreadPool.Server.SizeMax=3 --Ice.ThreadPool.Server.SizeWarn=0")

    if config.type == "server":
        components.append("--Ice.PrintProcessId=1 --Ice.PrintAdapterReady=1 --Ice.ServerIdleTime=30")

    if config.ipv6:
        components.append("--Ice.Default.Host=0:0:0:0:0:0:0:1 --Ice.IPv6=1")
    elif config.host != None and len(config.host) != 0:
        components.append("--Ice.Default.Host=%s" % config.host)

    #
    # Not very many tests actually require an option override, so not to worried
    # about optimal here.
    #
    if config.overrides != None and len(config.overrides) > 0:
        propTable = {}
        for c in components:
            argsToDict(c, propTable)
                    
        argsToDict(config.overrides, propTable)
        components = []
        for k, v in propTable.iteritems():
            if v != None:
                components.append("%s=%s" % (k, v))
            else:
                components.append("%s" % k)
    
    output = StringIO.StringIO()
    if config.mono and config.lang == "cs":
        print >>output, "mono", "--debug %s.exe" % exe,
    elif config.lang == "rb" and config.type == "client":
        print >>output, "ruby", exe,
    elif config.lang == "java":
        print >>output, "%s -ea" % javaCmd,
        if isSolaris() and config.x64:
            print >>output, "-d64",
        if not config.ipv6:
            print >>output, "-Djava.net.preferIPv4Stack=true",
        print >>output,  exe,
    elif config.lang == "py":
        print >>output, "python", exe,
    elif config.lang == "php" and config.type == "client":
        print >>output, phpCmd, "-c tmp.ini -f", exe, " -- ",
    else:
        print >>output, exe,

    for c in components:
        print >>output, c,
    commandline = output.getvalue()
    output.close()

    return commandline

def runTests(start, expanded, num = 0, script = False):
    total = 0
    for tests in expanded:
        for i, args, config in tests:
            total = total + 1
    #
    # The configs argument is a list containing one or more test configurations.
    #
    index = 0
    for tests in expanded:
        for i, args, config in tests:
            index = index + 1
            if index < start:
                continue
            i = os.path.normpath(i)
            dir = os.path.join(toplevel, i)

            print
            if num > 0:
                print "[" + str(num) + "]",
            if script:
                prefix = "echo \""
                suffix = "\""
            else:
                prefix = ""
                suffix = ""

            print "%s*** running tests %d/%d in %s%s" % (prefix, index, total, dir, suffix)
            print "%s*** configuration:" % prefix,
            if len(args.strip()) == 0:
                print "Default",
            else:
                print args.strip(),
            print suffix

            #
            # Skip tests not supported with IPv6 if necessary
            #
            if args.find("ipv6") != -1 and "noipv6" in config:
                print "%s*** test not supported with IPv6%s" % (prefix, suffix)
                continue

            if isVista() and "novista" in config:
                print "%s*** test not supported under Vista%s" % (prefix, suffix)
                continue

            if not isWin32() and "win32only" in config:
                print "%s*** test only supported under Win32%s" % (prefix, suffix)
                continue

            # If this is mono and we're running ssl protocol tests
            # then skip. This occurs when using --all.
            if mono and ("nomono" in config or (i.find(os.path.join("cs","test")) != -1 and args.find("ssl") != -1)):
                print "%s*** test not supported with mono%s" % (prefix, suffix)
                continue

            # If this is java and we're running ipv6 under windows then skip.
            if isWin32() and i.find(os.path.join("java","test")) != -1 and args.find("ipv6") != -1:
                print "%s*** test not supported under windows%s" % (prefix, suffix)
                continue

            if script:
                print "echo \"*** test started: `date`\""
                print "cd %s" % dir
            else:
                print "*** test started:", time.strftime("%x %X")
                sys.stdout.flush()

                os.chdir(dir)

            global keepGoing
            if script:
                print "if ! python %s %s; then" % (os.path.join(dir, "run.py"), args)
                print "  echo 'test in %s failed'" % os.path.abspath(dir)
                if not keepGoing:
                    print "  exit 1"
                print "fi"
            else:
                status = os.system("python " + os.path.join(dir, "run.py " + args))

                if status:
                    if(num > 0):
                        print "[" + str(num) + "]",
                    message = "test in " + os.path.abspath(dir) + " failed with exit status", status,
                    print message
                    if not keepGoing:
                        sys.exit(status)
                    else:
                        print " ** Error logged and will be displayed again when suite is completed **"
                        global testErrors
                        testErrors.append(message)

def getDefaultServerFile():
    lang = getDefaultMapping()
    if lang in ["rb", "php", "cpp", "cs"]:
        return "server"
    if lang == "py":
        return "Server.py"
    if lang == "java":
        return "Server"

def getDefaultClientFile():
    lang = getDefaultMapping()
    if lang == "rb":
        return "Client.rb"
    if lang == "php":
        return "Client.php"
    if lang in ["cpp", "cs"]:
        return "client"
    if lang == "py":
        return "Client.py"
    if lang == "java":
        return "Client"

def getDefaultCollocatedFile():
    lang = getDefaultMapping()
    if lang == "rb":
        return "Collocated.rb"
    if lang == "php":
        return "Collocated.php"
    if lang in ["cpp", "cs"]:
        return "collocated"
    if lang == "py":
        return "Collocated.py"
    if lang == "java":
        return "Collocated"

def isDebug():
    return debug

def clientServerTestWithOptionsAndNames(name, additionalServerOptions, additionalClientOptions, \
                                        serverName, clientName):
    lang = getDefaultMapping()
    testdir = os.path.join(findTopLevel(), lang, "test", name)
    
    server = serverName
    client = clientName
    
    if lang != "java":
        if lang in ["rb", "php"]:
            server = os.path.join(findTopLevel(), "cpp", "test", name, serverName)
        else:
            server = os.path.join(testdir, serverName)
        client = os.path.join(testdir, clientName)
    
    print "starting " + serverName + "...",
    serverCfg = DriverConfig("server")
    if lang in ["rb", "php"]:
        serverCfg.lang = "cpp"
    serverCmd = getCommandLine(server, serverCfg) + " " + additionalServerOptions
    if debug:
        print "(" + serverCmd + ")",
    serverPipe = os.popen(serverCmd + " 2>&1")
    if lang != "java":
        getServerPid(serverPipe)
    getAdapterReady(serverPipe)
    print "ok"

    cwd = os.getcwd()
    os.chdir(testdir)

    if lang == "php":
        writePhpIni("php.ini", "tmp.ini")

    print "starting " + clientName + "...",
    clientCmd = getCommandLine(client, DriverConfig("client")) + " " + additionalClientOptions
    if debug:
        print "(" + clientCmd + ")",
    clientPipe = os.popen(clientCmd + " 2>&1")
    print "ok"

    printOutputFromPipe(clientPipe)

    clientStatus = closePipe(clientPipe)
    if clientStatus:
        killServers()

    joinServers()

    if lang == "php":
	os.remove("tmp.ini")

    os.chdir(cwd)

    if clientStatus or serverStatus():
        sys.exit(1)

def clientServerTestWithOptions(name, additionalServerOptions, additionalClientOptions):

    clientServerTestWithOptionsAndNames(name, additionalServerOptions, additionalClientOptions, getDefaultServerFile(),
                                        getDefaultClientFile())

def clientServerTest(name):

    clientServerTestWithOptions(name, "", "")

def clientServerTestWithClasspath(name, serverClasspath, clientClasspath):

    cp = os.getenv("CLASSPATH", "")
    scp = serverClasspath + os.pathsep + cp
    ccp = clientClasspath + os.pathsep + cp

    print "starting server...",
    os.environ["CLASSPATH"] = scp
    serverPipe = startServer(getDefaultServerFile(), "")
    os.environ["CLASSPATH"] = cp

    getAdapterReady(serverPipe)
    print "ok"

    print "starting client...",
    os.environ["CLASSPATH"] = ccp
    clientPipe = startClient(getDefaultClientFile(), "")
    os.environ["CLASSPATH"] = cp
    print "ok"

    printOutputFromPipe(clientPipe)
    clientStatus = closePipe(clientPipe)

    if clientStatus or serverStatus():
        killServers()
        sys.exit(1)

def mixedClientServerTestWithOptions(name, additionalServerOptions, additionalClientOptions):

    testdir = os.path.join(findTopLevel(), getDefaultMapping(), "test", name)
    lang = getDefaultMapping()
    server =  getDefaultServerFile()
    client = getDefaultClientFile()
    if lang != "java":
        server = os.path.join(testdir, server)
        client = os.path.join(testdir, client)

    print "starting server...",
    serverCmd = getCommandLine(server, DriverConfig("server")) + ' ' + additionalServerOptions
    if debug:
        print "(" + serverCmd + ")",
    serverPipe = os.popen(serverCmd + " 2>&1")
    if lang != "java":
        getServerPid(serverPipe)
    getAdapterReady(serverPipe)
    print "ok"
    
    print "starting client...",
    clientCmd = getCommandLine(client, DriverConfig("client")) + ' ' + additionalClientOptions
    if debug:
        print "(" + clientCmd + ")",
    clientPipe = os.popen(clientCmd + " 2>&1")
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

    testdir = os.path.join(findTopLevel(), getDefaultMapping(), "test", name)
    lang = getDefaultMapping()
    collocated = getDefaultCollocatedFile()
    if lang != "java":
        collocated = os.path.join(testdir, collocated) 

    print "starting collocated...",
    command = getCommandLine(collocated, DriverConfig("colloc")) + ' ' + additionalOptions 
    if debug:
        print "(" + command + ")",
    collocatedPipe = os.popen(command + " 2>&1")
    print "ok"

    printOutputFromPipe(collocatedPipe)

    collocatedStatus = closePipe(collocatedPipe)

    if collocatedStatus:
        sys.exit(1)

def collocatedTest(name):

    collocatedTestWithOptions(name, "")

def cleanDbDir(path):
    for filename in [ os.path.join(path, f) for f in os.listdir(path) if f != ".gitignore" and f != "DB_CONFIG" ]:
	os.remove(filename)

def startClient(exe, args, config=None, env=None):
    if config == None:
        config = DriverConfig("client")
    if debug:
        print "(" + getCommandLine(exe, config, env) + ' ' + args + ")",

    if config.lang == "php":
        os.chdir(os.path.dirname(os.path.abspath(exe)))
        writePhpIni("php.ini", "tmp.ini")

    return os.popen(getCommandLine(exe, config, env) + ' ' + args + " 2>&1")

def startServer(exe, args, config=None, env=None):
    if config == None:
        config = DriverConfig("server")
    if debug:
        print "(" + getCommandLine(exe, config, env) + ' ' + args + ")",
    return os.popen(getCommandLine(exe, config, env) + ' ' +args + " 2>&1")

def startColloc(exe, args, config=None, env=None):
    if config == None:
        config = DriverConfig("colloc")
    if debug:
        print "(" + getCommandLine(exe, config, env) + ' ' + args + ")",
    return os.popen(getCommandLine(exe, config, env) + ' ' + args + " 2>&1")

def getMappingDir(currentDir):
    return os.path.abspath(os.path.join(findTopLevel(), getDefaultMapping(currentDir)))

def getCppBinDir():
    binDir = os.path.join(getIceDir("cpp"), "bin")
    if ice_home and x64:
        if isHpUx():
            binDir = os.path.join(binDir, "pa20_64")
        elif isSolaris():
            if isSparc():
                binDir = os.path.join(binDir, "sparcv9")
            else:
                binDir = os.path.join(binDir, "amd64")
        elif isWin32():
            binDir = os.path.join(binDir, "x64")
    return binDir

def processCmdLine():
    def usage():
        print "usage: " + sys.argv[0] + """
          --debug                 Display debugging information on each test.
          --protocol=tcp|ssl      Run with the given protocol.
          --compress              Run the tests with protocol compression.
          --host=host             Set --Ice.Default.Host=<host>.
          --serialize             Run with connection serialization.
          --ipv6                  Use IPv6 addresses.
          --ice-home=<path>       Use the binary distribution from the given path.
          --x64                   Binary distribution is 64-bit.
        """
        sys.exit(2)

    try:
        opts, args = getopt.getopt(
            sys.argv[1:], "", ["debug", "protocol=", "compress", "host=", "serialize", "ipv6", \
                              "ice-home=", "x64"])
    except getopt.GetoptError:
        usage()

    if args:
        usage()

    for o, a in opts:
        if o == "--ice-home":
            global ice_home
            ice_home = a
        elif o == "--x64":
            global x64
            x64 = True
        elif o == "--compress":
            global compress
            compress = True
        elif o == "--serialize":
            global serialize
            serialize = True
        elif o == "--host":
            global host
            host = a
        elif o == "--ipv6":
            global ipv6
            ipv6 = True
        elif o == "--debug":
            global debug
            debug = True
        elif o == "--protocol":
            if a not in ( "ssl", "tcp"):
                usage()
            # ssl protocol isn't directly supported with mono.
            if mono and getDefaultMapping() == "cs" and a == "ssl":
                print "SSL is not supported with mono"
                sys.exit(1)
            global protocol
            protocol = a

    if len(args) > 0:
        usage()

    # Only use binary distribution from ICE_HOME environment variable if USE_BIN_DIST=yes
    if not ice_home and os.environ.get("USE_BIN_DIST", "no") == "yes":
        if os.environ.get("ICE_HOME", "") != "":
            ice_home = os.environ["ICE_HOME"]
        elif isLinux():
            ice_home = "/usr"
            
    if not x64:
        x64 = isWin32() and os.environ.get("XTARGET") == "x64" or os.environ.get("LP64") == "yes"
    
    configurePaths()

if os.environ.has_key("ICE_CONFIG"):
    os.unsetenv("ICE_CONFIG")
