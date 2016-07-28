# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os, re, getopt, time, string, threading, atexit, platform, traceback, subprocess

# Global flags and their default values.
protocol = ""                   # If unset, default to TCP. Valid values are "tcp", "ssl", "ws", "wss", "bt" or "bts".
compress = False                # Set to True to enable bzip2 compression.
serialize = False               # Set to True to have tests use connection serialization
host = None                     # Will default to loopback.
debug = False                   # Set to True to enable test suite debugging.
mono = False                    # Set to True when not on Windows
keepGoing = False               # Set to True to have the tests continue on failure.
ipv6 = False                    # Default to use IPv4 only
socksProxy = False              # Use SOCKS proxy running on localhost
global x64
x64 = False                     # Binary distribution is 64-bit
global x86
x86 = False                     # Binary distribution is 32-bit
global armv7l
armv7l = False                  # Binary distribution is armv7l
cpp11 = False                   # Binary distribution is c++11
static = False                  # Static build
global buildMode
buildMode = None

extraArgs = []
clientTraceFilters = []
serverTraceFilters = []

# Default java loader

javaHome = os.environ.get("JAVA_HOME", "")
javaCmd = '"%s"' % os.path.join(javaHome, "bin", "java") if javaHome else "java"
javaLibraryPath = None

valgrind = False                # Set to True to use valgrind for C++ executables.
appverifier = False             # Set to True to use appverifier for C++ executables, This is windows only feature
tracefile = None
printenv = False
cross = []
watchDog = None
clientHome = None
serviceDir = None
compact = False
global winrt
winrt = False
global serverOnly
serverOnly = False
mx = False
controller = None
configName = None

queuedTests = []

global additionalBinDirectories
additionalBinDirectories = []

def addAdditionalBinDirectories(directories):
    global additionalBinDirectories
    additionalBinDirectories += directories

def resetAdditionalBinDirectories():
    global additionalBinDirectories
    additionalBinDirectories = []

#
# Linux distribution
#
linuxDistribution = None
for path in ["/etc/redhat-release", "/etc/issue"]:
    if not os.path.isfile(path):
        continue

    f = open(path, "r")
    issue = f.read()
    f.close()

    if issue.find("Red Hat") != -1:
        linuxDistribution = "RedHat"
    elif issue.find("Amazon Linux") != -1:
        linuxDistribution = "Amazon"
    elif issue.find("CentOS") != -1:
        linuxDistribution = "CentOS"
    elif issue.find("Ubuntu") != -1:
        linuxDistribution = "Ubuntu"
    elif issue.find("Debian") != -1:
        linuxDistribution = "Debian"
    elif issue.find("SUSE Linux") != -1:
        linuxDistribution = "SUSE LINUX"
    elif issue.find("Yocto") != -1:
        linuxDistribution = "Yocto"

def isCygwin():
    # The substring on sys.platform is required because some cygwin
    # versions return variations like "cygwin_nt-4.01".
    return sys.platform[:6] == "cygwin"

def isWin32():
    return sys.platform == "win32" or isCygwin()

def isVista():
    return isWin32() and sys.getwindowsversion()[0] == 6 and sys.getwindowsversion()[1] == 0

def isWin9x():
    if isWin32():
        return not ("OS" in os.environ and os.environ["OS"] == "Windows_NT")
    else:
        return 0

def isSolaris():
    return sys.platform == "sunos5"

def isSparc():
    p = subprocess.Popen("uname -p", stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True)
    if not p or not p.stdout:
        print("unable to get system information!")
        sys.exit(1)
    l = p.stdout.readline().decode("utf-8").strip()
    if l == "sparc":
        return True
    else:
        return False

def dpkgHostMultiArch():
    p = subprocess.Popen("dpkg-architecture -qDEB_HOST_MULTIARCH", stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True)
    if not p or not p.stdout:
        print("unable to get system information!")
        sys.exit(1)
    return p.stdout.readline().decode("utf-8").strip()

def isI686():
    return x86 or any(platform.machine() == p for p in ["i386", "i686"])

def isAIX():
    return sys.platform.startswith("aix")

def isDarwin():
    return sys.platform == "darwin"

def isLinux():
    return sys.platform.startswith("linux") or sys.platform.startswith("gnukfreebsd")

def isUbuntu():
    return isLinux() and linuxDistribution and linuxDistribution == "Ubuntu"

def isRhel():
    return isLinux() and linuxDistribution in ["RedHat", "Amazon", "CentOS"]

def isYocto():
    return isLinux() and linuxDistribution and linuxDistribution == "Yocto"

def isDebian():
    return isLinux() and linuxDistribution and linuxDistribution == "Debian"

def isSles():
    return isLinux() and linuxDistribution and linuxDistribution == "SUSE LINUX"

def iceUseOpenSSL():
    return any(sys.platform.startswith(p) for p in ["linux", "freebsd"])

def getCppCompiler():
    compiler = ""
    if os.environ.get("CPP_COMPILER", "") != "":
        compiler = os.environ["CPP_COMPILER"]
    elif isMINGW():
        compiler = "MINGW"
    else:
        p = subprocess.Popen("cl", stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True)
        if not p or not p.stdout:
            print("Cannot detect C++ compiler")
            compiler = VC120
        else:
            l = p.stdout.readline().decode("utf-8").strip()
            if l.find("Version 16.") != -1:
                compiler = "VC100"
            elif l.find("Version 17.") != -1:
                compiler = "VC110"
            elif l.find("Version 18.") != -1:
                compiler = "VC120"
            elif l.find("Version 19.") != -1:
                compiler = "VC140"
            else:
                #
                # Cannot detect C++ compiler use default
                #
                compiler = "VC140"
    return compiler

def isMINGW():
    if not isWin32():
        return False
    # Ruby Installer DEVKIT sets the RI_DEVKIT environment variable,
    # we check for this variable to detect the Ruby MINGW environment.
    return "RI_DEVKIT" in os.environ

def isVC100():
    if not isWin32():
        return False
    return getCppCompiler() == "VC100"

def isVC110():
    if not isWin32():
        return False
    return getCppCompiler() == "VC110"

def isVC120():
    if not isWin32():
        return False
    return getCppCompiler() == "VC120"

def isVC140():
    if not isWin32():
        return False
    return getCppCompiler() == "VC140"

def getIceSoVersion():
    config = open(os.path.join(toplevel, "cpp", "include", "IceUtil", "Config.h"), "r")
    intVersion = int(re.search("ICE_INT_VERSION ([0-9]*)", config.read()).group(1))
    majorVersion = int(intVersion / 10000)
    minorVersion = int(intVersion / 100) - 100 * majorVersion
    patchVersion = intVersion % 100
    if patchVersion < 50:
        return '%d' % (majorVersion * 10 + minorVersion)
    elif patchVersion < 60:
        return '%da%d' % (majorVersion * 10 + minorVersion, patchVersion - 50)
    else:
        return '%db%d' % (majorVersion * 10 + minorVersion, patchVersion - 60)

def getIceJsonVersion():
    r = re.search(r"([0-9]+)\.([0-9]+)(\.[0-9]+|[ab][0-9]+)", iceVersion)
    major = int(r.group(1))
    minor = int(r.group(2))
    v = ("%d.%d" % (major, minor)).strip()
    if r.group(3).startswith("a"):
        return v + ".0-" + r.group(3).replace("a", "alpha").strip()
    elif r.group(3).startswith("b"):
        return v + ".0-" + r.group(3).replace("b", "beta").strip()
    else:
        return ("%s%s" % (v, r.group(3))).strip()

def getJdkVersion():
    process = subprocess.Popen("java -version", stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True)
    if not process or not process.stdout:
        print("unable to get Java version!")
        sys.exit(1)
    global jdkVersion
    jdkVersion = process.stdout.readline()
    if not jdkVersion:
        print("unable to get Java version!")
        sys.exit(1)
    return jdkVersion.decode("utf-8")

def getWinRegistryKeyValue(key, subKey):
    import winreg
    try:
        key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, key, 0, winreg.KEY_READ | winreg.KEY_WOW64_64KEY)
        try:
            return winreg.QueryValueEx(key, subKey)
        finally:
            winreg.CloseKey(key)
    except:
        pass

def sanitize(cp):
    np = ""
    for p in cp.split(os.pathsep):
        if len(np) > 0:
            np = np + os.pathsep
        np = np + p
    return np

def quoteArgument(arg):
    if arg is None:
        return None
    return '"%s"' % arg

def dumpenv(env, lang):
    if env is None:
        env = os.environ
    vars = ["PATH", "LD_LIBRARY_PATH", "DYLD_LIBRARY_PATH", "SHLIB_PATH", "LIBPATH", "LD_LIBRARY_PATH_64"]
    if lang is None:
        vars.extend(["CLASSPATH", "MONO_PATH", "DEVPATH", "PYTHONPATH", "RUBYLIB"])
    elif lang == "cpp":
        pass
    elif lang == "java":
        vars.append("CLASSPATH")
    elif lang == "csharp":
        vars.extend(["MONO_PATH", "DEVPATH"])
    elif lang == "python":
        vars.append("PYTHONPATH")
    elif lang == "ruby":
        vars.append("RUBYLIB")
    elif lang == "js":
        vars.append("NODE_PATH")
    for i in vars:
        if i in env:
            print("%s=%s" % (i, env[i]))

def addLdPath(libpath, env = None):
    if env is None:
        env = os.environ
    if isWin32():
        addPathToEnv("PATH", libpath, env)
    elif isDarwin():
        addPathToEnv("DYLD_LIBRARY_PATH", libpath, env)
    elif isAIX():
        addPathToEnv("LIBPATH", libpath, env)
    else:
        addPathToEnv("LD_LIBRARY_PATH", libpath, env)
        addPathToEnv("LD_LIBRARY_PATH_64", libpath, env)
    return env

def addClasspath(path, env = None):
    return addPathToEnv("CLASSPATH", path, env)

def addPathToEnv(variable, path, env = None):
    if env is None:
        env = os.environ
    if variable not in env:
        env[variable] = path
    else:
        env[variable] = path + os.pathsep + env.get(variable)
    return env

#
# This is set by the choice of init method. If not set, before it is
# used, it indicates a bug and things should terminate.
#
defaultMapping = None
testErrors = []
toplevel = None
testToplevel = None

path = [ ".", "..", "../..", "../../..", "../../../..", "../../../../..", "../../../../../..",
         "../../../../../../..", "../../../../../../../..", "../../../../../../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) == 0:
    head = "."
# Try to find scripts/TestUtil.py or ice/scripts/TestUtil.py in parent directories
path = [os.path.join(head, p) for p in path] + [os.path.join(head, p, "ice") for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
toplevel = path[0]
testToplevel = toplevel

#
# Set the default arch to x64 on x64 machines, this could be overriden
# with the --x86 command line argument.
#
if isWin32():
    if os.environ.get("PLATFORM", "").upper() == "X64":
        x64 = True
elif isAIX():
    if os.environ.get("OBJECT_MODE", "") == "64":
        x64 = True
else:
    p = subprocess.Popen("uname -m", shell = True, stdout = subprocess.PIPE, stderr = subprocess.STDOUT)
    if(p.wait() != 0):
        print("uname failed:\n" + p.stdout.read().strip())
        sys.exit(1)
    line = p.stdout.readline().decode('UTF-8').strip()
    if line == "x86_64" and os.environ.get("LP64", "") != "no":
        x64 = True
    elif line == "armv7l":
        armv7l = True

#
# The PHP interpreter is called "php5" on some platforms (e.g., SLES).
#
phpCmd = "php"
for path in os.environ["PATH"].split(os.pathsep):
    #
    # Stop if we find "php" in the PATH first.
    #
    if os.path.exists(os.path.join(path, "php")):
        break
    elif os.path.exists(os.path.join(path, "php5")):
        phpCmd = "php5"
        break

#
# The NodeJS interpreter is called "nodejs" on some platforms
# (e.g., Ubuntu)
#
nodeCmd = "node"
if "NODE" in os.environ:
    nodeCmd = os.environ["NODE"]
else:
    for path in os.environ["PATH"].split(os.pathsep):
        #
        # Stop if we find "node" in the PATH first.
        #
        if os.path.exists(os.path.join(path, "node")):
            break
        elif os.path.exists(os.path.join(path, "nodejs")):
            nodeCmd = "nodejs"
            break

#
# Figure out the Ice version
#
iceVersion = None
try:
    if isWin32():
        config = open(os.path.join(toplevel, "config", "icebuilder.props"), "r")
        iceVersion = re.search("<IceVersion>[\t\s]*([0-9]+\.[0-9]+(\.[0-9]+|[ab][0-9]*))</IceVersion>", config.read()).group(1)
    else:
        config = open(os.path.join(toplevel, "config", "Make.rules"), "r")
        iceVersion = re.search("version[\t\s]*= ([0-9]+\.[0-9]+(\.[0-9]+|[ab][0-9]*))", config.read()).group(1)
    config.close()
except:
    print("error: couldn't figure Ice version")
    sys.exit(1)

#
# Figure out Ice installation directory
#
iceHome = None  # Binary distribution to use (or None to use binaries from source distribution)
if os.environ.get("USE_BIN_DIST", "no") == "yes":
    # Only use binary distribution from ICE_HOME environment variable if USE_BIN_DIST=yes
    if os.environ.get("ICE_HOME", "") != "":
        iceHome = os.environ["ICE_HOME"]
    elif isLinux():
        iceHome = "/usr"
    elif isDarwin():
        iceHome = "/usr/local"
    elif isWin32():
        path = getWinRegistryKeyValue("SOFTWARE\\ZeroC\\Ice %s" % iceVersion, "InstallDir")
        if path and len(path) > 0 and os.path.exists(path[0]):
            iceHome = path[0]

# List of supported cross-language tests.
crossTests = [ #"Ice/adapterDeactivation",
               #"Ice/background",
               #"Ice/binding",
               #"Ice/checksum",
               #"Ice/custom",
               "Ice/ami",
               "Ice/info",
               "Ice/exceptions",
               "Ice/enums",
               "Ice/facets",
               #"Ice/hold",
               "Ice/inheritance",
               "Ice/invoke",
               #"Ice/location",
               "Ice/objects",
               "Ice/operations",
               "Ice/proxy",
               #"Ice/retry",
               "Ice/servantLocator",
               #"Ice/timeout",
               "Ice/slicing/exceptions",
               "Ice/slicing/objects",
               "Ice/optional",
               ]

def run(tests, root = False):
    def usage():
        print("usage: " + sys.argv[0] + """
          --all                Run all sensible permutations of the tests.
          --all-cross          Run all sensible permutations of cross language tests.
          --start=index        Start running the tests at the given index.
          --loop               Run the tests in a loop.
          --filter=<regex>     Run all the tests that match the given regex.
          --rfilter=<regex>    Run all the tests that do not match the given regex.
          --debug              Display debugging information on each test.
          --protocol=<prot>    Run with the given protocol (tcp|ssl|ws|wss|bt).
          --compress           Run the tests with protocol compression.
          --host=host          Set --Ice.Default.Host=<host>.
          --valgrind           Run the test with valgrind.
          --appverifier        Run the test with appverifier under Windows.
          --serialize          Run with connection serialization.
          --continue           Keep running when a test fails.
          --ipv6               Use IPv6 addresses.
          --socks              Use SOCKS proxy running on localhost.
          --no-ipv6            Don't use IPv6 addresses.
          --ice-home=<path>    Use the binary distribution from the given path.
          --mode=debug|release Run the tests with debug or release mode builds (win32 only)."
          --x86                Binary distribution is 32-bit.
          --x64                Binary distribution is 64-bit.
          --c++11              Binary distribution is c++11.
          --static             Binary distribution is static.
          --cross=lang         Run cross language test.
          --client-home=<dir>  Run cross test clients from the given Ice source distribution.
          --script             Generate a script to run the tests.
          --env                Print important environment variables.
          --service-dir=<dir>  Where to locate services for builds without service support.
          --compact            Ice for .NET uses the Compact Framework.
          --winrt              Run server with configuration suited for WinRT client.
          --server             Run only the server.
          --mx                 Enable IceMX when running the tests.
          --controller=<host>  Use the test controller on the specified host.
          --arg=<property>     Append the given argument.
        """)
        sys.exit(2)

    try:
        opts, args = getopt.getopt(sys.argv[1:], "lr:R:",
                                   ["start=", "start-after=", "filter=", "rfilter=", "all", "all-cross", "loop",
                                    "debug", "protocol=", "compress", "valgrind", "host=", "serialize", "continue",
                                    "ipv6", "no-ipv6", "socks", "ice-home=", "mode=", "cross=", "client-home=", "x64", "x86",
                                    "script", "env", "arg=", "service-dir=", "appverifier", "compact",
                                    "winrt", "server", "mx", "c++11", "static", "controller=", "configName="])
    except getopt.GetoptError:
        usage()

    if args:
        usage()

    start = 0
    loop = False
    all = False
    allCross = False
    arg = ""
    script = False
    noipv6 = False
    compact = "--compact" in opts
    winrt = "--winrt" in opts
    serverOnly = "--server" in opts
    mx = "--mx" in opts
    controller = "--controller" in opts
    configName = "--configName" in opts

    filters = []
    for o, a in opts:
        if o == "--continue":
            global keepGoing
            keepGoing = True
        elif o in ("-l", "--loop"):
            loop = True
        elif o in ("-r", "-R", "--filter", '--rfilter'):
            testFilter = re.compile(a)
            if o in ("--rfilter", "-R"):
                filters.append((testFilter, True))
            else:
                filters.append((testFilter, False))
        elif o == "--cross":
            global cross
            crossLang = ["cpp", "csharp", "java", "js", "php", "python", "ruby", "objective-c"]
            if a not in crossLang:
                print("cross must be one of %s" % ', '.join(crossLang))
                sys.exit(1)
            cross.append(a)
        elif o == "--all" :
            all = True
        elif o == "--no-ipv6" :
            noipv6 = True
        elif o == "--all-cross" :
            allCross = True
        elif o in '--start':
            start = int(a)
        elif o == "--script":
            script = True
        elif o == '--arg':
            arg += " --arg="
            arg += '"'
            arg += a
            arg += '"'
        elif o == "--protocol":
            if a not in ("bt", "bts", "ws", "wss", "ssl", "tcp"):
                usage()
            if not root and getDefaultMapping() == "csharp" and (a == "ssl" or a == "wss"):
                if mono:
                    print("SSL is not supported with mono")
                    sys.exit(1)
                if compact:
                    print("SSL is not supported with the Compact Framework")
                    sys.exit(1)
            if a in ["bt", "bts"] and not isLinux():
                print("Bluetooth is only supported on Linux")
                sys.exit(1)
        elif o == "--c++11":
            global cpp11
            cpp11 = True
        elif o == "--static":
            global static
            static = True
        elif o == "--x86":
            global x86
            x86 = True
        elif o == "--x64":
            global x64
            x64 = True
        if o in ( "--cross", "--protocol", "--host", "--debug", "--compress", "--valgrind", "--serialize", "--ipv6", \
                  "--socks", "--ice-home", "--mode", "--x86", "--x64", "--c++11", "--static", "--env", \
                  "--service-dir", "--appverifier", "--compact", "--winrt", \
                  "--server", "--mx", "--client-home", "--controller", "--configName"):
            arg += " " + o
            if len(a) > 0:
                arg += " " + a

    if not root:
        tests = [ (os.path.join(getDefaultMapping(), "test", x), y) for x, y in tests ]

    # Expand all the test and argument combinations.
    expanded = []
    if all:
        expanded.append([(test, arg, config) for test,config in tests if "once" in config ])

        a = '--protocol=tcp %s'  % arg
        expanded.append([ (test, a, config) for test,config in tests if "core" in config])

        a = '--protocol=ssl %s'  % arg
        expanded.append([ (test, a, config) for test,config in tests if "core" in config])

        a = '--protocol=ws %s'  % arg
        expanded.append([ (test, a, config) for test,config in tests if "core" in config])

        if not noipv6:
            a = "--ipv6 --protocol=wss --compress --mx --serialize %s" % arg
            expanded.append([ (test, a, config) for test,config in tests if "core" in config])

        a = "--protocol=tcp %s" % arg
        expanded.append([ (test, a, config) for test,config in tests if "service" in config])

        if not noipv6:
            a = "--ipv6 --protocol=ssl --compress --mx --serialize %s" % arg
            expanded.append([ (test, a, config) for test,config in tests if "service" in config])

    elif not allCross:
        expanded.append([ (test, arg, config) for test,config in tests])

    if allCross:
        if len(cross) == 0:
            cross = ["cpp", "java", "js", "php", "python", "ruby"]
            if isWin32():
                cross.append("csharp")

                # TODO: PHP only builds against VC140
                if getCppCompiler() != "VC140":
                    cross.remove('php')

                # TODO: We're currently not building Ruby on Windows
                cross.remove('ruby')
            if isDarwin():
                cross.append("objective-c")
        if root:
            allLang = ["cpp", "java", "js", "python"]
            if isWin32():
                allLang.append("csharp")
            if isDarwin():
                allLang.append("objective-c")
        else:
            allLang = [ getDefaultMapping() ]
        for lang in allLang:
            # js test user server for other language so we can ignore this
            if lang == "js":
                continue
            # This is all other languages than the current mapping.
            crossLang = [ l for l in cross if lang != l ]
            # This is all eligible cross tests for the current mapping.
            # Now expand out the tests. We run only tcp for most cross tests.
            for c in crossLang:
                a = "--cross=%s --protocol=tcp %s" % (c, arg)
                for test in crossTests:
                    name = "%s/test/%s" % (lang, test)
                    expanded.append([(name, a, testConfig(name, tests))])

                # Add ssl & compress for the operations test.
                if ((compact or mono) and c == "csharp") or (c == "js"): # Don't add the ssl tests.
                    continue
                a = "--cross=%s --protocol=ssl --compress %s" % (c, arg)
                expanded.append([("%s/test/Ice/operations" % lang, a, [])])

    # Apply filters after expanding.
    if len(filters) > 0:
        for testFilter, removeFilter in filters:
            nexpanded = []
            for tests in expanded:
                if removeFilter:
                    nexpanded.append([ (x, a, y) for x,a,y in tests if not testFilter.search(x) ])
                else:
                    nexpanded.append([ (x, a, y) for x,a,y in tests if testFilter.search(x) ])
            expanded = nexpanded

    if loop:
        num = 1
        while 1:
            runTests(start, expanded, num, script = script)
            num += 1
    else:
        runTests(start, expanded, script = script)

    global testErrors
    if len(testErrors) > 0:
        print("The following errors occurred:")
        for x in testErrors:
            print(x)

if not isWin32():
    mono = True

def testConfig(name, tests):
    for i in tests:
        if i[0] == name:
            return i[1]
    return []

def getIceDir(subdir = None, testdir = None):
    #
    # If client-home is set and if the given test directory is from a
    # sub-directory of the client home directory, run the test against
    # the client-home source distribution.
    #
    global clientHome
    if testdir and clientHome and os.path.commonprefix([testdir, clientHome]) == clientHome:
        return os.path.join(clientHome, subdir)

    #
    # If ICE_HOME is set we're running the test against a binary distribution. Otherwise,
    # we're running the test against a source distribution.
    #
    global iceHome
    if iceHome:
        return iceHome
    elif subdir:
        return os.path.join(toplevel, subdir)
    else:
        return toplevel

def getSliceDir():
    #
    # If ICE_HOME is set we're running the test against a binary distribution. Otherwise,
    # we're running the test against a source distribution.
    #
    global iceHome
    if iceHome:
        if isDarwin() and iceHome == "/usr/local":
            return "/usr/local/share/slice"
        elif isLinux() and iceHome == "/usr":
            return "/usr/share/Ice-" + iceVersion + "/slice"
        else:
            return os.path.join(iceHome, "slice")
    else:
        return os.path.join(toplevel, "slice")

def phpCleanup():
    if os.path.exists("ice.profiles"):
        os.remove("ice.profiles")

def phpProfileSetup(clientConfig = False, iceOptions = None, iceProfile = None):
    interpreterOptions = []

    if iceProfile != None:
        atexit.register(phpCleanup)
        interpreterOptions.append("-d ice.profiles='ice.profiles'")
        tmpProfiles = open("ice.profiles", "w")
        tmpProfiles.write("[%s]\n" % iceProfile)
        if clientConfig:
            tmpProfiles.write("ice.config=\"config.client\"\n")
        if iceOptions != None:
            tmpProfiles.write("ice.options=\"%s\"\n" % iceOptions)
        tmpProfiles.close()
    else:
        if clientConfig:
            interpreterOptions.append("-d ice.config='config.client'")
        if iceOptions != None:
            interpreterOptions.append("-d ice.options='%s'" % iceOptions)

    return ' '.join(interpreterOptions)

def phpFlags():
    flags = []
    extDir = None
    ext = None
    incDir = None

    if isWin32():
        ext = "php_ice.dll"
        if not iceHome:
            extDir = os.path.abspath(os.path.join(getIceDir("php"), "lib", "x64" if x64 else "Win32",
                "Debug" if buildMode == "debug" else "Release"))
            incDir = os.path.abspath(os.path.join(getIceDir("php"), "lib"))
        else:
            extDir = os.path.join(iceHome, "php")
            incDir = extDir
    else:
        ext = "IcePHP.so"
        if not iceHome:
            extDir = os.path.abspath(os.path.join(toplevel, "php", "lib"))
            incDir = extDir
        else:
            #
            # If ICE_HOME points to the installation directory of a source build, the
            # PHP extension will be located in $ICE_HOME/php. For an RPM installation,
            # the extension should already be in PHP's default extension directory so
            # we don't need to set extension_dir.
            #
            # We start PHP with "php -n". The -n option causes PHP to ignore its
            # default configuration file(s). We do this to suppress any warnings
            # that PHP might generate when it can't find its default extensions after
            # we modify extension_dir. Even if we don't define extension_dir, we still
            # need to add "extension = IcePHP.<ext>".
            #
            extDir = os.path.join(iceHome, "php")
            incDir = extDir

            if not os.path.exists(os.path.join(extDir, ext)):
                if iceHome == "/usr":
                    extDir = None # Assume the extension is already in PHP's extension_dir.
                    #
                    # On SLES11, the PHP run time files are installed in /usr/share/php5, but
                    # this directory is not in the default include path when you specify an
                    # alternate configuration file, so we have to configure it explicitly.
                    #
                    if os.path.exists("/usr/share/php5/Ice.php"):
                        incDir = "/usr/share/php5"
                    else:
                        incDir = None
                elif iceHome == "/usr/local" and isDarwin():
                    extDir = "/usr/local/lib/php/extensions"
                    incDir = "/usr/local/share/php"
                else:
                    print("unable to find IcePHP extension!")
                    sys.exit(1)

    if extDir:
        flags.append("-d extension_dir='%s'" % extDir)
    flags.append("-d extension='%s'" % ext)
    if incDir:
        flags.append("-d include_path='%s'" % incDir)

    return ' '.join(flags)

def getIceBox():
    global cpp11

    #
    # Get and return the path of the IceBox executable
    #
    lang = getDefaultMapping()
    if lang == "cpp":
        iceBox = "icebox"
        if isWin32():
            if cpp11:
                iceBox += "++11"
            iceBox += ".exe"
        elif isLinux():
            if isI686():
                iceBox += "32"
            if cpp11:
                iceBox += "++11"
        elif isDarwin and cpp11:
            iceBox += "++11"
        iceBox = os.path.join(getCppBinDir(lang), iceBox)
    elif lang == "java":
        iceBox = "IceBox.Server"
    elif lang == "csharp":
        iceBox = os.path.join(getIceDir("csharp"), "bin", "iceboxnet")

    return iceBox

def getIceBoxAdmin():
    if getDefaultMapping() == "java":
        return "IceBox.Admin"
    else:
        return getIceExe("iceboxadmin")

def getIceGridAdmin():
    return getIceExe("icegridadmin")

def getIceStormAdmin():
    return getIceExe("icestormadmin")

def getIceGridNode():
    return getIceExe("icegridnode")

def getIceGridRegistry():
    return getIceExe("icegridregistry")

def getGlacier2Router():
    return getIceExe("glacier2router")

def getIceExe(name):
    if isMINGW():
        return os.path.join(getServiceDir(), name)
    else:
        return os.path.join(getCppBinDir(), name)

def getNodeCommand():
    return nodeCmd

#
# Create a passwords file that contains the given users/passwords using icehashpassword.py
#
def hashPasswords(filePath, entries):
    if os.path.exists(filePath):
      os.remove(filePath)
    passwords = open(filePath, "a")

    command = "%s %s" % (sys.executable, os.path.abspath(os.path.join(os.path.dirname(__file__), "icehashpassword.py")))

    #
    # For Linux ARM default rounds makes test slower (Usually runs on embbeded boards)
    #
    if isLinux() and armv7l:
        command += " --rounds 100000"

    for user, password in entries.items():
        p = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, stdin=subprocess.PIPE)
        p.stdin.write(password.encode('UTF-8'))
        p.stdin.write('\r\n'.encode('UTF-8'))
        p.stdin.flush()
        if(p.wait() != 0):
            print("icehashpassword.py failed:\n" + p.stdout.read().decode('UTF-8').strip())
            passwords.close()
            sys.exit(1)
        passwords.write("%s %s\n" % (user, p.stdout.readline().decode('UTF-8').strip()))
    passwords.close()

class InvalidSelectorString(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

sslConfigTree = {
        "cpp" : {
            "plugin" : " --Ice.Plugin.IceSSL=IceSSL:createIceSSL --IceSSL.Password=password " +
            "--IceSSL.DefaultDir=%(certsdir)s --IceSSL.CAs=cacert.pem --IceSSL.VerifyPeer=%(verifyPeer)s",
            "client" : " --IceSSL.CertFile=client.p12",
            "server" : " --IceSSL.CertFile=server.p12",
            "colloc" : " --IceSSL.CertFile=client.p12"
            },
        "java" : {
            "plugin" : " --Ice.Plugin.IceSSL=IceSSL.PluginFactory " +
            "--IceSSL.DefaultDir=%(certsdir)s --IceSSL.Password=password --IceSSL.VerifyPeer=%(verifyPeer)s",
            "client" : " --IceSSL.Keystore=client.jks",
            "server" : " --IceSSL.Keystore=server.jks",
            "colloc" : " --IceSSL.Keystore=client.jks"
            },
        "csharp" : {
            "plugin" : " --Ice.Plugin.IceSSL=%(icesslcs)s:IceSSL.PluginFactory --IceSSL.CAs=cacert.pem " +
            "--IceSSL.Password=password --IceSSL.DefaultDir=%(certsdir)s --IceSSL.VerifyPeer=%(verifyPeer)s",
            "client" : " --IceSSL.CertFile=client.p12 --IceSSL.CheckCertName=0",
            "server" : " --IceSSL.CertFile=server.p12",
            "colloc" : " --IceSSL.CertFile=client.p12 --IceSSL.CheckCertName=0"
            },
        }

if isDarwin():
    sslConfigTree["cpp"]["client"] += " --IceSSL.Keychain=client.keychain --IceSSL.KeychainPassword=password"
    sslConfigTree["cpp"]["server"] += " --IceSSL.Keychain=server.keychain --IceSSL.KeychainPassword=password"
    sslConfigTree["cpp"]["colloc"] += " --IceSSL.Keychain=colloc.keychain --IceSSL.KeychainPassword=password"

if isWin32():
    #
    # This cipher suites doesn't work well between Java and SChannel TLS1.2 implementations.
    #
    sslConfigTree["java"]["client"] += " --IceSSL.Ciphers=!TLS_DHE_RSA_WITH_AES_128_GCM_SHA256"
    sslConfigTree["java"]["server"] += " --IceSSL.Ciphers=!TLS_DHE_RSA_WITH_AES_128_GCM_SHA256"

sslConfigTree["python"] = sslConfigTree["cpp"]
sslConfigTree["ruby"] = sslConfigTree["cpp"]
sslConfigTree["php"] = sslConfigTree["cpp"].copy()
sslConfigTree["objective-c"] = sslConfigTree["cpp"]

if isUbuntu():
    sslConfigTree["php"]["client"] += " --IceSSL.InitOpenSSL=0"

def getDefaultMapping():
    """Try to guess the language mapping from the current path"""
    here = os.getcwd().split(os.sep)
    here.reverse()
    for i in range(0, len(here)):
        if here[i] in ["cpp", "csharp", "java", "js", "php", "python", "ruby", "objective-c", "icetouch", "tmp"]:
            return here[i]
    raise RuntimeError("cannot determine mapping")

def getStringIO():
    if sys.version_info[0] == 2:
        import StringIO
        return StringIO.StringIO()
    else:
        import io
        return io.StringIO()

class DriverConfig:
    lang = None
    protocol = None
    compress = 0
    serialize = 0
    host = None
    mono = False
    valgrind = False
    appverifier = False
    type = None
    overrides = None
    ipv6 = False
    socksProxy = False
    x64 = False
    x86 = False
    cpp11 = False
    static = False
    serviceDir = None
    mx = False
    controller = None
    extraArgs = []

    def __init__(self, type = None):
        global protocol
        global compress
        global serialize
        global host
        global mono
        global valgrind
        global appverifier
        global ipv6
        global socksProxy
        global x64
        global x86
        global cpp11
        global static
        global serviceDir
        global compact
        global mx
        global controller
        global extraArgs
        self.lang = getDefaultMapping()
        self.protocol = protocol
        self.compress = compress
        self.serialize = serialize
        self.host = host
        self.mono = mono
        self.valgrind = valgrind
        self.appverifier = appverifier
        self.type = type
        self.ipv6 = ipv6
        self.socksProxy = socksProxy
        self.x64 = x64
        self.x86 = x86
        self.cpp11 = cpp11
        self.static = static
        self.serviceDir = serviceDir
        self.compact = compact
        self.mx = mx
        self.controller = controller
        self.extraArgs = extraArgs

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

def getCommandLineProperties(exe, config, cfgName):

    #
    # Command lines are built up from the items in the components
    # sequence, which is initialized with command line options common to
    # all test drivers.
    #
    components = ["--Ice.NullHandleAbort=1", "--Ice.Warn.Connections=1"]

    if config.lang == "cpp":
        components.append("--Ice.CollectObjects=1")

    #
    # Turn on network tracing.
    #
    #components.append("--Ice.Trace.Network=3")
    #components.append("--Ice.Trace.Protocol=1")

    #
    # Now we add additional components depending on the desired configuration.
    #
    if config.protocol in ["ssl", "wss", "bts"]:
        sslenv = {}
        sslenv["icesslcs"] = quoteArgument("\\\"" + os.path.join(getIceDir("csharp"), "Assemblies", "IceSSL.dll") + "\\\"")
        sslenv["certsdir"] = quoteArgument(os.path.abspath(os.path.join(toplevel, "certs")))
        if winrt or config.protocol == "wss":
            sslenv["verifyPeer"] = "0"
        else:
            sslenv["verifyPeer"] = "2"
        components.append(sslConfigTree[config.lang]["plugin"] % sslenv)
        components.append(sslConfigTree[config.lang][config.type] % sslenv)

    if config.protocol in ["bt", "bts"]:
        components.append("--Ice.Plugin.IceBT=IceBT:createIceBT")

    components.append("--Ice.Default.Protocol=" + config.protocol)

    if config.compress:
        components.append("--Ice.Override.Compress=1")

    if config.serialize:
        components.append("--Ice.ThreadPool.Server.Serialize=1")

    #
    # TODO
    # config.lang == "cpp" required with C++11 mapping, we should be able to ged rid
    # of this once AsyncResult is optimized to not required a thread pool thread with
    # collocated calls and C++11 mapping. We must also check if it still required for
    # python.
    #
    if (config.type == "server" or config.type == "colloc" and
        (config.lang == "python" or config.lang == "cpp")):
        components.append("--Ice.ThreadPool.Server.Size=1")
        components.append("--Ice.ThreadPool.Server.SizeMax=3")
        components.append("--Ice.ThreadPool.Server.SizeWarn=0")

    if config.type == "server":
        components.append("--Ice.PrintAdapterReady=1")

    if config.mx:
        if config.type == "server":
            components.append("--Ice.Admin.Endpoints=tcp")
            components.append("--Ice.Admin.InstanceName=Server")
        else:
            components.append("--Ice.Admin.Endpoints=tcp")
            components.append("--Ice.Admin.InstanceName=Client")

        components.append("--IceMX.Metrics.Debug.GroupBy=id")
        components.append("--IceMX.Metrics.Parent.GroupBy=parent")
        components.append("--IceMX.Metrics.All.GroupBy=none")

    if config.controller:
        components.append("--ControllerHost=" + config.controller)

    if cfgName:
        components.append("--ConfigName=" + cfgName)

    if config.socksProxy:
        components.append("--Ice.SOCKSProxyHost=127.0.0.1")

    if config.ipv6:
        components.append("--Ice.IPv4=1 --Ice.IPv6=1 --Ice.PreferIPv6Address=1")
        if config.host == None:
            components.append("--Ice.Default.Host=0:0:0:0:0:0:0:1")
    else:
        if not config.protocol in ["bt", "bts"]:
            components.append("--Ice.IPv4=1 --Ice.IPv6=0")
        if config.host == None and not config.protocol in ["bt", "bts"]:
            components.append("--Ice.Default.Host=127.0.0.1")

    if config.host != None and len(config.host) != 0:
        components.append("--Ice.Default.Host=%s" % config.host)

    for arg in config.extraArgs:
        components.append('--' + arg)

    #
    # Not very many tests actually require an option override, so not too worried
    # about optimal here.
    #
    if config.overrides != None and len(config.overrides) > 0:
        propTable = {}
        for c in components:
            argsToDict(c, propTable)

        argsToDict(config.overrides, propTable)
        components = []
        for k, v in propTable.items():
            if v != None:
                components.append("%s=%s" % (k, v))
            else:
                components.append("%s" % k)

    output = getStringIO()
    for c in components:
        output.write(c + " ")
    properties = output.getvalue()
    output.close()
    return properties

def getCommandLine(exe, config, options = "", interpreterOptions = "", cfgName = None):
    output = getStringIO()

    if config.mono and config.lang == "csharp":
        output.write("mono --debug '%s.exe' " % exe)
    elif config.lang == "ruby" and config.type == "client":
        #
        # If running with a binary distribution ensure rubygems
        # are enabled
        #
        if iceHome:
            output.write("ruby -rubygems")
        else:
            output.write("ruby")
        if interpreterOptions:
            output.write(" " + interpreterOptions)
        output.write(' "%s" ' % exe)
    elif config.lang == "java":
        output.write("%s -ea " % javaCmd)
        if isSolaris() and config.x64:
            output.write("-d64 ")
        if not config.ipv6:
            output.write("-Djava.net.preferIPv4Stack=true ")
        if javaLibraryPath:
            output.write("-Djava.library.path=" + javaLibraryPath)
        if interpreterOptions:
            output.write(" " + interpreterOptions)
        output.write(" " + exe + " ")
    elif config.lang == "python":
        output.write(sys.executable)
        if interpreterOptions:
            output.write(" " + interpreterOptions)
        output.write(' "%s" ' % exe)
    elif config.lang == "php" and config.type == "client":
        output.write(phpCmd + " -n %s" % phpFlags())
        if interpreterOptions:
            output.write(" " + interpreterOptions)
        output.write(" -f \""+ exe +"\" -- ")
    elif config.lang == "js":
        output.write(nodeCmd)
        if interpreterOptions:
            output.write(" " + interpreterOptions)
        output.write(' "%s" ' % exe)
    elif config.lang == "cpp" and config.valgrind:
        # --child-silent-after-fork=yes is required for the IceGrid/activator test where the node
        # forks a process with execv failing (invalid exe name).
        output.write("valgrind -q --child-silent-after-fork=yes --leak-check=full ")
        output.write('--suppressions="' + os.path.join(toplevel, "config", "valgrind.sup") + '" "' + exe + '" ')
    else:
        if exe.find(" ") != -1:
            output.write('"' + exe + '" ')
        else:
            output.write(exe + " ")

    if exe.find("IceUtil\\") != -1 or exe.find("IceUtil/") != -1:
        output.write(' ' + options)
    else:
        output.write(getCommandLineProperties(exe, config, cfgName) + ' ' + options)

    commandline = output.getvalue()
    output.close()

    return commandline

def directoryToPackage():
    """Determine the package name from the directory."""
    base = os.getcwd()
    after = []
    before = base
    lang = getDefaultMapping()
    while len(before) > 0:
        current = os.path.basename(before)
        before = os.path.dirname(before)
        if current == lang:
            break
        after.insert(0, current)
    else:
        raise RuntimeError("cannot find language dir")
    return ".".join(after)

def getDefaultServerFile(baseDir = os.getcwd()):
    lang = getDefaultMapping()
    if lang in ["cpp", "objective-c", "js", "ruby", "php"]:
        return getTestExecutable("server", baseDir)
    if lang in ["csharp"]:
        return "server"
    if lang == "python":
        return "Server.py"
    if lang == "java":
        pkg = directoryToPackage()
        if len(pkg) > 0:
            pkg = pkg + "."
        return pkg + "Server"
    raise RuntimeError("unknown language")

def getDefaultClientFile(lang = None, baseDir = ""):
    if lang is None:
        lang = getDefaultMapping()
    if lang in ["cpp", "objective-c"]:
        return getTestExecutable("client", baseDir)
    if lang == "ruby":
        return "Client.rb"
    if lang == "php":
        return "Client.php"
    if lang in ["csharp"]:
        return "client"
    if lang == "python":
        return "Client.py"
    if lang == "java":
        pkg = directoryToPackage()
        if len(pkg) > 0:
            pkg = pkg + "."
        return pkg + "Client"
    if lang == "js":
        return "run.js"
    raise RuntimeError("unknown language")

def getDefaultCollocatedFile():
    lang = getDefaultMapping()
    if lang in ["cpp", "objective-c"]:
        return getTestExecutable("collocated")
    if lang == "ruby":
        return "Collocated.rb"
    if lang == "php":
        return "Collocated.php"
    if lang in ["csharp"]:
        return "collocated"
    if lang == "python":
        return "Collocated.py"
    if lang == "java":
        return directoryToPackage() + ".Collocated"

import Expect

def _spawn(cmd, env=None, cwd=None, startReader=True, lang=None):
    # Start/Reset the watch dog thread
    global watchDog
    if watchDog is None:
        watchDog = WatchDog()
    else:
        watchDog.reset()

    if debug:
        sys.stdout.write("(%s) " % cmd)
        sys.stdout.flush()
    if printenv:
        dumpenv(env, lang)

    try:
        process = Expect.Expect(cmd, startReader=startReader, env=env, logfile=tracefile, cwd=cwd)
    except:
        print("Command failed:\n" + cmd)
        traceback.print_exc(file=sys.stderr)
        sys.exit(1)

    return process

def spawn(cmd, cwd=None):
    # Spawn given command with test environment.
    return _spawn(cmd, getTestEnv(getDefaultMapping(), os.getcwd()))

def spawnClient(cmd, env=None, cwd=None, echo=True, startReader=True, lang=None):
    client = _spawn(cmd, env, cwd, startReader=startReader, lang=lang)
    if echo:
        client.trace(clientTraceFilters)
    return client

def spawnServer(cmd, env=None, cwd=None, count=1, adapter=None, echo=True, lang=None, mx=False, timeout=60):
    server = _spawn(cmd, env, cwd, lang=lang)

    # Count + 1 if IceMX enabled
    if mx:
        count = count + 1

    if adapter:
        server.expect("%s ready\n" % adapter, timeout = timeout)
    else:
        while count > 0:
            server.expect("[^\n]+ ready\n", timeout = timeout)
            count = count - 1
    if echo:
        server.trace([re.compile("[^\n]+ ready")] + serverTraceFilters)
    return server

import subprocess
def runCommand(command):
    env = getTestEnv(getDefaultMapping(), os.getcwd())

    #
    # popen3 has problems dealing with white spaces in command line.
    #
    if isWin32():
        CREATE_NEW_PROCESS_GROUP = 512
        #
        # We set shell=True to make sure executables are correctly searched
        # in directories specified by the PATH environment variable.
        #
        p = subprocess.Popen(command, shell=True, bufsize=0, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE, creationflags = 512, env = env)
    else:
        p = subprocess.Popen(command, shell=True, bufsize=1024, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE, close_fds=True, env = env)

    return p;

def matchAppVerifierSuccess():
    return re.escape("\nApplication Verifier ") +  ".*\n" + \
           re.escape("Copyright (c) Microsoft Corporation. All rights reserved.") + \
           ".*\n\n$" #After all match to newlines at end.

def setAppVerifierSettings(targets, cwd=os.getcwd()):
    for exe in targets:
        if exe.endswith(".exe") == False:
            exe += ".exe"

        #First enable all appverifier tests
        cmd = "appverif -enable * -for " + exe
        verifier = _spawn(cmd, cwd=cwd)
        verifier.expect(matchAppVerifierSuccess(), -1)

        #Now disable all tests we are not intested in
        cmd = "appverif -disable LuaPriv PrintDriver PrintApi Networking HighVersionLie -for " + exe
        verifier = _spawn(cmd, cwd=cwd)
        verifier.expectall(["Application Verifier","",""], -1)

def appVerifierAfterTestEnd(targets, cwd=os.getcwd()):
    for exe in targets:
        if exe.endswith(".exe") == False:
            exe += ".exe"

        # Export appverifier logs to a xml file in cwd
        logName = cwd
        if logName is None:
            logName = os.path.dirname(exe)
        logName += "/" + os.path.basename(exe) + "_appverifier_log.xml"
        cmd = "appverif -export log -for " + exe + " -with To=" + logName
        verifier = _spawn(cmd, cwd=cwd)
        verifier.expect(matchAppVerifierSuccess(), -1)

        # Delete appverifier logs from registry
        cmd = "appverif -delete logs -for " + exe
        verifier = _spawn(cmd, cwd=cwd)
        verifier.expect(matchAppVerifierSuccess(), -1)

        # Delete appverifier settings
        cmd = "appverif -delete settings -for " + exe
        verifier = _spawn(cmd, cwd=cwd)
        verifier.expect(matchAppVerifierSuccess(), -1)

def getMirrorDir(base, mapping):
    """Get the mirror directory for the current test in the given mapping."""
    lang = getDefaultMapping()
    after = []
    before = base
    while len(before) > 0:
        current = os.path.basename(before)
        before = os.path.dirname(before)
        if current == lang:
            # Deal with Java's different directory structure
            if lang == "java":
                while len(before) > 0:
                    current = os.path.basename(before)
                    before = os.path.dirname(before)
                    if current == lang:
                        break
            break
        after.insert(0, current)
    else:
        raise RuntimeError("cannot find language dir")
    dir = os.path.join(before, mapping)
    # Deal with Java's different directory structure
    if mapping == "java":
        dir = os.path.join(dir, "test", "src", "main", "java")
    return os.path.join(dir, *after)

def getMappingDir(base, mapping, dirnames):
    """Get the directory for the given mapping."""
    lang = getDefaultMapping()
    after = []
    before = base
    while len(before) > 0:
        current = os.path.basename(before)
        before = os.path.dirname(before)
        if current == lang:
            break
        after.insert(0, current)
    else:
        raise RuntimeError("cannot find language dir")
    return os.path.join(before, mapping, *dirnames)

def getClientCrossTestDir(base):
    """Get the client directory from client-home for the given test."""
    global clientHome
    if not clientHome:
        return base

    lang = getDefaultMapping()
    after = []
    before = base
    while len(before) > 0:
        current = os.path.basename(before)
        before = os.path.dirname(before)
        if current == lang:
            break
        after.insert(0, current)
    else:
        raise RuntimeError("cannot find language dir")
    return os.path.join(clientHome, lang, *after)


def clientServerTest(cfgName = None, additionalServerOptions = "", additionalClientOptions = "",
                     server = None, client = None, serverenv = None, clientenv = None,
                     interpreterOptions = ""):
    lang = getDefaultMapping()
    testdir = os.getcwd()
    # Setup the server.
    if lang in ["ruby", "php", "js"]:
        serverdir = getMirrorDir(testdir, "cpp")
    else:
        serverdir = testdir

    if server is None:
        server = getDefaultServerFile(serverdir)
    elif lang in ["ruby", "php", "js"]:
        server = getTestExecutable(server, serverdir)

    if client is None:
        client = getDefaultClientFile()

    serverDesc = os.path.basename(server)
    clientDesc = os.path.basename(client)

    if lang != "java":
        server = os.path.join(serverdir, server)

    if serverenv is None:
        if lang in ["ruby", "php", "js"]:
            serverenv = getTestEnv("cpp", serverdir)
        else:
            serverenv = getTestEnv(lang, serverdir)

    global cross
    if len(cross) > 0:
        if lang == "js" and cross[0] != lang:
            print("** skipping js cross test ")
            return
    elif len(cross) == 0:
        cross.append(lang)

    global controller
    global clientHome
    for clientLang in cross:
        clientCfg = DriverConfig("client")
        if clientLang != lang:
            if clientDesc != os.path.basename(getDefaultClientFile()):
                print("** skipping cross test")
                return

            if clientHome:
                clientdir = getMirrorDir(getClientCrossTestDir(testdir), clientLang)
            else:
                clientdir = getMirrorDir(testdir, clientLang)

            client = getDefaultClientFile(clientLang, clientdir)
            clientDesc = os.path.basename(client)
            clientCfg.lang = clientLang

            if not os.path.exists(clientdir):
                print("** no matching test for %s" % clientLang)
                return
        else:
            if clientHome:
                clientdir = getClientCrossTestDir(testdir)
            else:
                clientdir = testdir
            if not os.path.exists(clientdir):
                print("** no matching test for %s" % clientLang)
                return

        if clientLang != "java":
            client = os.path.join(clientdir, client)

        if clientenv is None:
            clientenv = getTestEnv(clientLang, clientdir)

        clientExe = client
        serverExe = server

        if appverifier:
          setAppVerifierSettings([clientExe, serverExe])

        if not controller:
            sys.stdout.write("starting " + serverDesc + "... ")
            sys.stdout.flush()
            serverCfg = DriverConfig("server")
            if lang in ["ruby", "php", "js"]:
                serverCfg.lang = "cpp"
            server = getCommandLine(server, serverCfg, additionalServerOptions, interpreterOptions)
            serverProc = spawnServer(server, env = serverenv, lang=serverCfg.lang, mx=serverCfg.mx)
            print("ok")
            sys.stdout.flush()

        if not serverOnly:
            if clientLang == lang:
                sys.stdout.write("starting %s... " % clientDesc)
            else:
                sys.stdout.write("starting %s %s ... " % (clientLang, clientDesc))
            sys.stdout.flush()
            if not controller:
                cfgName = None
            client = getCommandLine(client, clientCfg, additionalClientOptions, interpreterOptions, cfgName)
            clientProc = spawnClient(client, env = clientenv, cwd = clientdir, startReader = False, lang=clientCfg.lang)
            print("ok")

            global watchDog
            clientProc.startReader(watchDog)
            clientProc.waitTestSuccess()

        if not controller:
            serverProc.waitTestSuccess()

        if appverifier:
            appVerifierAfterTestEnd([clientExe, serverExe])

def collocatedTest(additionalOptions = ""):
    if controller or serverOnly:
        print("** skipping collocated test")
        return
    lang = getDefaultMapping()
    if len(cross) > 0 and cross[0] != lang:
        print("** skipping cross test")
        return
    testdir = os.getcwd()

    collocated = getDefaultCollocatedFile()
    if lang != "java":
        collocated = os.path.join(testdir, collocated)

    exe = collocated
    if appverifier:
        setAppVerifierSettings([exe])

    env = getTestEnv(lang, testdir)

    sys.stdout.write("starting collocated... ")
    sys.stdout.flush()
    collocated = getCommandLine(collocated, DriverConfig("colloc"), additionalOptions)
    collocatedProc = spawnClient(collocated, env = env, startReader = False, lang=lang)
    print("ok")

    global watchDog
    collocatedProc.startReader(watchDog)
    collocatedProc.waitTestSuccess()
    if appverifier:
        appVerifierAfterTestEnd([exe])

def clientEchoTest(additionalServerOptions = "", additionalClientOptions = "",
                   server = None, client = None, serverenv = None, clientenv = None):

    lang = getDefaultMapping()
    testdir = os.getcwd()

    # Setup the server.
    if lang in ["ruby", "php", "js"]:
       serverdir = getMappingDir(testdir, "cpp", ["test", "Ice", "echo"])
    else:
       serverdir = testdir

    print("serverdir: " + serverdir)

    if server is None:
        server = getDefaultServerFile(serverdir)
    if client is None:
        client = getDefaultClientFile()
    serverDesc = server
    clientDesc = client

    if lang != "java":
        server = os.path.join(serverdir, server)

    if serverenv is None:
        if lang in ["ruby", "php", "js"]:
            serverenv = getTestEnv("cpp", serverdir)
        else:
            serverenv = getTestEnv(lang, serverdir)

    global cross
    if len(cross) == 0:
        cross.append(lang)

    global clientHome
    for clientLang in cross:
        clientCfg = DriverConfig("client")
        if clientLang != lang:
            if clientDesc != getDefaultClientFile():
                print("** skipping cross test")
                return

            clientCfg.lang = clientLang
            client = getDefaultClientFile(clientLang)
            if clientHome:
                clientdir = getMirrorDir(getClientCrossTestDir(testdir), clientLang)
            else:
                clientdir = getMirrorDir(testdir, clientLang)
            if not os.path.exists(clientdir):
                print("** no matching test for %s" % clientLang)
                return
        else:
            if clientHome:
                clientdir = getClientCrossTestDir(testdir)
            else:
                clientdir = testdir
            if not os.path.exists(clientdir):
                print("** no matching test for %s" % clientLang)
                return

        if clientLang != "java":
            client = os.path.join(clientdir, client)

        if clientenv is None:
            clientenv = getTestEnv(clientLang, clientdir)

        clientExe = client
        serverExe = server

        if appverifier:
            setAppVerifierSettings([clientExe, serverExe])

        sys.stdout.write("starting " + serverDesc + "... ")
        sys.stdout.flush()
        serverCfg = DriverConfig("server")
        if lang in ["ruby", "php", "js"]:
            serverCfg.lang = "cpp"
        server = getCommandLine(server, serverCfg, additionalServerOptions)
        serverProc = spawnServer(server, env = serverenv, lang=serverCfg.lang, mx=serverCfg.mx)
        print("ok")
        sys.stdout.flush()

        if not serverOnly:
            if clientLang == lang:
                sys.stdout.write("starting %s... " % clientDesc)
            else:
                sys.stdout.write("starting %s %s ... " % (clientLang, clientDesc))
            sys.stdout.flush()
            client = getCommandLine(client, clientCfg, additionalClientOptions)
            clientProc = spawnClient(client, env = clientenv, startReader = False, lang=clientCfg.lang)
            print("ok")
            clientProc.startReader()
            clientProc.waitTestSuccess()

        serverProc.waitTestSuccess()

        if appverifier:
            appVerifierAfterTestEnd([clientExe, serverExe])

def cleanDbDir(path):
    if os.path.exists(os.path.join(path, "__Freeze", "lock")):
        os.remove(os.path.join(path, "__Freeze", "lock"))
    if os.path.exists(os.path.join(path, "__Freeze")):
        os.rmdir(os.path.join(path, "__Freeze"))
    #
    # We include __Freeze in this list even though we just removed it - see ICE-5108.
    #
    ignore = [".gitignore", "DB_CONFIG", "__Freeze"]
    for filename in [ os.path.join(path, f) for f in os.listdir(path) if f not in ignore ]:
        os.remove(filename)

def startClient(exe, args = "", config=None, env=None, echo = True, startReader = True, clientConfig = False, iceOptions = None, iceProfile = None):
    if config is None:
        config = DriverConfig("client")
    if env is None:
        env = getTestEnv(getDefaultMapping(), os.getcwd())

    interpreterOptions = ""
    if config.lang == "php":
        interpreterOptions = phpProfileSetup(clientConfig, iceOptions, iceProfile)

    cmd = getCommandLine(exe, config, args, interpreterOptions)

    return spawnClient(cmd, env = env, echo = echo, startReader = startReader, lang=config.lang)

def startServer(exe, args = "", config = None, env = None, adapter = None, count = 1, echo = True, timeout = 60,
                interpreterOptions = ""):
    if config is None:
        config = DriverConfig("server")
    if env is None:
        env = getTestEnv(getDefaultMapping(), os.getcwd())
    cmd = getCommandLine(exe, config, args, interpreterOptions)
    return spawnServer(cmd, env = env, adapter = adapter, count = count, echo = echo, lang = config.lang,
                       mx = config.mx, timeout = timeout)

def startColloc(exe, args, config=None, env=None, interpreterOptions = ""):
    exe = quoteArgument(exe)
    if config is None:
        config = DriverConfig("colloc")
    if env is None:
        env = getTestEnv(config.lang, testdir)
    cmd = getCommandLine(exe, config, args, interpreterOptions)
    return spawnClient(cmd, env = env, lang=config.lang)

def simpleTest(exe = None, options = "", interpreterOptions = ""):
    if exe is None:
        exe = getDefaultClientFile()
    if appverifier:
        setAppVerifierSettings([quoteArgument(exe)])

    testdir = os.getcwd()

    config = DriverConfig("client")
    env = getTestEnv(config.lang, testdir)

    sys.stdout.write("starting client... ")
    sys.stdout.flush()
    command = getCommandLine(exe, config, options, interpreterOptions)
    client = spawnClient(command, startReader = False, env = env, lang = config.lang)
    print("ok")
    client.startReader()
    client.waitTestSuccess()

    if appverifier:
        appVerifierAfterTestEnd([exe])

def createConfig(path, lines, enc=None):
    if sys.version_info[0] > 2 and enc:
        config = open(path, "w", encoding=enc)
    else:
        config = open(path, "wb")
    for l in lines:
        config.write("%s\n" % l)
    config.close()

def getCppBinDir(lang = None):
    binDir = os.path.join(getIceDir("cpp"), "bin")

    if isWin32():
        binDir = os.path.join(binDir, "x64" if x64 else "Win32", "Debug" if buildMode == "debug" else "Release")

    if isMINGW() and x64:
        binDir = os.path.join(binDir, "x64")
    if iceHome:
        if lang == None:
            lang = getDefaultMapping()
        if isVC110() and lang != "python":
            binDir = os.path.join(binDir, "vc110")
        elif isVC140():
            binDir = os.path.join(binDir, "vc140")
        if x64:
            if isSolaris():
                if isSparc():
                    binDir = os.path.join(binDir, "sparcv9")
                else:
                    binDir = os.path.join(binDir, "amd64")
            elif isWin32() and lang != "php":
                binDir = os.path.join(binDir, "x64")

    return binDir

def getSliceTranslator(lang = "cpp"):
    if iceHome:
        return os.path.join(iceHome, "bin", "slice2%s" % lang)
    else:
        return os.path.join(getCppBinDir(), ("slice2%s" % lang))

def getCppLibDir(lang = None):
    if isWin32():
        return getCppBinDir(lang)
    else:
        libDir = os.path.join(getIceDir("cpp"), "lib")
        if isUbuntu() or isDebian():
            libDir = os.path.join(libDir, dpkgHostMultiArch())
        elif x64:
            if isSolaris():
                if isSparc():
                    libDir = os.path.join(libDir, "64")
                else:
                    libDir = os.path.join(libDir, "amd64")
            if isLinux():
                libDir = libDir + "64"
        return libDir
    return None

def getServiceDir():
    global serviceDir
    if serviceDir is None:
        if iceHome:
            serviceDir = os.path.join(iceHome, "bin")
        else:
            serviceDir = "C:\\Program Files\ZeroC\Ice-" + iceVersion + "\\bin"
    return serviceDir

def getTestEnv(lang, testdir):
    global compact
    env = os.environ.copy()

    #
    # Jar files from the source of binary distribution
    #
    iceJARs = ["ice", "glacier2", "freeze", "icebox", "icestorm", "icegrid", "icepatch2", "icediscovery",
               "icelocatordiscovery"]
    jarSuffix = "-" + getIceJsonVersion() + ".jar"

    # First sanitize the environment.
    env["CLASSPATH"] = sanitize(os.getenv("CLASSPATH", ""))

    for d in additionalBinDirectories:
        addLdPath(d, env)

    #
    # Add cpp/test/Common output directory to the env
    #
    if lang == "cpp":
        baseDir = os.path.join(toplevel, "cpp", "test", "Common")
        addLdPath(os.path.join(baseDir, getTestDirectory("testcommon", baseDir)), env)

    # Make sure bzip2 can be found by x86 C# builds on x64 platforms
    if lang == "csharp" and not x64:
        addPathToEnv("PATH", os.path.join(getCppBinDir("csharp"), "x64"), env)

    # Add test directory to env
    if lang == "cpp":
        addLdPath(os.path.join(testdir), env)
    elif lang == "java":
        addClasspath(os.path.join(testToplevel, "java", "lib", "test.jar"), env)
    elif lang == "js":
        addPathToEnv("NODE_PATH", os.path.join(testdir), env)

        # NodeJS is always installed locally even when testing against a binary installation
        if os.environ.get("USE_BIN_DIST", "no") != "yes":
            addPathToEnv("NODE_PATH", os.path.join(getIceDir("js", testdir), "src"), env)

    if isWin32():
        if lang == "java" and javaLibraryPath:
            addPathToEnv("PATH", javaLibraryPath, env)

        if lang == "csharp" and not iceHome:
            pkgdir = os.path.join(getIceDir("cpp"), "msbuild", "packages")
            pkgsubdir = os.path.join("build", "native", "bin", "x64", "Release")
            if isVC120():
                addPathToEnv("PATH", os.path.join(pkgdir, "bzip2.v120.1.0.6.4", pkgsubdir), env)
            else:
                addPathToEnv("PATH", os.path.join(pkgdir, "bzip2.v140.1.0.6.4", pkgsubdir), env)

    #
    # If Ice is installed on the system, set the CLASSPATH for Java and
    # NODE_PATH for JavaScript
    #
    if iceHome in ["/usr", "/usr/local"]:
        if lang == "java":
            javaDir = os.path.join(iceHome, "share", "java")
            for jar in iceJARs:
                addClasspath(os.path.join(javaDir, jar + jarSuffix), env)
        return env # That's it, we're done!

    #
    # For Win32 we always need to add the bin dir to the PATH. For
    # others we just need to add it for scripting languages that use
    # C++ extensions (py, ruby, php)
    #
    if isWin32():
        if lang == "java":
            addLdPath(os.path.join(getIceDir("cpp"), "bin", "x64" if x64 else ""), env) # Add bin for db53_vc100.dll
        addLdPath(getCppLibDir(lang), env)
    elif isAIX():
        addLdPath(getCppLibDir(lang), env)
    elif lang in ["python", "ruby", "php", "js", "objective-c"]:
        # C++ binaries use rpath $ORIGIN or similar to find the Ice libraries
        addLdPath(getCppLibDir(lang), env)

    if lang == "java":
        # The Ice.jar and Freeze.jar comes from the installation
        # directory or the toplevel dir.
        javaDir = os.path.join(getIceDir("java", testdir), "lib")
        for jar in iceJARs:
            addClasspath(os.path.join(javaDir, jar + jarSuffix), env)

    #
    # On Windows, C# assemblies are found thanks to the .exe.config files.
    #
    if lang == "csharp":
        if compact:
            addPathToEnv("DEVPATH", os.path.join(getIceDir("csharp", testdir), "Assemblies", "cf"), env)
        elif isWin32():
            addPathToEnv("DEVPATH", os.path.join(getIceDir("csharp", testdir), "Assemblies"), env)
        else:
            addPathToEnv("MONO_PATH", os.path.join(getIceDir("csharp", testdir), "Assemblies"), env)

    if os.environ.get("USE_BIN_DIST", "no") != "yes":
        #
        # On Windows x64, set PYTHONPATH to python/x64.
        #
        if lang == "python":
            pythonDir = os.path.join(getIceDir("python", testdir), "python")
            if isWin32():
                addPathToEnv("PYTHONPATH", pythonDir, env)
                pythonDir = os.path.join(pythonDir, "Win32" if x86 else "x64", "Debug" if buildMode == "debug" else "Release")
            addPathToEnv("PYTHONPATH", pythonDir, env)

        #
        # If testing with source dist we need to set RUBYLIB
        #
        if lang == "ruby":
            addPathToEnv("RUBYLIB", os.path.join(getIceDir("ruby", testdir), "ruby"), env)

    return env;

def getTestName():
    lang = getDefaultMapping()
    here = os.getcwd().split(os.sep)
    here.reverse()
    for i in range(0, len(here)):
        if here[i] == lang:
            break
    else:
        raise RuntimeError("cannot find language dir")
    here = here[:i-1]
    here.reverse()
    # The crossTests list is in UNIX format.
    return os.path.join(*here).replace(os.sep, '/')

def stopWatchDog():
    global watchDog
    watchDog.stop()
    watchDog.join()
    watchDog = None

def joindog(dog):
    dog.stop()
    dog.join()

class WatchDog(threading.Thread):
    def __init__(self):
        self._resetFlag = False
        self._stopFlag = False
        self._cv = threading.Condition()
        threading.Thread.__init__(self)

        # The thread is marked as a daemon thread. The atexit handler
        # joins with the thread on exit. If the thread is not daemon,
        # the atexit handler will not be called.
        self.setDaemon(True)
        self.start()
        atexit.register(joindog, self)

    def run(self):
        try:
            self._cv.acquire()
            while True:
                self._cv.wait(240)
                if self._stopFlag:
                    break
                if self._resetFlag:
                    self._resetFlag = False
                else:
                    print("\a*** %s Warning: Test has been inactive for 4 minutes and may be hung" %
                          time.strftime("%x %X"))
            self._cv.release()
        except:
            #
            # Exceptions can be raised if the thread is still alive when the interpreter is
            # in the process of shutting down. For example, Python resets module objects to None,
            # so code such as "time.strftime()" can fail with AttributeError.
            #
            pass

    def reset(self):
        self._cv.acquire()
        self._resetFlag = True
        self._cv.notify()
        self._cv.release()

    def stop(self):
        self._cv.acquire()
        self._stopFlag = True
        self._cv.notify()
        self._cv.release()

def processCmdLine():
    def usage():
        print("usage: " + sys.argv[0] + """
          --debug              Display debugging information on each test.
          --trace=<file>       Display tracing.
          --protocol=<prot>    Run with the given protocol (tcp|ssl|ws|wss|bt).
          --compress           Run the tests with protocol compression.
          --valgrind           Run the tests with valgrind.
          --appverifier        Run the tests with appverifier.
          --host=host          Set --Ice.Default.Host=<host>.
          --serialize          Run with connection serialization.
          --ipv6               Use IPv6 addresses.
          --socks              Use SOCKS proxy running on localhost.
          --ice-home=<path>    Use the binary distribution from the given path.
          --mode=debug|release Run the tests with debug or release mode builds (win32 only)."
          --x86                Binary distribution is 32-bit.
          --x64                Binary distribution is 64-bit.
          --c++11              Binary distribution is c++11.
          --static             Binary distribution is static.
          --env                Print important environment variables.
          --cross=lang         Run cross language test.
          --client-home=<dir>  Run cross test clients from the given Ice source distribution.
          --service-dir=<dir>  Where to locate services for builds without service support.
          --compact            Ice for .NET uses the Compact Framework.
          --winrt              Run server with configuration suited for WinRT client.
          --server             Run only the server.
          --mx                 Enable IceMX when running the tests.
          --controller=<host>  Use the test controller on the specified host.
          --arg=<property>     Append the given argument.
        """)
        sys.exit(2)

    try:
        opts, args = getopt.getopt(
            sys.argv[1:], "", ["debug", "trace=", "protocol=", "compress", "valgrind", "host=", "serialize", "ipv6", \
                               "socks", "ice-home=", "mode=", "x86", "x64", "cross=", "client-home=", "env", \
                               "service-dir=", "appverifier", "arg=", \
                               "compact", "winrt", "server", "mx", "c++11", "static", "controller=", "configName="])
    except getopt.GetoptError:
        usage()

    if args:
        usage()

    global serverOnly
    global winrt
    global buildMode
    for o, a in opts:
        if o == "--ice-home":
            global iceHome
            iceHome = a
        elif o == "--mode":
            if a not in ( "debug", "release"):
                usage()
            buildMode = a
        elif o == "--cross":
            global cross
            cross.append(a)
            crossLang = ["cpp", "csharp", "java", "js", "php", "python", "ruby", "objective-c"]
            if not a in crossLang:
                print("cross must be one of %s" % ', '.join(crossLang))
                sys.exit(1)
            if getTestName() not in crossTests:
                print("*** This test does not support cross language testing")
                sys.exit(0)
        elif o == "--client-home":
            global clientHome
            clientHome = a
        elif o == "--x64":
            global x64
            x64 = True
        elif o == "--x86":
            global x86
            x86 = True
            x64 = False
        elif o == "--c++11":
            global cpp11
            cpp11 = True
        elif o == "--static":
            global static
            static = True
        elif o == "--compress":
            global compress
            compress = True
        elif o == "--serialize":
            global serialize
            serialize = True
        elif o == "--host":
            global host
            host = a
        elif o == "--valgrind":
            global valgrind
            valgrind = True
        elif o == "--appverifier":
            if not isWin32() or getDefaultMapping() != "cpp":
                print("--appverifier option is only supported for Win32 c++ tests.")
                sys.exit(1)
            global appverifier
            appverifier = True
        elif o == "--ipv6":
            global ipv6
            ipv6 = True
        elif o == "--socks":
            global socksProxy
            socksProxy = True
        if o == "--trace":
            global tracefile
            if a == "stdout":
                tracefile = sys.stdout
            else:
                tracefile = open(a, "w")
        elif o == "--debug":
            global debug
            debug = True
        elif o == "--env":
            global printenv
            printenv = True
        elif o == "--protocol":
            if a not in ("bt", "bts", "ws", "wss", "ssl", "tcp"):
                usage()
            # ssl protocol isn't directly supported with mono.
            if mono and getDefaultMapping() == "csharp" and (a == "ssl" or a == "wss"):
                print("SSL is not supported with mono")
                sys.exit(1)
            if a in ["bt", "bts"] and not isLinux():
                print("Bluetooth is only supported on Linux")
                sys.exit(1)
            global protocol
            protocol = a
        elif o == "--service-dir":
            global serviceDir
            serviceDir = a
        elif o == "--compact":
            global compact
            compact = True
        elif o == "--winrt":
            winrt = True
            serverOnly = True
        elif o == "--server":
            serverOnly = True
        elif o == '--arg':
            extraArgs.append(a)
        elif o == "--mx":
            global mx
            mx = True
        elif o == "--controller":
            global controller
            controller = a
        elif o == "--configName":
            global configName
            configName = a
        if protocol in ["ssl", "wss"] and not serverOnly and getDefaultMapping() == "js":
            print("SSL is not supported with Node.js")
            sys.exit(1)

    if len(args) > 0:
        usage()

    if isWin32() and not buildMode:
        print("Error: please define --mode=debug or --mode=release")
        sys.exit(1)

    if not os.environ.get("TESTCONTROLLER"):
        if iceHome:
            sys.stdout.write("*** using Ice installation from " + iceHome + " ")
        else:
            sys.stdout.write("*** using Ice source dist ")
        if armv7l:
            sys.stdout.write("(ARMv7)")
        elif x64:
            sys.stdout.write("(64bit) ")
        else:
            sys.stdout.write("(32bit) ")
        sys.stdout.write("\n")

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

            # Deal with Java's different directory structure
            if i.find(os.path.join("java","test")) != -1:
                dir = os.path.join(testToplevel, "java", "test", "src", "main", i)
            else:
                dir = os.path.join(testToplevel, i)
            dir = os.path.normpath(dir)

            sys.stdout.write("\n")
            if num > 0:
                sys.stdout.write("[" + str(num) + "] ")
            if script:
                prefix = "echo \""
                suffix = "\""
            else:
                prefix = ""
                suffix = ""

            print("%s*** running tests %d/%d in %s%s" % (prefix, index, total, dir, suffix))
            sys.stdout.write("%s*** configuration: " % prefix)
            if len(args.strip()) == 0:
                sys.stdout.write("Default ")
            else:
                sys.stdout.write(args.strip() + " ")
            print(suffix)

            if args.find("cross") != -1:
                test = os.path.join(*i.split(os.sep)[2:])
                # The crossTests list is in UNIX format.
                test = test.replace(os.sep, '/')
                if not test in crossTests:
                    print("%s*** test does not support cross testing%s" % (prefix, suffix))
                    continue

            #
            # Skip tests not supported with IPv6 if necessary
            #
            if args.find("ipv6") != -1 and "noipv6" in config:
                print("%s*** test not supported with IPv6%s" % (prefix, suffix))
                continue

            if args.find("compress") != -1 and "nocompress" in config:
                print("%s*** test not supported with compression%s" % (prefix, suffix))
                continue

            if args.find("socks") != -1 and "nosocks" in config:
                print("%s*** test not supported with SOCKS%s" % (prefix, suffix))
                continue

            if args.find("compact") != -1 and \
                         ("nocompact" in config or \
                          args.find("ssl") != -1 or \
                          args.find("compress") != -1):
                print("%s*** test not supported with Compact Framework%s" % (prefix, suffix))
                continue

            if args.find("mx") != -1 and "nomx" in config:
                print("%s*** test not supported with IceMX enabled%s" % (prefix, suffix))
                continue

            if args.find("compact") == -1 and "compact" in config:
                print("%s*** test requires Compact Framework%s" % (prefix, suffix))
                continue

            if isVista() and "novista" in config:
                print("%s*** test not supported under Vista%s" % (prefix, suffix))
                continue

            if isDarwin() and "nodarwin" in config:
                print("%s*** test not supported under Darwin%s" % (prefix, suffix))
                continue

            if isYocto() and "noyocto" in config:
                print("%s*** test not supported under Yocto%s" % (prefix, suffix))
                continue

            if not isWin32() and "win32only" in config:
                print("%s*** test only supported under Win32%s" % (prefix, suffix))
                continue

            if isVC100() and "novc100" in config:
                print("%s*** test not supported with VC++ 10.0%s" % (prefix, suffix))
                continue

            if cpp11 and "noc++11" in config:
                print("%s*** test not supported with C++11%s" % (prefix, suffix))
                continue

            if static and "nostatic" in config:
                print("%s*** test not supported with static%s" % (prefix, suffix))
                continue

            if x86 and iceHome and "nomultiarch" in config:
                print("%s*** test not supported with x86 in multiarch%s" % (prefix, suffix))
                continue

            if isMINGW() and "nomingw" in config:
                print("%s*** test not supported with MINGW%s" % (prefix, suffix))
                continue

            if isWin32() and "nowin32" in config:
                print("%s*** test not supported with Win32%s" % (prefix, suffix))
                continue

            # If this is mono and we're running ssl protocol tests
            # then skip. This occurs when using --all.
            if mono and ("nomono" in config or (i.find(os.path.join("csharp", "test")) != -1 and
                                                (args.find("ssl") != -1 or args.find("wss") != -1))):
                print("%s*** test not supported with mono%s" % (prefix, suffix))
                continue

            #
            # Skip configurations not supported by node
            #
            if (i.find(os.path.join("js","test")) != -1 and
                                  ((not serverOnly and (args.find("ssl") != -1 or
                                                        args.find("wss") != -1 or
                                                        args.find("ws") != -1)) or
                                   args.find("compress") != -1 or
                                   args.find("mx") != -1)):
                print("%s*** test not supported with Node.js%s" % (prefix, suffix))
                continue

            if (args.find("ssl") != -1 or args.find("wss") != -1) and ("nossl" in config):
                print("%s*** test not supported with IceSSL%s" % (prefix, suffix))
                continue

            if (args.find("bt") != -1 or args.find("bts") != -1) and ("bt" not in config):
                print("%s*** test not supported with Bluetooth%s" % (prefix, suffix))
                continue

            if (args.find("ws") != -1 or args.find("wss") != -1) and ("nows" in config):
                print("%s*** test not supported with IceWS%s" % (prefix, suffix))
                continue

            # If this is java and we're running ipv6 under windows then skip.
            if isWin32() and i.find(os.path.join("java","test")) != -1 and args.find("ipv6") != -1 and \
                    getJdkVersion().find("java version \"1.6") != -1:
                print("%s*** test not supported under windows%s" % (prefix, suffix))
                continue

            # Skip tests not supported by valgrind
            if args.find("valgrind") != -1 and ("novalgrind" in config or args.find("ssl") != -1 or
                                                args.find("wss") != -1):
                print("%s*** test not supported with valgrind%s" % (prefix, suffix))
                continue

            # Skip tests not supported by appverifier
            if args.find("appverifier") != -1 and ("noappverifier" in config):
                print("%s*** test not supported with appverifier%s" % (prefix, suffix))
                continue

            # Skip tests that no not exist for this language mapping
            if not os.path.exists(dir):
                print("%s*** test does not exist for this language mapping%s" % (prefix, suffix))
                continue

            if script:
                print("echo \"*** test started: `date`\"")
                print("cd %s" % dir)
            else:
                print("*** test started: " + time.strftime("%x %X"))
                sys.stdout.flush()
                os.chdir(dir)

            global keepGoing
            if script:
                print("if ! %s %s %s; then" % (sys.executable, os.path.join(dir, "run.py"), args))
                print("  echo 'test in %s failed'" % os.path.abspath(dir))
                if not keepGoing:
                    print("  exit 1")
                print("fi")
            else:
                status = os.system(sys.executable + " " +  quoteArgument(os.path.join(dir, "run.py")) + " " + args)
                if status:
                    status = status if isWin32() else (status >> 8)
                    if(num > 0):
                        sys.stdout.write("[" + str(num) + "] ")
                    message = "test in " + os.path.abspath(dir) + " failed with exit status", status,
                    print(message)
                    if not keepGoing:
                        sys.exit(status)
                    else:
                        print(" ** Error logged and will be displayed again when suite is completed **")
                        global testErrors
                        testErrors.append(message)

class ClientServerTest:
    def __init__(self, cfgName, message, additionalServerOptions, additionalClientOptions,
                 server, client, serverenv, clientenv, interpreterOptions, localOnly):
        if cfgName is None:
            cfgName = "default"
        self.cfgName = cfgName
        self.message = message
        self.additionalServerOptions = additionalServerOptions
        self.additionalClientOptions = additionalClientOptions
        self.server = server
        self.client = client
        self.serverenv = serverenv
        self.clientenv = clientenv
        self.interpreterOptions = interpreterOptions
        self.localOnly = localOnly

    def getConfigName(self):
        return self.cfgName

    def getMessage(self):
        return self.message

    def isLocalOnly(self):
        return self.localOnly

    def getDefaultMessage(self):
        return "Running test with regular server."

    def run(self):
        clientServerTest(self.cfgName, self.additionalServerOptions, self.additionalClientOptions, self.server,
                         self.client, self.serverenv, self.clientenv, self.interpreterOptions)

def queueClientServerTest(configName = None, message = None, additionalServerOptions = "",
                          additionalClientOptions = "", server = None, client = None, serverenv = None,
                          clientenv = None, interpreterOptions = "", localOnly = False):
    global queuedTests
    queuedTests.append(ClientServerTest(configName, message, additionalServerOptions, additionalClientOptions, server,
                                        client, serverenv, clientenv, interpreterOptions, localOnly))

class CollocatedTest:
    def __init__(self, message = None, additionalOptions = ""):
        self.message = message
        self.additionalOptions = additionalOptions

    def getMessage(self):
        return self.message

    def isLocalOnly(self):
        return True

    def getDefaultMessage(self):
        return "Running test with collocated server."

    def run(self):
        collocatedTest(self.additionalOptions)

def queueCollocatedTest(message = None, additionalOptions = ""):
    global queuedTests
    queuedTests.append(CollocatedTest(message, additionalOptions))

def runQueuedTests():
    global queuedTests
    global serverOnly
    global controller
    global configName

    if serverOnly:
        name = configName
        if not name:
            name = "default"

        for t in queuedTests:
            if isinstance(t, ClientServerTest) and t.getConfigName() == name:
                t.run()
                return

        print("no queued test found matching configuration name `" + name + "'")
        sys.exit(1)

    tests = []
    for t in queuedTests:
        if controller and t.isLocalOnly():
            continue
        tests.append(t)

    for t in tests:
        msg = t.getMessage()
        if msg:
            print(msg)
        elif len(tests) > 1:
            print(t.getDefaultMessage())
        t.run()

if "ICE_CONFIG" in os.environ:
    os.unsetenv("ICE_CONFIG")

import inspect
frame = inspect.currentframe()
# Move to the top-most frame in the callback.
while frame.f_back is not None:
    frame = frame.f_back
if os.path.split(frame.f_code.co_filename)[1] == "run.py":
    # If we're not in the test directory, chdir to the correct
    # location.
    if not os.path.isabs(sys.argv[0]):
        d = os.path.join(os.getcwd(), sys.argv[0])
    else:
        d = sys.argv[0]
    d = os.path.split(d)[0]
    if os.path.normpath(d) != os.getcwd():
        os.chdir(d)
    processCmdLine()

def getTestDirectory(name, baseDir = os.getcwd()):
    if isWin32():
        buildDir = "msbuild"
        platform = "x64" if x64 else "Win32"
        if cpp11:
            configuration = "Cpp11-Debug" if buildMode == "debug" else "Cpp11-Release"
        else:
            configuration = "Debug" if buildMode == "debug" else "Release"
    else:
        buildDir = "build"
        if isDarwin():
            platform = "macosx"
        elif isUbuntu() or isDebian():
            platform = "x86_64-linux-gnu" if x64 else "i386-linux-gnu"
        elif isAIX():
            platform = "ppc64" if x64 else "ppc"
        else:
            platform = "x64" if x64 else "x86"
        configuration = ("cpp11-" if cpp11 else "") + ("static" if static else "shared")

    if os.path.isdir(os.path.join(baseDir, buildDir, name)):
        return os.path.join(buildDir, name, platform, configuration)
    else:
        return os.path.join(buildDir, platform, configuration)

def getTestExecutable(name, baseDir = os.getcwd()):
    return os.path.join(getTestDirectory(name, baseDir), name)

#
# Used for tests which have Ice as a submodule
#
def setTestToplevel(path):
    global testToplevel
    testToplevel = path

def setJavaLibraryPath(path):
    global javaLibraryPath
    javaLibraryPath = path
