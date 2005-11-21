# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import getopt, os, re, shutil, string, sys, zipfile

def prependEnvPath(name, path):
    """Prepend a path to an existing environment variable."""
    os.environ[name] = path + os.pathsep + os.environ[name]

def prependEnvPathList(name, list):
    """Prepend a list of paths to an existing environment variable."""
    for path in list:
	prependEnvPath(name, path)

class ExtProgramError:
    def __init__(self, msg):
	self.msg = msg

    def __str__(self):
	return repr(self.msg)

def runprog(command, haltOnError = True):
    result = os.system(command)
    if not result == 0 and haltOnError:
	raise ExtProgramError('Command %s failed with error code %d' % (command, result))

def usage():
    """Print usage/help information"""
    print "Usage: " + sys.argv[0] + " [options]"
    print
    print "Options:"
    print "-h, --help            Show this message."
    print "    --skip-build      Do not build any sources."
    print "    --clean           Clean compiled or staged files."
    print "    --skip-installer  Do not build any installers or merge modules."

def cleanIceDists(sourcesDir, sourcesVersion, installVersion):
    """Clean all Ice distributions."""
    iceHome = os.environ['ICE_HOME']
    if installVersion == "vc71":
	#
	# Ice for C++ 
	#
	os.chdir(iceHome)
	print "Cleaning in " + os.getcwd() + "..."
	runprog("devenv all.sln /useenv /clean Debug")
	runprog("devenv all.sln /useenv /clean Release")

	#
	# Ice for Java 
	# 
	# XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
	# Leave Ice for Java alone. Everything that is needed is already
	# included in the source distribution.
	# XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
	#

	#
	# Ice for C#
	#
	os.chdir(os.path.join(sourcesDir, "IceCS-" + sourcesVersion))
	print "Cleaning in " + os.getcwd() + "..."
	runprog("devenv all.sln /useenv /clean Debug")

	#
	# Ice for PHP
	#
	os.chdir(os.path.join(sourcesDir, "IcePHP-" + sourcesVersion))
	print "Cleaning in " + os.getcwd() + "..."
	runprog("devenv icephp.sln /useenv /clean Release")

	#
	# Ice for Python
	#
	os.chdir(os.path.join(sourcesDir, "IcePy-" + sourcesVersion))
	print "Cleaning in " + os.getcwd() + "..."
	runprog("devenv all.sln /useenv /clean Release")

	#
	# Ice for Visual Basic
	#
	os.rename(os.path.join(sourcesDir, "IceCS-" + sourcesVersion), os.path.join(sourcesDir, "IceCS")) # XXX temp
	os.chdir(os.path.join(sourcesDir, "IceVB-" + sourcesVersion))
	print "Cleaning in " + os.getcwd() + "..."
	runprog("devenv all.sln /useenv /clean Debug")
	os.rename(os.path.join(sourcesDir, "IceCS"), os.path.join(sourcesDir, "IceCS-" + sourcesVersion)) # XXX temp
    elif installVersion == "vc60":
	os.chdir(iceHome)
	print "Cleaning in " + os.getcwd() + "..."
	runprog("msdev all.dsw /useenv /make all - Win32 Debug /clean")
	runprog("msdev all.dsw /useenv /make all - Win32 Release /clean")

def buildIceDists(stageDir, sourcesDir, sourcesVersion, installVersion):
    """Build all Ice distributions."""

    #
    # Update PATH, LIB and INCLUDE environment variables required for
    # building Ice for C++.
    #
    path = [
	os.path.join(stageDir, "berkeley/dev/bin"),
	os.path.join(stageDir, "berkeley/runtime/bin"),
	os.path.join(stageDir, "berkeley/java/bin"),
	os.path.join(stageDir, "expat/runtime/bin"),
	os.path.join(stageDir, "openssl/runtime/bin")
    ]
    if installVersion == "vc60":
	path.append(os.path.join(stageDir, "stlport/dev/bin"))
	path.append(os.path.join(stageDir, "stlport/runtime/bin"))
    prependEnvPathList('PATH', path)

    lib = [
	os.path.join(stageDir, "berkeley/dev/lib"),
	os.path.join(stageDir, "bzip2/dev/lib"),
	os.path.join(stageDir, "expat/dev/lib"),
	os.path.join(stageDir, "openssl/dev/lib")
    ]
    if installVersion == "vc60":
	lib.append(os.path.join(stageDir, "stlport/dev/lib"))
    prependEnvPathList('LIB', lib)

    include = [
	os.path.join(stageDir, "berkeley/dev/include"),
	os.path.join(stageDir, "bzip2/dev/include"),
	os.path.join(stageDir, "expat/dev/include"),
	os.path.join(stageDir, "openssl/dev/include")
    ]
    if installVersion == "vc60":
	include.append(os.path.join(stageDir, "stlport/dev/include/stlport"))
    prependEnvPathList('INCLUDE', include)

    iceHome = os.environ['ICE_HOME']
    prependEnvPath('PATH', os.path.join(iceHome, "bin"))
    prependEnvPath('LIB', os.path.join(iceHome, "lib"))
    prependEnvPath('INCLUDE', os.path.join(iceHome, "include"))

    if installVersion == "vc71":
	#
	# Ice for C++ 
	#
	os.chdir(iceHome)
	print "Building in " + os.getcwd() + "..."
	runprog("devenv all.sln /useenv /build Debug")
	runprog("devenv all.sln /useenv /build Release")

	#
	# Ice for Java  
	#
	# XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
	# Leave Ice for Java alone. Everything that is needed is already
	# included in the source distribution.
	# XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
	#

	#
	# Ice for C#
	#
	os.chdir(os.path.join(sourcesDir, "IceCS-" + sourcesVersion))
	print "Building in " + os.getcwd() + "..."
	runprog("devenv all.sln /useenv /build Debug")

	#
	# Ice for PHP
	#
	phpBinHome = os.environ['PHP_BIN_HOME']
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

	os.chdir(os.path.join(sourcesDir, "IcePHP-" + sourcesVersion))
	print "Building in " + os.getcwd() + "..."
	runprog("devenv icephp.sln /useenv /build Release")

	#
	# Ice for Python
	#
	pythonHome = os.environ['PYTHON_HOME']
	prependEnvPath('LIB', os.path.join(pythonHome, "libs"))
	prependEnvPath('INCLUDE', os.path.join(pythonHome, "include"))

	os.chdir(os.path.join(sourcesDir, "IcePy-" + sourcesVersion))
	print "Building in " + os.getcwd() + "..."
	runprog("devenv all.sln /useenv /build Release")

	#
	# Ice for Visual Basic
	#
	os.chdir(os.path.join(sourcesDir, "IceVB-" + sourcesVersion))
	print "Building in " + os.getcwd() + "..."
	runprog("devenv all.sln /useenv /build Debug")
    elif installVersion == "vc60":
	#
	# Ice for C++ 
	#
	os.chdir(iceHome)
	print "Building in " + os.getcwd() + "..."
	runprog('msdev all.dsw /useenv /make "all - Win32 Debug"')
	runprog('msdev all.dsw /useenv /make "all - Win32 Release"')

def buildMergeModules(startDir, stageDir, sourcesVersion, installVersion):
    """Build third party merge modules."""
    modules = [
	("BerkeleyDBDevKit", "BERKELEYDB_DEV_KIT"),
	("BerkeleyDBRuntime", "BERKELEYDB_RUNTIME"),
	("BerkeleyDBJava", "BERKELEYDB_JAVA"),
	("BZip2DevKit", "BZIP2_DEV_KIT"),
	("BZip2Runtime", "BZIP2_RUNTIME"),
	("ExpatDevKit", "EXPAT_DEV_KIT"),
	("ExpatRuntime", "EXPAT_RUNTIME"),
	("OpenSSLDevKit", "OPENSSL_DEV_KIT"),
	("OpenSSLRuntime", "OPENSSL_RUNTIME")
    ]
    if installVersion == "vc60":
	extras = [ ("STLPortDevKit", "STLPORT_DEV_KIT"), ("STLPortRuntime", "STLPORT_RUNTIME") ]
	modules.extend(extras)
    elif installVersion == "vc71":
	extras = [ ("JGoodies", "JGOODIES_RUNTIME") ]
	modules.extend(extras)

    #
    # Build modules.
    #
    os.chdir(startDir)
    for project, release in modules:
	runprog(os.environ['INSTALLSHIELD_HOME'] + "\IsCmdBld -c COMP -a ZEROC -p " + project + ".ism -r " + release)

    #
    # Archive modules in the stage directory root.
    #
    zipPath = "ThirdPartyMergeModules-" + sourcesVersion + "-" + installVersion.upper() + ".zip"
    zip = zipfile.ZipFile(os.path.join(stageDir, zipPath), 'w')
    for project, release in modules:
	msm = project + "." + installVersion.upper() + ".msm"
	msmPath = os.path.join(os.getcwd(), project, "ZEROC", release, "DiskImages/DISK1", msm)
	zip.write(msmPath, os.path.basename(msmPath))
    zip.close()

def buildInstallers(startDir, stageDir, sourcesVersion, installVersion):
    """Build MSI installers."""
    installers = [("ThirdParty", "THIRD_PARTY_MSI"), ("Ice", "ICE_MSI")]

    #
    # Build and copy to the stage directory root.
    #
    os.chdir(startDir)
    for project, release in installers:
	runprog("ISCmdBld -c COMP -a ZEROC -p " + project + ".ism -r " + release)
	msi = project + "-" + sourcesVersion + "-" + installVersion.upper() + ".msi"
	msiPath = os.path.join(os.getcwd(), project, "ZEROC", release, "DiskImages/DISK1", msi)
	shutil.copy(msiPath, stageDir)

def main():
    #
    # Save our start dir. This script expects that this will be 
    # <ice-cvs-root>/install/[vc60|vc71]
    #
    startDir = os.getcwd()
    print "Start Directory: " + startDir

    installDir = startDir[:startDir.rfind("\\")]
    print "Install Directory: " + installDir

    #
    # Check the installer version string.
    #
    installVersion = startDir[startDir.rfind("\\")+1:].lower()
    if installVersion != "vc60" and installVersion != "vc71":
	print "Invalid install version."
        sys.exit(2)
    else:
	print "Install Version: " + installVersion

    #
    # Where all the files will be staged so that the install projects
    # can find them.
    #
    stageDir = os.path.join(startDir, "install")
    print "Stage Directory: " + stageDir

    #
    # Process args.
    #
    try:
        optionList, args = getopt.getopt(
	    sys.argv[1:], "h:", [ "help", "clean", "skip-build", "skip-installer" ])
    except getopt.GetoptError:
        usage()
        sys.exit(2)

    #
    # Set a few defaults.
    #
    clean = False
    build = True
    installer = True

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
	    'Deleting intermediate files and rebuilding from scratch!'

    #
    # Check the environment for the required vars.
    #
    ok = True
    required = ['ICE_HOME', 'BERKELEY_HOME', 'BZIP2_HOME', 'EXPAT_HOME', 'OPENSSL_HOME', 'JGOODIES_HOME']
    if installVersion == "vc71":
	required.extend(['PHP_BIN_HOME', 'PHP_SRC_HOME', 'PYTHON_HOME'])
    elif installVersion == "vc60":
	required.append('STLPORT_HOME')
    for var in required:
	path = os.environ[var]
	if path == "":
	    print var + " is not set!"
	    ok = False
	elif not os.path.isdir(path):
	    print var + " is invalid!"
	    ok = False
	else:
	    print var + ": " + path
    if not ok:
        sys.exit(2)

    iceHome = os.environ['ICE_HOME']
    berkeleyHome = os.environ['BERKELEY_HOME']
    bzip2Home = os.environ['BZIP2_HOME']
    expatHome = os.environ['EXPAT_HOME']
    opensslHome = os.environ['OPENSSL_HOME']
    jgoodiesHome = os.environ['JGOODIES_HOME']

    if installVersion == "vc71":
	phpBinHome = os.environ['PHP_BIN_HOME']
	phpSrcHome = os.environ['PHP_SRC_HOME']
	pythonHome = os.environ['PYTHON_HOME']
	stlportHome = ""
    elif installVersion == "vc60":
	phpBinHome = ""
	phpSrcHome = ""
	pythonHome = ""
	stlportHome = os.environ['STLPORT_HOME']

    #
    # Extract the Ice source distributions directory.
    #
    sourcesDir = iceHome[:iceHome.rfind("\\Ice-")]
    print "Ice Sources Directory: " + sourcesDir

    #
    # Extract the Ice distribution version.
    #
    sourcesVersion = iceHome[iceHome.rfind("-")+1:]
    if not re.match("^\d\.\d\.\d$", sourcesVersion):
	print "Invalid Version: " + sourcesVersion
        sys.exit(2)
    else:
	print "Ice Sources Version: " + sourcesVersion

    antOptions = " -buildfile " + os.path.join(installDir, "common/stage.xml") + \
		 " -Dinstall.dir=" + installDir + \
		 " -Dinstall.version=" + installVersion + \
                 " -Dsources.dir=" + sourcesDir + \
                 " -Dsources.version=" + sourcesVersion + \
                 " -Dstage.dir=" + stageDir 

    #
    # This should be performed every time. The third party libraries are
    # rebuilt 'out-of-band' so there is no way for the script to pickup
    # changes in them.
    #
    runprog("ant" + antOptions + " clean")

    if clean:
	cleanIceDists(sourcesDir, sourcesVersion, installVersion)

    #
    # Stage the third party packages.
    #
    if installer:
	runprog("ant" + antOptions + " packages-stage-" + installVersion)

    #
    # Build the Ice distributions.
    #
    if build:
	buildIceDists(stageDir, sourcesDir, sourcesVersion, installVersion)

    #
    # Build the merge module and installer projects.
    #
    if installer:
	runprog("ant" + antOptions + " ice-stage-" + installVersion)
	buildMergeModules(startDir, stageDir, sourcesVersion, installVersion)
	buildInstallers(startDir, stageDir, sourcesVersion, installVersion)

if __name__ == "__main__":
    main()
