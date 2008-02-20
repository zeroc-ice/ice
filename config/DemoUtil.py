#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, getopt, re

keepGoing = False
ice_home = None
toplevel = None
testErrors = []

def configurePaths():
    os.environ["PATH"] = os.path.join(getIceDir("cpp"), "bin") + os.pathsep + os.getenv("PATH", "")
    os.environ["PATH"] = os.path.join(getIceDir("cs"), "bin") + os.pathsep + os.getenv("PATH", "")

    if not isCygwin():
        libDir = os.path.join(getIceDir("cpp"), "lib")
        if isHpUx():
            os.environ["SHLIB_PATH"] = libDir + os.pathsep + os.getenv("SHLIB_PATH", "")
        elif isDarwin():
            os.environ["DYLD_LIBRARY_PATH"] = libDir + os.pathsep + os.getenv("DYLD_LIBRARY_PATH", "")
        elif isAIX():
            os.environ["LIBPATH"] = libDir + os.pathsep + os.getenv("LIBPATH", "")
        else:
            os.environ["LD_LIBRARY_PATH"] = libDir + os.pathsep + os.getenv("LD_LIBRARY_PATH", "")
            os.environ["LD_LIBRARY_PATH_64"] = libDir + os.pathsep + os.getenv("LD_LIBRARY_PATH_64", "")

    javaDir = getIceDir("java")
    os.environ["CLASSPATH"] = os.path.join(javaDir, "lib", "Ice.jar") + os.pathsep + os.getenv("CLASSPATH", "")
    os.environ["CLASSPATH"] = os.path.join(javaDir, "lib") + os.pathsep + os.getenv("CLASSPATH", "")
    os.environ["CLASSPATH"] = os.path.join("classes") + os.pathsep + os.getenv("CLASSPATH", "")

    # 
    # On Windows, C# assemblies are found thanks to the .exe.config files.
    #
    if not isCygwin():
        os.environ["MONO_PATH"] = os.path.join(getIceDir("cs"), "bin") + os.pathsep + os.getenv("MONO_PATH", "")

    os.environ["PYTHONPATH"] = os.path.join(getIceDir("py"), "python") + os.pathsep + os.getenv("PYTHONPATH", "")
    os.environ["RUBYLIB"] = os.path.join(getIceDir("rb"), "ruby") + os.pathsep + os.getenv("RUBYLIB", "")

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
    if os.environ.has_key("ICE_HOME"):
        return os.environ["ICE_HOME"]
    elif subdir:
        return os.path.join(findTopLevel(), subdir)
    else:
        return findTopLevel()

def isCygwin():
    # The substring on sys.platform is required because some cygwin
    # versions return variations like "cygwin_nt-4.01".
    return sys.platform[:6] == "cygwin"

def isHpUx():
   return sys.platform == "hp-ux11"

def isAIX():
   return sys.platform in ['aix4', 'aix5']

def isDarwin():
   return sys.platform == "darwin"

if sys.platform == "win32":
    print "allDemos.py only supports cygwin python under Windows (use /usr/bin/python allDemos.py)"
    sys.exit(1)

def runDemos(args, demos, num = 0):
    global testErrors
    global keepGoing

    configurePaths()

    rootPath = "demo"
    if not os.path.exists(rootPath):
        rootPath = "."
        
    if len(demos) > 0 and not os.path.exists(os.path.join(rootPath, os.path.normpath(demos[0]))):
        print "Unable to locate first demo. Check directory structure and location of scripts"
        sys.exit(1)

    #
    # Run each of the demos.
    #
    for i in demos:

        i = os.path.normpath(i)
        dir = os.path.join(rootPath, i)

        print
        if(num > 0):
            print "[" + str(num) + "]",
        print "*** running demo in " + dir,
        print

        if isCygwin():
            status = os.system("cd %s ; %s %s" % (dir, "/usr/bin/python expect.py", args))
        else:
            status = os.system("cd %s ; %s %s" % (dir, "./expect.py", args))

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
                testErrors.append(message)

def usage():
    print "usage: %s " % (sys.argv[0])
    print "  --start=<regex>         Start running the demos at the given demo."
    print "  --start-after=<regex>   Start running the demos after the given demo."
    print "  --loop                  Run the demos in a loop."
    print "  --filter=<regex>        Run all the demos that match the given regex."
    print "  --rfilter=<regex>       Run all the demos that do not match the given regex."
    print "  --fast                  Run an abbreviated version of the demos."
    print "  --debug                 Display debugging information on each demos."
    print "  --trace                 Run the demos with tracing enabled."
    print "  --host=host             Set --Ice.Default.Host=<host>."
    print "  --mode=debug|release    Run the demos with debug or release mode builds (win32 only)."
    print "  --continue              Keep running when a demo fails."
    print "  --ice-home=<path>       Use the binary distribution from the given path."
    sys.exit(2)

def index(l, re):
    """Find the index of the first item in the list that matches the given re"""
    for i in range(0, len(l)):
        if re.search(l[i]):
            return i
    return -1

def run(demos):
    try:
        opts, args = getopt.getopt(sys.argv[1:], "lr:R:", [
                "filter=", "rfilter=", "start-after=", "start=", "loop", "fast", "trace", "debug", "host=", "mode=",
                "continue", "ice-home="])
    except getopt.GetoptError:
        usage()

    # Extra args cause a usage error.
    if args:
        usage()

    demoFilter = None
    removeFilter = False
    start = 0
    loop = False
    arg = ""
    for o, a in opts:
        if o in ("-l", "--loop"):
            loop = True
        elif o == "--ice-home":
            global ice_home
            ice_home = a
        elif o in ("-c", "--continue"):
            keepGoing = True
        elif o in ("-r", "-R", "--filter", '--rfilter'):
            demoFilter = re.compile(a)
            if o in ("--rfilter", "-R"):
                removeFilter = True
        elif o in ("--host", "--fast", "--trace", "--debug", "--mode"):
            if o == "--mode":
                if a not in ( "debug", "release"):
                    usage()
            arg += " " + o
            if len(a) > 0:
                arg += " " + a
        elif o in ('--start', "--start-after"):
            start = index(demos, re.compile(a))
            if start == -1:
                print "demo %s not found. no demos to run" % (a)
                sys.exit(2)
            if o == "--start-after":
                start += 1
            demos = demos[start:]

    if demoFilter != None:
        if removeFilter:
            demos = [ x for x in demos if not demoFilter.search(x) ]
        else:
            demos = [ x for x in demos if demoFilter.search(x) ]
            
    if loop:
        num = 1
        while 1:
            runDemos(arg, demos, num)
            num += 1
    else:
        runDemos(arg, demos)

    if len(testErrors) > 0:
        print "The following errors occurred:"
        for x in testErrors:
            print x
