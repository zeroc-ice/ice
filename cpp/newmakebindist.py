#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************
import os, sys, shutil, re, string, getopt, glob, logging, fileinput
import RPMTools

#
# TODO:
#
#  * Tidying and tracing.
#

def getIceVersion(file):
    """Extract the ICE version string from a file."""
    config = open(file, "r")
    return  re.search("ICE_STRING_VERSION \"([0-9\.]*)\"", config.read()).group(1)

def getIceSoVersion(file):
    """Extract the ICE version ordinal from a file."""
    config = open(file, "r")
    intVersion = int(re.search("ICE_INT_VERSION ([0-9]*)", config.read()).group(1))
    majorVersion = intVersion / 10000
    minorVersion = intVersion / 100 - 100 * majorVersion
    return '%d' % (majorVersion * 10 + minorVersion)

def getPlatform():
    """Determine the platform we are building and targetting for"""
    if sys.platform.startswith("win") or sys.platform.startswith("cygwin"):
        return "win32"
    elif sys.platform.startswith("linux"):
        return "linux"
    elif sys.platform.startswith("sunos"):
        return "solaris"
    elif sys.platform.startswith("hp"):
        return "hpux"
    elif sys.platform.startswith("darwin"):
        return "macosx"
    elif sys.platform.startswith("aix"):
        return "aix"
    else:
        return None
    
def initDirectory(d):
    """Check for the existance of the directory. If it isn't there make it."""
    
    if os.path.exists(d):
        #
        # Make sure its a directory and has correct permissions.
        #
        if not os.path.isdir(d):
            print "Path " + d + " exists but is not a directory."
            sys.exit(1)
            
        if os.access(d, os.X_OK | os.R_OK | os.W_OK):
            logging.info("Path " + d + " exists and is ok, continuing")
        else:
            logging.warning("Directory " + d + " exists, but has incorrect permissions")
            sys.exit(1)
    else:
        #
        # This could fail due to lack of permissions.
        #
        os.makedirs(d, 0770)

def getVersion(cvsTag, buildDir):
    """Extracts a source file from the repository and gets the version number from it"""
    cwd = os.getcwd()
    os.chdir(buildDir)
    if getPlatform() == "aix":
        os.environ["LIBPATH"] = ""
    os.system("cvs -d cvs.mutablerealms.com:/home/cvsroot export -r " + cvsTag + " ice/include/IceUtil/Config.h")

    result = [ getIceVersion("ice/include/IceUtil/Config.h"), getIceSoVersion("ice/include/IceUtil/Config.h")]
    os.remove("ice/include/IceUtil/Config.h")
    os.removedirs("ice/include/IceUtil")
    os.chdir(cwd)
    return result

def collectSourceDistributions(tag, sourceDir, cvsdir, distro):
    """The location for the source distributions was not supplied so
       we are going to assume we are being called from a CVS tree and we
       are going to go get them ourselves"""
    cwd = os.getcwd()
    os.chdir(cwd + "/../" + cvsdir)
    if cvsdir == "icepy" or cvsdir == "ice":
        os.system("./makedist.py " + tag)
    else:
        os.system("./makedist.py -d " + tag)
    shutil.copy("dist/" + distro + ".tar.gz", sourceDir)
    os.chdir(cwd)

def extractDemos(sources, buildDir, version, distro, demoDir):
    """Pulls the demo directory out of a distribution and massages its
       build system so it can be built against an installed version of
       Ice"""
    cwd = os.getcwd()
    os.chdir(buildDir + "/demotree")
    os.system("gzip -dc " + sources + "/" + distro + ".tar.gz | tar xf - " + distro + "/demo " + distro + "/config " \
	    + distro + "/certs")
    if demoDir == "":
	os.system("gzip -dc " + sources + "/" + distro + ".tar.gz | tar xf - " + distro + "/install/unix/README.DEMOS")
	shutil.move(distro + "/install/unix/README.DEMOS", buildDir + "/Ice-" + version + "-demos/README.DEMOS")
	
    shutil.move(distro + "/demo", buildDir + "/Ice-" + version + "-demos/demo" + demoDir)

    #
    # 'System' copying of files here because its just easier!  We don't
    # need any configuration out of the Python tree.
    # 
    if not demoDir == "py":
	os.system("cp " + distro + "/config/* " + buildDir + "/Ice-" + version + "-demos/config")

    if not os.path.exists(buildDir + "/Ice-" + version + "-demos/certs"):
	os.mkdir(buildDir + "/Ice-" + version + "-demos/certs")

    os.system("cp -pR " + distro + "/certs/* " + buildDir + "/Ice-" + version + "-demos/certs")

    # 
    # Clean up some unwanted files.
    #
    if os.path.exists(buildDir + "/Ice-" + version + "-demos/certs/openssl"):
	os.system("rm -rf " + buildDir + "/Ice-" + version + "-demos/certs/openssl")
    if os.path.exists(buildDir + "/Ice-" + version + "-demos/certs/makecerts"):
	os.system("rm " + buildDir + "/Ice-" + version + "-demos/certs/makecerts")
    if os.path.exists(buildDir + "/Ice-" + version + "-demos/config/makecerts"):
	os.system("rm " + buildDir + "/Ice-" + version + "-demos/config/makecerts")

    #
    # C++ specific build modifications.
    #
    if demoDir == "" or demoDir == "cs":
        tcwd = os.getcwd()
        os.chdir(buildDir + "/Ice-" + version + "-demos/config")

	state = 'header'
	reIceLocation = re.compile('^[a-z]*dir.*=\s*\$\(top_srcdir\)')
	filename = 'Make.rules'
	if demoDir == 'cs':
	    filename = filename + '.cs'
	for line in fileinput.input(filename, True, ".bak"):
	    if state == 'done':
		if line.startswith('slicedir'):
		    state = 'untilblank'
		    print """
ifeq ($(ICE_DIR),/usr)
    slicedir = $(ICE_DIR)/share/slice
else
    slicedir = $(ICE_DIR)/slice
endif
"""
		elif reIceLocation.search(line) <> None:
		    print line.rstrip('\n').replace('top_srcdir', 'ICE_DIR', 10)
		elif line.startswith('install_'):
		    #
		    # Do nothing.
		    #
		    pass
		else:
		    print line.rstrip('\n')
	    elif state == 'untilblank':
		if line.isspace():
		    state = 'done'
	    elif state == 'header':
		#
		# Reading header.
		#
		print line.rstrip('\n')
		if line.strip() == "":
		    state = 'untilprefix'
		    print """
#
# Checks for ICE_HOME environment variable.  If it isn't present it will
# attempt to find an Ice installation in /usr.
#

ifeq ($(ICE_HOME),)
    ICE_DIR = /usr
else
    ICE_DIR = $(ICE_HOME)
endif

ifneq ($(shell test -d $(ICE_DIR)/include/Ice && echo 0),0)
    $(error Ice distribution not found, please set ICE_HOME!)
endif
"""
	    elif state == 'untilprefix':
		if line.startswith('prefix'):
		    state = 'done'

        # Dependency files are all going to be bogus.  The makedepend
        # script doesn't seem to work properly for the slice files.
        os.chdir("..")
        os.system("sh -c 'for f in `find . -name .depend` ; do echo \"\" > $f ; done'")
        
        os.chdir(tcwd)
    elif demoDir == "j":
        tcwd = os.getcwd()
        os.chdir(buildDir + "/Ice-" + version + "-demos/config")
	for line in fileinput.input('common.xml', True, ".bak"):
	    print line.rstrip('\n').replace('ICE_VERSION', version)
        os.chdir(tcwd)
        
    shutil.rmtree(buildDir + "/demotree/" + distro, True)
    os.chdir(cwd)

def archiveDemoTree(buildDir, version):
    cwd = os.getcwd()
    os.chdir(buildDir)
    
    #
    # Remove unnecessary files from demos here.
    #
    os.remove("Ice-" + version + "-demos/config/TestUtil.py")
    os.remove("Ice-" + version + "-demos/config/IcePackAdmin.py")

    # 
    # Remove compiled Java.
    # 
    os.system("sh -c 'for f in `find Ice-" + version + "-demos/demoj -name classes -type d` ; do rm -rf $f/* ; done'")

    #
    # Remove generated source files.
    #
    os.system("sh -c 'for f in `find Ice-" + version + "-demos/demoj -name generated -type d` ; do rm -rf $f/* ; done'")

    #
    # Remove Windows project files.
    #
    os.system("sh -c 'for f in `find Ice-" + version + "-demos -regex \".*\.ds[wp]\" ` ; do rm -rf $f ; done'")
    os.system("sh -c 'for f in `find Ice-" + version + "-demos/democs -name \"*.sln\" ` ; do rm -rf $f ; done'")
    os.system("sh -c 'for f in `find Ice-" + version + "-demos/democs -name \"*.csproj\" ` ; do rm -rf $f ; done'")

    os.system("tar cvf Ice-" + version + "-demos.tar Ice-" + version + "-demos")
    os.system("gzip -9 Ice-" + version + "-demos.tar")
    os.chdir(cwd)

def makeInstall(sources, buildDir, installDir, distro, clean):
    """Make the distro in buildDir sources and install it to installDir."""
    cwd = os.getcwd()
    os.chdir(buildDir)
    if clean:
        shutil.rmtree(distro, True)
        
    if not os.path.exists(distro):
        os.system("gzip -dc " + sources + "/" + distro + ".tar.gz | tar xf -")
        
    os.chdir(distro)

    #
    # Java does not have a 'make install' process, but comes complete with the Jar 
    # already built.
    # 
    if distro.startswith("IceJ"):
        shutil.copy(buildDir + "/" + distro + "/lib/Ice.jar", installDir + "/lib")
	#
	# We really just want to copy the files, not move them.
	# Shelling out to a copy is easier (and more likely to always
	# work) than shutil.copytree().
	#
	os.system("cp -pR " + buildDir + "/" + distro + "/ant " + installDir)
        os.chdir(cwd)
        return

    if distro.startswith("IceCS"):
	os.system("perl -pi -e 's/^prefix.*$/prefix = \$\(INSTALL_ROOT\)/' config/Make.rules.cs")
    else:
	os.system("perl -pi -e 's/^prefix.*$/prefix = \$\(INSTALL_ROOT\)/' config/Make.rules")

    if distro.startswith("IcePy"):
        try:
            pyHome = os.environ["PYTHON_HOME"]
        except KeyError:
            pyHome = None
            
        if pyHome == None or pyHome == "":
            logging.info("PYTHON_HOME is not set, figuring it out and trying that")
            pyHome = sys.exec_prefix
            
        os.system("perl -pi -e 's/^PYTHON.HOME.*$/PYTHON\_HOME \?= "+ pyHome.replace("/", "\/") + \
		"/' config/Make.rules")

    if getPlatform() <> "linux":
	if distro.startswith("IcePy"):
	    os.system("perl -pi -e 's/^PYTHON.INCLUDE.DIR.*$/PYTHON_INCLUDE_DIR = \$\(PYTHON_HOME\)\/include\/\$\(PYTHON_VERSION\)/' config/Make.rules")
	    os.system("perl -pi -e 's/^PYTHON.LIB.DIR.*$/PYTHON_LIB_DIR = \$\(PYTHON_HOME\)\/lib\/\$\(PYTHON_VERSION\)\/config/' config/Make.rules")

    os.system("gmake NOGAC=yes OPTIMIZE=yes INSTALL_ROOT=" + installDir + " install")
    os.chdir(cwd)
    
def shlibExtensions(versionString, versionInt):
    """Returns a tuple containing the extensions for the shared library, and the 2 symbolic links (respectively)"""
    platform = getPlatform()
    if platform == "hpux":
        return [".sl." + versionString, ".sl." + versionInt, ".sl"]
    elif platform == "macosx":
        return ["." + versionString + ".dylib", "." + versionInt + ".dylib", ".dylib"]
    else:
        return [".so." + versionString, ".so." + versionInt, ".so"]

def strip(files):
    stripCmd = "strip "
    if getPlatform() == "macosx":
        stripCmd = stripCmd + "-x "
    for f in files:
	if not f.endswith(".dll"):
	    print "Stripping " + f
	    os.system(stripCmd + f)

def usage():
    """Print usage/help information"""
    print "Usage: " + sys.argv[0] + " [options] [tag]"
    print
    print "Options:"
    print "-h                     Show this message."
    print "--build-dir=[path]     Specify the directory where the distributions"
    print "                       will be unpacked and built."
    print "--install-dir=[path]   Specify the directory where the distribution"
    print "                       contents will be installed to."
    print "--install-root=[path]  Specify the root directory that will appear"
    print "                       in the tarball."
    print "--sources=[path]       Specify the location of the sources directory."
    print "                       If this is omitted makebindist will traverse"
    print "                       ../icej ../icepy ../icecs, etc and make the"
    print "                       distributions for you."
    print "-v, --verbose          Print verbose processing messages."
    print "-t, --tag              Specify the CVS version tag for the packages."
    print "--noclean              Do not clean up current sources where"
    print "                       applicable (some bits will still be cleaned."
    print "--nobuild              Run through the process but don't build"
    print "                       anything new."
    print "--specfile             Just print the RPM spec file and exit."
    print "--usecvs		  Use contents of existing CVS directories"
    print "                       to create binary package (This option cannot"
    print "                       be used to create RPMS)"
    print 
    print "The following options set the locations for third party libraries"
    print "that may be required on your platform.  Alternatively, you can"
    print "set these locations using environment variables of the form.  If"
    print "you do not set locations through the enviroment or through the "
    print "command line, default locations will be used (system defaults +"
    print "the default locations indicated)."
    print 
    print "LIBRARY_HOME=[path to library]"
    print
    print "e.g. for bzip2"
    print 
    print "export BZIP2_HOME=/opt/bzip2-1.0.3"
    print 
    print "--stlporthome=[path]   Specify location of the STLPort libraries, "
    print "                       if required."
    print "--bzip2home=[path]     Specify location of the bzip2 libraries "
    print "                       (default=/opt/bzip2)."
    print "--dbhome=[path]        Specify location of Berkeley DB"
    print "                       (default=/opt/db)."
    print "--sslhome=[path]       Specify location of OpenSSL"
    print "                       (default=/opt/openssl)."
    print "--expathome=[path]	  Specify location of expat libraries "
    print "                       (default=/opt/expat)."
    print "--readlinehome=[path]  Specify readline library and location "
    print "                       (defaults to /opt/readline if set)."
    print
    print "If no tag is specified, HEAD is used."

def main():

    buildEnvironment = dict()
    buildDir = None
    installDir = None
    sources = None
    installRoot = None
    verbose = False
    cvsTag = "HEAD"
    clean = True
    build = True
    version = None
    soVersion = 0
    printSpecFile = False
    verbose = False
    cvsMode = False    # Use CVS directories.

    #
    # Process args.
    #
    try:
        optionList, args = getopt.getopt(sys.argv[1:], "hvt:",
                                         [ "build-dir=", "install-dir=", "install-root=", "sources=",
                                           "verbose", "tag=", "noclean", "nobuild", "specfile",
					   "stlporthome=", "bzip2home=", "dbhome=", "sslhome=",
					   "expathome=", "readlinehome=", "usecvs"])
               
    except getopt.GetoptError:
        usage()
        sys.exit(2)

    for o, a in optionList:
        if o == "--build-dir":
            buildDir = a
        elif o == "--install-dir":
            installDir = a
        elif o == "--install-root":
            installRoot = a
        elif o == "--sources":
            sources = a
        elif o in ("-h", "--help"):
            usage()
            sys.exit()
        elif o in ("-v", "--verbose"):
            verbose = True
        elif o in ("-t", "--tag"):
            cvsTag = a
        elif o == "--noclean":
            clean = False
        elif o == "--nobuild":
            build = False
        elif o == "--specfile":
            printSpecFile = True
	elif o == "--stlporthome":
	    buildEnvironment['STLPORT_HOME'] = a
	elif o == "--bzip2home":
	    buildEnvironment['BZIP2_HOME'] = a
	elif o == "--dbhome":
	    buildEnvironment['DB_HOME'] = a
	elif o == "--sslhome":
	    buildEnvironment['OPENSSL_HOME'] = a
	elif o == "--expathome":
	    buildEnvironment['EXPAT_HOME'] = a
	elif o == "--readlinehome":
	    buildEnvironment['READLINE_HOME'] = a
	elif o == "--usecvs":
	    cvsMode = True

    if verbose:
	logging.getLogger().setLevel(logging.DEBUG)

    #
    # Configure environment.
    #
    if getPlatform() == "aix":
	dylibEnvironmentVar = 'LIBPATH'
    elif getPlatform() == "hpux":
	dylibEnvironmentVar = 'SHLIB_PATH'
    else:
	dylibEnvironmentVar = 'LD_LIBRARY_PATH'

    for k, v in buildEnvironment.iteritems():
	os.environ[k] = v
	if os.environ.has_key(dylibEnvironmentVar):
	    os.environ[dylibEnvironmentVar] = v + "/lib:" + os.environ[dylibEnvironmentVar] 

    if buildDir == None:
        print "No build directory specified, defaulting to $HOME/tmp/icebuild"
        buildDir = os.environ.get('HOME') + "/tmp/icebuild"

    if cvsMode:
	print "Using CVS mode"

    if installDir == None:
        print "No install directory specified, default to $HOME/tmp/iceinstall"
        installDir = os.environ.get('HOME') + "/tmp/iceinstall"

    #
    # We need to clean the directory out to keep obsolete files from
    # being installed.  This needs to happen whether we are running with
    # noclean or not.
    #
    if build:
        if os.path.exists(installDir):
            shutil.rmtree(installDir, True)

    #
    # In CVS mode we are relying on the checked out CVS sources *are* the build sources.
    #
    if cvsMode:
	directories = []
    else:
	directories = [buildDir, buildDir + "/sources", buildDir + "/demotree"]

    directories.append(installDir)

    for d in directories:
        initDirectory(d)

    if cvsMode:
	version = getIceVersion("include/IceUtil/Config.h")
	soVersion = getIceSoVersion("include/IceUtil/Config.h")
    else:
	version, soVersion = getVersion(cvsTag, buildDir)

    if verbose:
        print "Building binary distributions for Ice-" + version + " on " + getPlatform()
        print "Using build directory: " + buildDir
        print "Using install directory: " + installDir
        if getPlatform() == "linux":
            print "(RPMs will be built)"
        print

    #
    # Primarily for debugging spec file creation.
    #
    if printSpecFile:
	createRPMSFromBinaries(sys.stdout, installDir, version, soVersion)
        sys.exit(0)

    if not cvsMode:
	#
	# These last build directories will have to wait until we've got the version number for the distribution.
	#
	shutil.rmtree(buildDir + "/Ice-" + version + "-demos", True)
	initDirectory(buildDir + "/Ice-" + version + "-demos/config")

    if build and not cvsMode:
        collectSources = False
        if sources == None:
            sources = buildDir + "/sources"
            collectSources = clean

        #
        # Ice must be first or building the other source distributions will fail.
        #
        sourceTarBalls = [ ("ice", "Ice-" + version, ""),
                           ("icej","IceJ-" + version, "j") ]

	if getPlatform() <> "aix":
	    sourceTarBalls.append(("icepy","IcePy-" + version, "py"))

	if getPlatform() == "linux":
	    sourceTarBalls.append(("icecs","IceCS-" + version, "cs"))

        os.environ['ICE_HOME'] = installDir + "/Ice-" + version
        currentLibraryPath = None
        try:
            currentLibraryPath = os.environ[dylibEnvironmentVar] 
        except KeyError:
            currentLibraryPath = ""

        os.environ[dylibEnvironmentVar] = installDir + "/Ice-" + version + "/lib:" + currentLibraryPath
        os.environ['PATH'] = installDir + "/Ice-" + version + "/bin:" + os.environ['PATH']

        for cvs, tarball, demoDir in sourceTarBalls:
            if collectSources:
                collectSourceDistributions(cvsTag, sources, cvs, tarball)

            extractDemos(sources, buildDir, version, tarball, demoDir)
            makeInstall(sources, buildDir, installDir + "/Ice-" + version, tarball, clean)

        #
        # Pack up demos
        #
        archiveDemoTree(buildDir, version)
        shutil.move(buildDir + "/Ice-" + version + "-demos.tar.gz", installDir + "/Ice-" + version + "-demos.tar.gz")

    elif cvsMode:
	collectSources = False

	#
	# TODO: Sanity check to make sure that the script is being run
	# from a location that it expects.
	#
	cvsDirs = [ "ice", "icej", "icepy" ]
	if getPlatform() == "linux":
	    cvsDirs.append("icecs")

        os.environ['ICE_HOME'] = os.getcwd()  
        currentLibraryPath = None
        try:
            currentLibraryPath = os.environ[dylibEnvironmentVar] 
        except KeyError:
            currentLibraryPath = ""

        os.environ[dylibEnvironmentVar] = installDir + "/Ice-" + version + "/lib:" + currentLibraryPath
        os.environ['PATH'] = installDir + "/Ice-" + version + "/bin:" + os.environ['PATH']

	for d in cvsDirs:
	    currentDir = os.getcwd()
	    os.chdir("../" + d)
	    print "Going to directory " + d
	    if d == "icej":
		shutil.copy("lib/Ice.jar", installDir +"/Ice-" + version + "/lib")
		os.system("cp -pR ant " + installDir + "/Ice-" + version)
	    else:
		os.system("perl -pi -e 's/^prefix.*$/prefix = \$\(INSTALL_ROOT\)/' config/Make.rules")
		os.system("gmake INSTALL_ROOT=" + installDir + "/Ice-" + version + " install")
	    os.chdir(currentDir)

    #
    # Sources should have already been built and installed.  We
    # can pick the binaries up from the iceinstall directory.
    #
    binaries = glob.glob(installDir + "/Ice-" + version + "/bin/*")
    binaries.extend(glob.glob(installDir + "/Ice-" + version + "/lib/*" + shlibExtensions(version, soVersion)[0]))
    strip(binaries)
    cwd = os.getcwd()
    os.chdir(installDir)
    os.system("tar cvf Ice-" + version + "-bin-" + getPlatform() + ".tar Ice-" + version)
    os.system("gzip -9 Ice-" + version + "-bin-" + getPlatform() + ".tar")
    os.system("zip -9ry Ice-" + version + "-bin-" + getPlatform() + ".zip Ice-" + version)
    os.chdir(cwd)

    #
    # If we are running on Linux, we need to create RPMs.  This will probably blow up unless the user
    # that is running the script has massaged the permissions on /usr/src/redhat/.
    #
    if getPlatform() == "linux" and not cvsMode:
	RPMTools.createRPMSFromBinaries(buildDir, installDir, version, soVersion)

    #
    # TODO: Cleanups?  I've left everything in place so that the process can be easily debugged.
    #

if __name__ == "__main__":
    main()
