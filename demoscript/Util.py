# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os
import sys
import getopt
import re
import os
import signal
import time

# Locate the top level directory of the demo dist (or the top of the
# source tree for a source dist).
path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "demoscript")) ]
toplevel = path[0]
if os.path.isdir(os.path.join(toplevel, "cpp")):
    sourcedist = True
else:
    sourcedist = False

from scripts import Expect

keepGoing = False
iceHome = None
x64 = False
preferIPv4 = False
serviceDir = None
demoErrors = []

#
# Default value of --Ice.Default.Host
#
host = "127.0.0.1"

#
# Echo the commands.
#
debug = False


origenv = {}
def dumpenv():
    print "the following environment variables have been set:"
    for k, v in origenv.iteritems():
        added = os.environ[k][:len(os.environ[k])-len(v)]
	if len(v) > 0:
	    if isWin32():
		print "%s=%s%%%s%%" % (k, added, k)
	    else:
		print "%s=%s$%s" % (k, added, k)
	else:
		print "%s=%s" % (k, added)

def addenv(var, val):
    global origenv
    if not var in origenv:
        origenv[var] = os.environ.get(var, "")
    if var in os.environ:
        os.environ[var] = "%s%s%s" % (val, os.pathsep, os.environ.get(var, ""))
    else:
        os.environ[var] = val

def configurePaths():

    if iceHome:
        print "[ using Ice installation from " + iceHome,
        if x64:
            print "(64bit)",
        print "]"

    #
    # If Ice is installed from RPMs, just set the CLASSPATH for Java.
    #
    if iceHome == "/usr":
        javaDir = os.path.join("/", "usr", "share", "java")
        addenv("CLASSPATH", os.path.join(javaDir, "Ice.jar"))
        addenv("CLASSPATH", os.path.join(javaDir, "Freeze.jar"))
        addenv("CLASSPATH", "classes")
        return # That's it, we're done!

    binDir = os.path.join(getIceDir("cpp"), "bin")        
    shlibVar = None
    libDir = None
    if not isWin32():
        libDir = os.path.join(getIceDir("cpp"), "lib")

    # 64-bits binaries are located in a subdirectory with binary
    # distributions.
    if iceHome and x64: 
        if isWin32():
            binDir = os.path.join(binDir, "x64")
        elif isSolaris():
            if isSparc():
                libDir = os.path.join(libDir, "sparcv9")
                binDir = os.path.join(binDir, "sparcv9")
            else:
                libDir = os.path.join(libDir, "amd64")
                binDir = os.path.join(binDir, "amd64")
        else:
            libDir = libDir + "64"
            binDir = binDir + "64"

    # Only add the lib directory to the shared library path if we're
     # not using the embedded location.
    if libDir and iceHome != "/opt/Ice-3.3":
        addLdPath(libDir)

    if not iceHome:
        addenv("PATH", os.path.join(getIceDir("cs"), "bin"))
    addenv("PATH", binDir)

    javaDir = getIceDir("java")

    addenv("CLASSPATH", os.path.join(javaDir, "lib", "Ice.jar"))
    addenv("CLASSPATH", os.path.join(javaDir, "lib", "Freeze.jar"))
    if not iceHome:
        addenv("CLASSPATH", os.path.join(javaDir, "lib"))
    addenv("CLASSPATH", os.path.join("classes"))

    # 
    # On Windows, C# assemblies are found thanks to the .exe.config files.
    #
    if not isWin32():
        addenv("MONO_PATH", os.path.join(getIceDir("cs"), "bin"))

    #
    # On Windows x64, set PYTHONPATH to python/x64.
    #
    if isWin32() and x64:
        addenv("PYTHONPATH", os.path.join(getIceDir("py"), "python", "x64"))
    else:
        addenv("PYTHONPATH", os.path.join(getIceDir("py"), "python"))
    addenv("RUBYLIB", os.path.join(getIceDir("rb"), "ruby"))

# Mapping to the associated subdirectory.
mappingDirs = {
    "cpp" : ( os.path.join("cpp", "demo"), "demo"),
    "java" : ( os.path.join("java", "demo"), "demoj"),
    "cs" : ( os.path.join("cs", "demo"), "democs"),
    "vb" : ( os.path.join("vb", "demo"), "demovb"),
    "py" : ( os.path.join("py", "demo"), "demopy"),
    "rb" : ( os.path.join("rb", "demo"), "demorb"),
    "php" : ( os.path.join("php", "demo"), "demophp"),
}

def getMappingDir(suffix, mapping):
    """Get the directory containing the demos for the given mapping."""
    # In the source tree
    if sourcedist:
        return mappingDirs[mapping][0]
    else:
        return mappingDirs[mapping][1]

def getMirrorDir(mapping = None):
    """Get the mirror directory for the current demo in the given mapping."""
    here = os.path.abspath(os.getcwd())

    # Split off the front portion portion
    pref = here[:len(toplevel)]
    assert pref == toplevel
    post = here[len(toplevel)+1:]

    # In the source tree
    if sourcedist:
        scriptPath = os.path.join(post.split(os.sep)[2:])
        mappingDir = mappingDirs[mapping][0]
    else:
        scriptPath = os.path.join(post.split(os.sep)[1:])
        mappingDir = mappingDirs[mapping][1]
    return os.path.join(pref, mappingDir, *scriptPath)

def getIceDir(subdir = None):
    """Get the top level directory of the ice distribution.  If ICE_HOME
       is set we're running the test against a binary
       distribution. Otherwise, we're running the test against a
       source distribution."""
    global iceHome
    if iceHome:
        return iceHome
    elif subdir:
        return os.path.join(toplevel, subdir)
    else:
        return toplevel

def isWin32():
    return sys.platform == "win32"

def isSolaris():
    return sys.platform == "sunos5"

def isSparc():
    p = os.popen("uname -m")
    l = p.readline().strip()
    if l == "sun4u":
        return True
    else:
        return False

def isAIX():
   return sys.platform in ['aix4', 'aix5']

def isDarwin():
   return sys.platform == "darwin"

def isLinux():
    return sys.platform.startswith("linux")

def isMono():
    return not isWin32()

def isSolaris():
    return sys.platform == "sunos5"

def isNoServices():
    if not isWin32():
        return false
    compiler = ""
    if os.environ.get("CPP_COMPILER", "") != "":
        compiler = os.environ["CPP_COMPILER"]
    else:
        config = open(os.path.join(toplevel, "cpp", "config", "Make.rules.mak"), "r")
        compiler = re.search("CPP_COMPILER[\t\s]*= ([A-Z0-9]*)", config.read()).group(1)
    return compiler == "BCC2010" or compiler == "VC60"

def getMapping():
    """Determine the current mapping based on the cwd."""
    here = os.path.abspath(os.getcwd())
    assert here[:len(toplevel)] == toplevel
    dir = here[len(toplevel)+1:].split(os.sep)[0]

    if sourcedist:
        mapping = { "cpp": "cpp", "cs": "cs", "java": "java", "php": "php", "py": "py", "rb": "rb", "vb": "vb" }
    else:
        mapping = { "demo": "cpp", "democs": "cs", "demoj": "java", "demophp": "php", "demopy": "py",
                    "demorb": "rb", "demovb": "vb" }
    return mapping[dir]

def runDemos(start, args, demos, num = 0, script = False, root = False):
    global demoErrors
    global keepGoing

    total = len(demos)

    #
    # Run each of the demos.
    #
    index = 0
    for i in demos:
        index = index + 1
        if index < start:
            continue

        i = os.path.normpath(i)
        if root:
            dir = os.path.join(toplevel, i)
        else:
            dir = os.path.join(toplevel, getMappingDir(toplevel, getMapping()), i)

        if script:
            prefix = "echo \""
            suffix = "\""
        else:
            prefix = ""
            suffix = ""

        print
        if(num > 0):
            print "[" + str(num) + "]",
        print "%s*** running demo %d/%d in %s%s" % (prefix, index, total, dir, suffix)
        print "%s*** configuration:" % prefix,
        if len(args.strip()) == 0:
            print "Default",
        else:
            print args.strip(),
        print suffix

        if script:
            print "echo \"*** demo started: `date`\""
            print "cd %s" % dir
        else:
            print "*** demo started:", time.strftime("%x %X")
            sys.stdout.flush()
            os.chdir(dir)

        if script:
            print "if ! python %s %s; then" % (os.path.join(dir, "expect.py"), args)
            print "  echo 'demo in %s failed'" % os.path.abspath(dir)
            if not keepGoing:
                print "  exit 1"
            print "fi"
        else:
            status = os.system("python " + os.path.join(dir, "expect.py " + args))

            if status:
                if(num > 0):
                    print "[" + str(num) + "]",
                message = "demo in " + dir + " failed with exit status", status,
                print message
                if keepGoing == False:
                    print "exiting"
                    sys.exit(status)
                else:
                    print " ** Error logged and will be displayed again when suite is completed **"
                    demoErrors.append(message)


def run(demos, protobufDemos = [], root = False):
    def usage():
        print """usage: %s
        --start=index           Start running the demos at the given demo."
        --loop                  Run the demos in a loop."
        --filter=<regex>        Run all the demos that match the given regex."
        --rfilter=<regex>       Run all the demos that do not match the given regex."
        --protobuf              Run the protobuf demos."
        --debug                 Display debugging information on each demos."
        --trace=<output>        Run the demos with tracing enabled."
        --host=host             Set --Ice.Default.Host=<host>."
        --mode=debug|release    Run the demos with debug or release mode builds (win32 only)."
        --continue              Keep running when a demo fails."
        --ice-home=<path>       Use the binary distribution from the given path."
        --x64                   Binary distribution is 64-bit."
        --preferIPv4            Prefer IPv4 stack (java only)."
        --fast                  Run an abbreviated version of the demos."
        --script                Generate a script to run the demos.
        --service-dir=<path>    Directory to locate services for C++Builder/VC6.
        --env                   Dump the environment."
        --noenv                 Do not automatically modify environment.""" % (sys.argv[0])
        sys.exit(2)

    global keepGoing 

    try:
        opts, args = getopt.getopt(sys.argv[1:], "lr:R:", [
                "filter=", "rfilter=", "start=", "loop", "fast", "trace=", "debug", "host=", "mode=",
                "continue", "ice-home=", "x64", "preferIPv4", "env", "noenv", "script", "protobuf", "service-dir="])
    except getopt.GetoptError:
        usage()

    # Extra args cause a usage error.
    if args:
        usage()

    start = 0
    loop = False
    arg = ""
    filters = []
    script = False
    for o, a in opts:
        if o in ("-l", "--loop"):
            loop = True
        elif o == "--x64":
            global x64
            x64 = True
            arg += " " + o
        elif o == "--preferIPv4":
            global preferIPv4
            preferIPv4 = True
            arg += " " + o
        elif o in ("--env", "--noenv"):
            arg += " " + o
        elif o in ("-c", "--continue"):
            keepGoing = True
        elif o in ("-r", "-R", "--filter", '--rfilter'):
            if o in ("--rfilter", "-R"):
                filters.append((re.compile(a), True))
            else:
                filters.append((re.compile(a), False))
        elif o in ("--host", "--fast", "--trace", "--debug", "--mode", "--ice-home"):
            if o == "--mode":
                if a not in ( "debug", "release"):
                    usage()
            arg += " " + o
            if len(a) > 0:
                arg += " " + a
        elif o in '--start':
            start = int(a)
        elif o in '--script':
            script = True
        elif o in '--protobuf':
            demos = demos + protobufDemos
        elif o in '--service-dir':
            global serviceDir
            serviceDir = a

    for demoFilter, removeFilter in filters:
        if removeFilter:
            demos = [ x for x in demos if not demoFilter.search(x) ]
        else:
            demos = [ x for x in demos if demoFilter.search(x) ]

    if loop:
        num = 1
        while 1:
            runDemos(start, arg, demos, num, script = script, root = root)
            num += 1
    else:
        runDemos(start, arg, demos, script = script, root = root)

    if len(demoErrors) > 0:
        print "The following errors occurred:"
        for x in demoErrors:
            print x

def guessBuildModeForDir(cwd):
    import glob
    debugDll = glob.glob(os.path.join(cwd, "*d.dll"))
    # *.dll includes d.dll, so do an intersection on the set.
    dll = [p for p in glob.glob(os.path.join(cwd, "*.dll")) if not p in debugDll ]
    if len(debugDll) > 0 and len(dll) == 0:
        return "debug"
    if len(dll) > 0 and len(debugDll) == 0:
        return "release"
    if len(dll) > 0 and len(debugDll) > 0:
        # Find out which is newer.
        if os.stat(dll[0]).st_ctime > os.stat(debugDll[0]).st_ctime:
            return "release"
        else:
            return "debug"
    return None

def guessBuildMode():
    if not iceHome and sourcedist:
        m = guessBuildModeForDir(os.path.join(toplevel, "cpp", "bin"))
    else:
	m = guessBuildModeForDir(".")
    if m is None:
        raise "cannot guess debug or release mode"
    return m

def isDebugBuild():
    global buildmode
    # Guess the mode, if not set on the command line.
    if not isWin32():
	return False
    if buildmode is None:
	buildmode = guessBuildMode()
	print "(guessed build mode %s)" % buildmode
    return buildmode == "debug"

def getIceVersion():
    config = open(os.path.join(toplevel, "config", "Make.common.rules"), "r")
    return re.search("VERSION[\t\s]*= ([0-9]+\.[0-9]+(\.[0-9]+|b[0-9]*))", config.read()).group(1)

def getServiceDir():
    global serviceDir
    if serviceDir == None:
        serviceDir = "C:\\Ice-" + str(getIceVersion()) + "\\bin"
    return serviceDir
        
def getIceBox(mapping = "cpp"):
    if mapping == "cpp":
        if isNoServices():
            return os.path.join(getServiceDir(), "icebox.exe")
        if isWin32() and isDebugBuild():
	    return "iceboxd"
        return "icebox"
    elif mapping == "cs":
        if isMono(): # Mono cannot locate icebox in the PATH.
            # This is wrong for a demo dist.
            return os.path.join(getIceDir("cs"), "bin", "iceboxnet.exe")
        else:
            return "iceboxnet.exe"
    assert False

def getIceBoxAdmin():
    if isNoServices():
        return os.path.join(getServiceDir(), "iceboxadmin")
    else:
        return "iceboxadmin"

def getIceGridRegistry():
    if isNoServices():
        return os.path.join(getServiceDir(), "icegridregistry")
    else:
        return "icegridregistry"

def getIceGridNode():
    if isNoServices():
        return os.path.join(getServiceDir(), "icegridnode")
    else:
        return "icegridnode"

def getIceGridAdmin():
    if isNoServices():
        return os.path.join(getServiceDir(), "icegridadmin")
    else:
        return "icegridadmin"

def getGlacier2Router():
    if isNoServices():
        return os.path.join(getServiceDir(), "glacier2router")
    else:
        return "glacier2router"

def spawn(command, cwd = None):
    desc = command.split(' ')[0]

    if defaultHost:
        command = '%s %s' % (command, defaultHost)

    # magic
    knownCommands = [ "icegridnode", "icegridregistry", "icebox", "iceboxd", "icegridadmin", "icestormadmin",
                      "iceboxadmin", "transformdb", "glacier2router" ]
    if desc in knownCommands:
        mapping = "cpp"
    else:
        mapping = getMapping()

    if mapping == "cs":
        if isMono():
            command = "mono " + command
        else:
            command = "./" + command
    elif mapping == "py":
        command = "python -u " + command
    elif mapping == "vb":
        command = "./" + command
    elif mapping == "java":
        if preferIPv4:
            command = command.replace("java", "java -Djava.net.preferIPv4Stack=true")
        if isSolaris() and x64:
            command = command.replace("java", "java -d64")

    if debug:
        print '(%s)' % (command)
    if isWin32(): # Under Win32 ./ does not work.
        command = command.replace("./", "")

    return Expect.Expect(command, logfile = tracefile, desc = desc, mapping = mapping, cwd = cwd)

def cleanDbDir(path):
    for filename in [ os.path.join(path, f) for f in os.listdir(path) if f != ".gitignore" and f != "DB_CONFIG"]:
        if os.path.isdir(filename):
            cleanDbDir(filename)
            try:
                os.rmdir(filename)
            except OSError:
                # This might fail if the directory is empty (because
                # it itself contains a .gitignore file.
                pass
        else:
            os.remove(filename)

def addLdPath(libpath):
    if isWin32():
        addenv("PATH", libpath)
    elif isDarwin():
        addenv("DYLD_LIBRARY_PATH", libpath)
    elif isAIX():
        addenv("LIBPATH", libpath)
    elif isSolaris():
        if x64:
            addenv("LD_LIBRARY_PATH_64", libpath)
        else:
            addenv("LD_LIBRARY_PATH", libpath)
    else:
        if x64:
            addenv("LD_LIBRARY_PATH_64", libpath)
        else:
            addenv("LD_LIBRARY_PATH", libpath)

def processCmdLine():
    def usage():
	print "usage: " + sys.argv[0] + " --x64 --preferIPv4 --env --noenv --fast --trace=output --debug --host host --mode=[debug|release] --ice-home=<dir> --service-dir=<dir>"
	sys.exit(2)
    try:
	opts, args = getopt.getopt(sys.argv[1:], "", ["env", "noenv", "x64", "preferIPv4", "fast", "trace=", "debug", "host=", "mode=", "ice-home=", "--servicedir="])
    except getopt.GetoptError:
	usage()

    global fast
    global defaultHost
    global tracefile
    global buildmode
    global x64
    global preferIPv4
    global debug
    global host
    global iceHome
    global serviceDir

    fast = False
    trace = False
    buildmode = None
    x64 = False
    tracefile = None
    env = False
    noenv = False

    for o, a in opts:
	if o == "--debug":
	    debug = True
	if o == "--trace":
	    if a == "stdout":
		tracefile = sys.stdout
	    else:
		tracefile = open(a, "w")
	if o == "--host":
	    host = a
	if o == "--env":
	    env = True
	if o == "--noenv":
	    noenv = True
	if o == "--fast":
	    fast = True
	if o == "--x64":
	    x64 = True
	if o == "--preferIPv4":
	    preferIPv4 = True
        if o == "--ice-home":
            iceHome = a
        if o == "--service-dir":
            serviceDir = a
	if o == "--mode":
	    buildmode = a
	    if buildmode != 'debug' and buildmode != 'release':
		usage()

    if host != "":
	defaultHost = " --Ice.Default.Host=%s" % (host)
    else:
	defaultHost = None

    if not iceHome and os.environ.get("USE_BIN_DIST", "no") == "yes" or os.environ.get("ICE_HOME", "") != "":
        if os.environ.get("ICE_HOME", "") != "":
            iceHome = os.environ["ICE_HOME"]
        elif isLinux():
            iceHome = "/usr"

    if not x64:
        x64 = isWin32() and os.environ.get("XTARGET") == "x64" or os.environ.get("LP64") == "yes"

    if not noenv:
        configurePaths()
    if env:
        dumpenv()

import inspect
frame = inspect.currentframe().f_back
if frame and os.path.split(frame.f_code.co_filename)[1] == "expect.py":
    # If we're not in the demo directory, chdir to the correct
    # location.
    if not os.path.isabs(sys.argv[0]):
        d = os.path.join(os.getcwd(), sys.argv[0])
    else:
        d = sys.argv[0]
    d = os.path.split(d)[0]
    if os.path.normpath(d) != os.getcwd():
        os.chdir(d)
    processCmdLine()
