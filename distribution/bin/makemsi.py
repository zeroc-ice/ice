#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import getopt, os, re, shutil, string, sys, zipfile, fileinput
import logging, cStringIO, glob
import textwrap

iceVersion = '3.5b'
looksVersion = '2.2.2'
formsVersion = '1.2.1'

timeStampingURL = 'http://timestamp.verisign.com/scripts/timstamp.dll'

resources = os.path.join(os.path.abspath(os.path.dirname(sys.argv[0])), "..", "src", "windows")
sys.path.append(resources)
import components

DistPrefixes = ["Ice-%s"]

class DistEnvironmentError:
    def __init__(self, msg = None):
        self.msg = msg

    def __str__(self):
        return repr(self.msg)

class ExtProgramError:
    def __init__(self, msg):
        self.msg = msg

    def __str__(self):
        return repr(self.msg)

def prependEnvPath(name, path):
    """Prepend a path to an existing environment variable."""
    logging.debug('Prepending %s to %s' % (path, os.environ[name]))
    os.environ[name] = path + os.pathsep + os.environ[name]

def prependEnvPathList(name, list):
    """Prepend a list of paths to an existing environment variable."""
    for path in list:
        prependEnvPath(name, path)

def runprog(command, haltOnError = True):
    logging.debug('Running external command: %s' % command)
    result = os.system(command)
    if not result == 0:
        msg = 'Command %s failed with error code %d' % (command, result)
        if haltOnError:
            raise ExtProgramError('Command %s failed with error code %d' % (command, result))
        else:
            logging.error(msg)

def usage():
    """Print usage/help information"""
    print "Usage: " + sys.argv[0] + " [options]"
    print
    print "Options:"
    print "-h, --help            Show this message."
    print "    --skip-build      Do not build any sources."
    print "    --clean           Clean compiled or staged files."
    print "    --skip-installer  Do not build any installers or merge modules."
    print "-i, --info            Log information messages"
    print "-d, --debug           Log debug messages"
    print "-l, --logfile         Specify the destination log file"

def environmentCheck(target):
    """Warning: uses global environment."""
    required = ["BUILD_DIR", "THIRDPARTY_HOME"]

    if target == "vc60":
        required.extend(["PHP_HOME", "PHP_SRC_HOME"])
    elif target == "vc80" or target == "vc90":
        required.extend(["PYTHON_HOME_X86", "PYTHON_HOME_X64"])

    fail = False

    for f in required:
        if not os.environ.has_key(f):
            logging.error("Environment variable %s is missing" % f)
            fail = True
            continue

        if not os.path.isdir(os.environ[f]):
            logging.error("Value %s for env var %s is not a valid directory." % (os.environ[f], f))
            fail = True
            continue

    if fail:
        logging.error("Invalid environment. Please consult error log and repair environment/command line settings.")
        sys.exit(2)

def checkSources(buildDir, sourceDir):
    """Scans a directory for source distributions."""

    if not os.path.exists(os.path.join(sourceDir, 'distfiles-' + iceVersion + '.tar.gz')):
        print "Unable to locate distfiles.tar.gz"
        sys.exit(1)

    installFiles = os.path.join(buildDir, "install")
    if not os.path.exists(installFiles):
        os.mkdir(os.path.join(buildDir, "install"))

    command = 'bash -c "gzip -dc `cygpath ' + \
            os.path.join(os.path.join(sourceDir, 'distfiles-' + iceVersion + '.tar.gz')).replace("\\", r'/') + \
            "` | tar xf - -C " + installFiles.replace("\\", r'/') + '"'
    result = os.system(command)
    if result != 0:
        print "Unable to extract distfiles.tar.gz"
        sys.exit(1)

def setMakefileOption(filename, optionName, value):
    optre = re.compile("^\#?\s*?%s\s*?=.*" % optionName)
    f = fileinput.input(filename, True)
    for line in f:
        l = line.rstrip('\n')
        if optre.search(l):
            print "%s = %s" % (optionName, value)
        else:
            print l
    f.close()

def setMakefileYesNoOption(filename, optionName, value):
    optionString = "no"
    if value:
        optionString = "yes"
    setMakefileOption(filename, optionName, optionString)

def setOptimize(filename, optimizeOn):
    setMakefileYesNoOption(filename, "OPTIMIZE", optimizeOn)

def setDefaultCompiler(filename, compiler):
    setMakefileOption(filename, "CPP_COMPILER", compiler)

def setDebug(filename, debugOn):
    setMakefileYesNoOption(filename, "DEBUG", debugOn)

def buildIceDists(stageDir, sourcesDir, iceVersion, installVersion):
    """Build all Ice distributions."""

    #
    # Run debug builds first.
    #
    iceCppHome = os.path.join(sourcesDir, "debug", "Ice-%s" % iceVersion, "cpp")

    os.chdir(iceCppHome)
    setOptimize(os.path.join(os.getcwd(), "config", "Make.rules.mak"), False)
    os.chdir(os.path.join(iceCppHome, "src"))
    runprog("nmake /f Makefile.mak")
    
    #
    # Now run the release mode builds.
    #
    iceCppHome = os.path.join(sourcesDir, "release", "Ice-%s" % iceVersion, "cpp")
  
    os.chdir(iceCppHome)
    setOptimize(os.path.join(os.getcwd(), "config", "Make.rules.mak"), True)
    os.chdir(os.path.join(iceCppHome, "src"))
    runprog("nmake /f Makefile.mak")
   
    if installVersion == "vc80" or installVersion == "vc90":

        #
        # Ice for Python
        #
        os.environ['PYTHON_HOME'] = os.environ['PYTHON_HOME_X86']
      
        os.chdir(os.path.join(sourcesDir, "release", "Ice-" + iceVersion, "py"))
        print "Building in " + os.getcwd() + "..."
        setOptimize(os.path.join(os.getcwd(), "config", "Make.rules.mak"), True)
        runprog("nmake /f Makefile.mak")

    if installVersion == "vc80" or installVersion == "vc90":
     
        #
        # Ice for .NET
        #
        os.chdir(os.path.join(sourcesDir, "release", "Ice-%s" % iceVersion, "cs" ))
        print "Building in " + os.getcwd() + "..."
        setOptimize(os.path.join(os.getcwd(), "config", "Make.rules.mak.cs"), True)
        setDebug(os.path.join(os.getcwd(), "config", "Make.rules.mak.cs"), True)
        runprog("nmake /f Makefile.mak")
        os.chdir(os.path.join(sourcesDir, "release", "Ice-%s" % iceVersion, "cs", "demo"))
        runprog("nmake /f Makefile.mak config")
        os.chdir(os.path.join(sourcesDir, "release", "Ice-%s" % iceVersion, "vb", "demo"))
        runprog("nmake /f Makefile.mak config")

        #
        # Ice for Java
        #
        jgoodiesLooks = os.path.join(os.environ['THIRDPARTY_HOME'], 'lib', "looks-%s.jar" % looksVersion) 
        jgoodiesForms = os.path.join(os.environ['THIRDPARTY_HOME'], 'lib', "forms-%s.jar" % formsVersion) 
        dbJar = os.path.join(os.environ['THIRDPARTY_HOME'], 'lib', 'db.jar') 

        os.environ['CLASSPATH'] = jgoodiesLooks + os.pathsep + jgoodiesForms + os.pathsep + dbJar + os.pathsep + os.environ['CLASSPATH']
        os.chdir(os.path.join(sourcesDir, "release", "Ice-%s" % iceVersion, "java" ))
        print "Building in " + os.getcwd() + "..."
 
        jgoodiesDefines = "-Djgoodies.forms=" + jgoodiesForms + " -Djgoodies.looks=" + jgoodiesLooks
     
        runprog("ant -Dice.mapping=java2 -Dbuild.suffix=java2 jar")
        runprog("ant -Dice.mapping=java5 -Dbuild.suffix=java5 " + jgoodiesDefines + " jar")

        #
        # x64 build
        #

        os.environ['PATH'] = os.path.join(os.environ['VCINSTALLDIR'], 'bin','x86_amd64') + os.pathsep + os.environ['PATH']
        os.environ['XTARGET'] = 'x64'

        if installVersion == "vc80":
            
            x64Lib = [
                os.path.join(os.environ['VSINSTALLDIR'], 'SDK', 'v2.0', 'lib', 'amd64'),
                os.path.join(os.environ['VCINSTALLDIR'], 'PlatformSDK', 'lib', 'amd64'),
                os.path.join(os.environ['VCINSTALLDIR'], 'lib', 'amd64'),
                os.path.join(os.environ['VCINSTALLDIR'], 'ATLMFC', 'lib', 'amd64')
                ]
            prependEnvPathList('LIB', x64Lib)

            os.environ['LIBPATH'] = os.path.join(os.environ['VCINSTALLDIR'], 'ATLMFC', 'lib', 'amd64') + os.pathsep + os.environ['LIBPATH']
             
        if installVersion == "vc90":
            
            x64Lib = [
                os.path.join(os.environ['VCINSTALLDIR'], 'lib', 'amd64'),
                os.path.join(os.environ['VCINSTALLDIR'], 'ATLMFC', 'lib', 'amd64'),
                os.path.join(os.environ['WindowsSdkDir'], 'lib', 'x64')
                ]
            prependEnvPathList('LIB', x64Lib)

            x64LibPath = [
                os.path.join(os.environ['VCINSTALLDIR'], 'lib', 'amd64'),
                os.path.join(os.environ['VCINSTALLDIR'], 'ATLMFC', 'lib', 'amd64'),
                os.path.join(os.environ['FrameworkDir'] + '64', os.environ['FrameworkVersion']),
                os.path.join(os.environ['FrameworkDir'] + '64', os.environ['Framework35Version'])
                ]
            prependEnvPathList('LIBPATH', x64LibPath)
       

        #
        # Run debug builds first.
        #
        iceCppHome = os.path.join(sourcesDir, "debug-x64", "Ice-%s" % iceVersion, "cpp")

        os.chdir(iceCppHome)
        setOptimize(os.path.join(os.getcwd(), "config", "Make.rules.mak"), False)
        os.chdir(os.path.join(iceCppHome, "src"))
        runprog("nmake /f Makefile.mak")
    
        #
        # Now run the release mode builds.
        #
        iceCppHome = os.path.join(sourcesDir, "release-x64", "Ice-%s" % iceVersion, "cpp")
        
        os.chdir(iceCppHome)
        setOptimize(os.path.join(os.getcwd(), "config", "Make.rules.mak"), True)
        os.chdir(os.path.join(iceCppHome, "src"))
        runprog("nmake /f Makefile.mak")
  
        if installVersion == "vc80" or installVersion == "vc90":
            #
            # Ice for Python
            #
            os.environ['PYTHON_HOME'] = os.environ['PYTHON_HOME_X64']
            os.chdir(os.path.join(sourcesDir, "release-x64", "Ice-" + iceVersion, "py"))
            print "Building in " + os.getcwd() + "..."
            setOptimize(os.path.join(os.getcwd(), "config", "Make.rules.mak"), True)
            runprog("nmake /f Makefile.mak")

    if installVersion == "vc60":
        #
        # Ice for PHP
        #
        phpBinHome = os.environ['PHP_HOME']
        phpLib = [
            os.path.join(phpBinHome, "lib"),
            os.path.join(phpBinHome, "dev")
        ]
        prependEnvPathList('LIB', phpLib)

        phpSrcHome = os.environ['PHP_SRC_HOME']
        phpInc = [
            phpSrcHome,
            os.path.join(phpSrcHome, "main"),
            os.path.join(phpSrcHome, "Zend"),
            os.path.join(phpSrcHome, "TSRM")
        ]
        prependEnvPathList('INCLUDE', phpInc)

        os.chdir(os.path.join(sourcesDir, "release", "Ice-" + iceVersion, "php"))
        print "Building in " + os.getcwd() + "..."
        setOptimize(os.path.join(os.getcwd(), "config", "Make.rules.mak"), True)
        runprog("nmake /f Makefile.mak")


        #
        # Ice for Ruby
        #
        os.chdir(os.path.join(sourcesDir, "release", "Ice-%s" % iceVersion, "rb" ))
        setOptimize(os.path.join(os.getcwd(), "config", "Make.rules.mak"), True)
        print "Building in " + os.getcwd() + "..."
        runprog("nmake /f Makefile.mak")

def list2english(l):
    if len(l) == 1:
        return l[0]
    elif len(l) == 2:
        return l[0] + " and " + list2english(l[1:])
    else:
        return l[0] + ", " + list2english(l[1:])


def buildInstallers(startDir, stageDir, iceVersion, installVersion, installers):

    installVersion = installVersion.upper()

    #
    # Build msi
    #
    os.chdir(startDir)
    for project, release in installers:
        runprog(os.environ['INSTALLSHIELD_HOME'] + "\IsCmdBld -x -w -c COMP -a ZEROC -p " + project + ".ism -r " + release)
        if project == "Ice":
            msi = project + "-" + iceVersion + "-" + installVersion + ".msi"
        else:
            msi = "Ice-" + iceVersion + "-" + project + "-" + installVersion + ".msi"
        msiPath = os.path.join(os.getcwd(), project, "ZEROC", release, "DiskImages/DISK1", msi)
        runprog('signtool sign /f ' + os.environ['PFX_FILE'] + ' /p ' + os.environ['PFX_PASSWORD'] 
                + ' /t ' + timeStampingURL
                + ' /d ' + msi
                + ' /du http://www.zeroc.com'
                + ' ' + msiPath)

def environToString(tbl):
    '''Convert an environment hashtable to the typical k=v format'''
    ofile = cStringIO.StringIO()
    result = ''
    try:
        for k, v in tbl.iteritems():
            ofile.write('%s=%s\n' % (k, v))
        result = ofile.getvalue()
    finally:
        ofile.close()
    return result

def main():

    #
    # Save our start dir.
    #
    startDir = os.getcwd()

    #
    # baseDir will be the reference point for locating the working files
    # for the installer production scripts. It is defined as the parent
    # directory for this python file. We'll use the absolute path as
    # while it is verbose, it is more valuable in tracing.
    #
    installDir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
    os.environ['INSTALL_TOOLS'] = installDir

    try:
        #
        # Process args.
        #
        try:
            optionList, args = getopt.getopt(
                sys.argv[1:], "dhil:", [ "help", "clean", "skip-build", "skip-installer", "info", "debug",
                "logfile", "vc60", "vc80", "vc90", "thirdpartyhome=", "sources=", "buildDir=", "pfxfile=", "pfxpassword="])
        except getopt.GetoptError:
            usage()
            sys.exit(2)

        #
        # Set a few defaults.
        #
        clean = False
        build = True
        installer = True
        debugLevel = logging.NOTSET
        logFile = None
        target = None

        for o, a in optionList:
            if o in ("-h", "--help"):
                usage()
                sys.exit()
            elif o == "--clean":
                clean = True
            elif o == "--skip-build":
                build = False
            elif o == "--skip-installer":
                installer = False
            elif o in ('-l', '--logfile'):
                logFile = a
            elif o in ('-d', '--debug'):
                debugLevel = logging.DEBUG
            elif o in ('-', '--info'):
                debugLevel = logging.INFO
            elif o == '--vc60':
                target = 'vc60'
            elif o == '--vc80':
                target = 'vc80'
            elif o == '--vc90':
                target = 'vc90'
            elif o == '--pfxfile':
                os.environ['PFX_FILE'] = a
            elif o == '--pfxpassword':
                os.environ['PFX_PASSWORD'] = a
            elif o == '--sources':
                os.environ['SOURCES'] = a
            elif o == '--buildDir':
                os.environ['BUILD_DIR'] = a
            elif o == '--thirdpartyhome':
                os.environ['THIRDPARTY_HOME'] = a

        if debugLevel != logging.NOTSET:
            if a != None:
                logging.basicConfig(level = debugLevel, format='%(asctime)s %(levelname)s %(message)s', filename = a)
            else:
                logging.basicConfig(level = debugLevel, format='%(asctime)s %(levelname)s %(message)s')

        if target == None:
            print 'The development target must be specified'
            sys.exit(2)

        os.environ['target'] = target
        os.environ['CPP_COMPILER'] = target.upper()
        os.environ['PATH'] = os.path.join(os.environ['THIRDPARTY_HOME'], 'bin') + os.pathsep + os.environ['PATH']

        #
        # Where all the files will be staged so that the install projects
        # can find them.
        #
        targetDir = os.path.join(installDir, "src", "windows", target)
        stageDir = os.path.join(targetDir, "install")

        logging.info("Install Tool: " + installDir)
        logging.info("Target Directory: " + targetDir)
        logging.info("Stage Directory: " + stageDir)

        if clean:
            print('You have indicated you want to ''clean'' files, starting from scratch.')
            confirm = ''
            while not confirm in ['y', 'n']:
                confirm = raw_input('Are you sure? [y/N]')
                if confirm == '':
                    confirm = 'n'
            if confirm == 'n':
                sys.exit()
            else:
                logging.info('Deleting intermediate files and rebuilding from scratch!')

        logging.info('Starting windows installer creation.')

        environmentCheck(target)

        buildDir = os.environ['BUILD_DIR']

        logging.debug(environToString(os.environ))

        if build:
            checkSources(buildDir, os.environ['SOURCES'])

        defaults = os.environ
        defaults['dbver'] = '46'
        defaults['version'] = iceVersion
        defaults['dllversion'] = iceVersion.replace('.', '')[:2]
        if iceVersion.find('b') != -1:
            defaults['dllversion'] = defaults['dllversion'] + 'b'

        if target == 'vc80':
            defaults['installdir'] = "C:\\Ice-%s" % iceVersion
        else:
            defaults['installdir'] = "C:\\Ice-%s-%s" % (iceVersion, target.upper())

        defaults['pdb'] = 'pdb'
        defaults['OutDir'] = ''
        defaults['timeStampingURL'] = timeStampingURL

        if os.path.exists(stageDir):
            try:
                shutil.rmtree(stageDir)
            except IOError:
                print """
If you are getting a permission error here, try running 'attrib -r /s'
on both the stage directory and the source location for the third party
libraries."""
                raise
            os.mkdir(stageDir)

        #
        # Third-party license&source files.
        #

        if not os.path.exists(os.path.join(resources, "docs", target)):
            os.mkdir(os.path.join(resources, "docs", target))

        shutil.copy(os.path.join(os.environ['THIRDPARTY_HOME'], "SOURCES"),
            os.path.join(resources, "docs", target, "THIRD_PARTY_SOURCES"))

        shutil.copy(os.path.join(os.environ['THIRDPARTY_HOME'], "LICENSE"),
            os.path.join(resources, "docs", target, "THIRD_PARTY_LICENSE"))


        #
        # Screw clean rules, run the ultimate clean!
        #
        if clean:
            if os.path.exists(os.path.join(buildDir, "debug")):
               shutil.rmtree(os.path.join(buildDir, "debug"))
            if  os.path.exists(os.path.join(buildDir, "release")):
               shutil.rmtree(os.path.join(buildDir, "release"))
            if os.path.exists(os.path.join(buildDir, "debug-x64")):
               shutil.rmtree(os.path.join(buildDir, "debug-x64"))
            if  os.path.exists(os.path.join(buildDir, "release-x64")):
               shutil.rmtree(os.path.join(buildDir, "release-x64"))

        if not os.path.exists(buildDir):
            os.mkdir(buildDir)
        if not os.path.exists(os.path.join(buildDir, "debug")):
            os.mkdir(os.path.join(buildDir, "debug"))
        if not os.path.exists(os.path.join(buildDir, "release")):
            os.mkdir(os.path.join(buildDir, "release"))

        if target == "vc80" or target == "vc90":
            if not os.path.exists(os.path.join(buildDir, "debug-x64")):
                os.mkdir(os.path.join(buildDir, "debug-x64"))
            if not os.path.exists(os.path.join(buildDir, "release-x64")):
                os.mkdir(os.path.join(buildDir, "release-x64"))

        if build:
            for z in DistPrefixes:
                #
                # TODO: See if this can be replaced by ZipFile and native
                # Python code somehow.
                #
                filename = os.path.join(os.environ['SOURCES'], z % iceVersion + ".zip")
                if not os.path.exists(os.path.join(buildDir, "debug", z %  iceVersion)):
                    runprog("unzip -o -q %s -d %s" % (filename, os.path.join(buildDir, "debug")))
                if not os.path.exists(os.path.join(buildDir, "release", z %  iceVersion)):
                    runprog("unzip -o -q %s -d %s" % (filename, os.path.join(buildDir, "release")))
        
                if target == "vc80" or target == "vc90":
                    if not os.path.exists(os.path.join(buildDir, "debug-x64", z %  iceVersion)):
                        runprog("unzip -o -q %s -d %s" % (filename, os.path.join(buildDir, "debug-x64")))
                    if not os.path.exists(os.path.join(buildDir, "release-x64", z %  iceVersion)):
                        runprog("unzip -o -q %s -d %s" % (filename, os.path.join(buildDir, "release-x64")))

        # Need .txt version
        shutil.copy(os.path.join(buildDir, "release", "Ice-%s" % iceVersion, "RELEASE_NOTES"),
                    os.path.join(buildDir, "release", "Ice-%s" % iceVersion, "RELEASE_NOTES.txt"))
        shutil.copy(os.path.join(buildDir, "release", "Ice-%s" % iceVersion, "CHANGES"),
                    os.path.join(buildDir, "release", "Ice-%s" % iceVersion, "CHANGES.txt"))

        #
        # Build the Ice distributions.
        #
        if build:
            buildIceDists(stageDir, buildDir, iceVersion, target)

        #
        # Stage Ice!
        #
        components.stage(os.path.join(os.path.dirname(components.__file__), "components", "components.ini"),
                os.path.join(os.path.dirname(components.__file__), "components"), stageDir, "ice", defaults)

        #
        # Build the installer projects.
        #
        if installer:
            buildInstallers(targetDir, stageDir, iceVersion, target, [("Ice", "ICE_MSI")])

    finally:
        #
        # Return the user to where they started.
        #
        os.chdir(startDir)

if __name__ == "__main__":
    main()
