#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, shutil, fnmatch, re, string, getopt, glob

#
# defaults.
#
verbose = False

#
# fileLists is a mapping of all of the files that will go into a binary distribution.
#
fileLists = dict()

class Subpackage:
    """Encapsulates RPM spec file information to be used to generate a spec file on Linux
       and create RPMs for Ice"""
    def __init__(self, name, summary, group, description, filelist):
        self.name = name
        self.summary = summary
        self.group = group
        self.description = description
        self.filelist = filelist

    def writeHdr(self, ofile):
        ofile.write("%package " + self.name + "\n")
        ofile.write("Summary: " + self.summary + "\n")
        ofile.write("Group: " + self.group + "\n")
        ofile.write("%description " + self.name + "\n")
        ofile.write(self.description)

    def writeFiles(self, ofile, version, intVersion):
        ofile.write("%files " + self.name + "\n")
        ofile.write("%defattr(-, root, root, -)\n\n")
        for f in self.filelist:
            if f.endswith(".so"):
                ofile.write("/usr/" + f + "." + version + "\n")
                ofile.write("/usr/" + f + "." + str(intVersion) + "\n")
            ofile.write("/usr/" + f + "\n")
        ofile.write("\n")
                    
fileLists['core'] = Subpackage("core",
                               "The core Ice libraries and tools.",
                               "Development/Libraries Development/Tools System Environment/Libraries",
                               "",
                               ["ICE_LICENSE",
                                "LICENSE",
                                "bin/dumpdb",
                                "bin/transformdb",
                                "bin/glacier2router",
                                "bin/icebox",
                                "bin/iceboxadmin",
                                "bin/icepackadmin",
                                "bin/icecpp",
                                "bin/icepacknode",
                                "bin/icepackregistry",
                                "bin/icepatch2calc",
                                "bin/icepatch2client",
                                "bin/icepatch2server",
                                "bin/icestormadmin",
                                "bin/slice2docbook", 
                                "lib/libFreeze.so",
                                "lib/libGlacier2.so",
                                "lib/libIceBox.so",
                                "lib/libIcePack.so",
                                "lib/libIcePatch2.so",
                                "lib/libIce.so",
                                "lib/libIceSSL.so",
                                "lib/libIceStormService.so",
                                "lib/libIceStorm.so",
                                "lib/libIceUtil.so",
                                "lib/libIceXML.so",
                                "lib/libSlice.so",
                                "lib/glacier2cs.dll",
                                "lib/icecs.dll",
                                "lib/icepackcs.dll",
                                "lib/Ice.jar",
                                "lib/IcePy.so",
                                "slice"])

fileLists['c++'] = Subpackage("c++",
                              "Ice tools, files and libraries for developing Ice applications in C++",
                              "Development/Libraries Development/Tools",
                              "",
                              ["ICE_LICENSE",
                               "LICENSE",
                               "bin/slice2cpp",
                               "bin/slice2freeze",
                               "include"])

fileLists['cs'] = Subpackage("cs",
                             "Ice tools, files and libraries for developing Ice applications in C\#",
                             "Development/Libraries Development/Tools",
                             "",
                             ["ICE_LICENSE",
                              "LICENSE",
                              "bin/slice2cs"])

fileLists['java'] = Subpackage("java",
                               "Ice tools, files and libraries for developing Ice applications in Java",
                               "Development/Libraries Development/Tools",
                               "",
                               ["ICE_LICENSE",
                                "LICENSE",
                                "bin/slice2java",
                                "bin/slice2freezej"])

fileLists['py'] = Subpackage("py",
                             "Ice tools, files and libraries for developing Ice applications in Python",
                             "Development/Libraries Development/Tools",
                             "",
                             ["ICE_LICENSE",
                              "LICENSE",
                              "bin/slice2py",
                              "python"])

fileLists['vb'] = Subpackage("vb",
                             "Ice tools, files and libraries for developing Ice client applications using vb.",
                             "Development/Libraries Development/Tools",
                             "",
                             ["ICE_LICENSE",
                              "LICENSE",
                              "bin/slice2vb"])

fileLists['docs'] = Subpackage("docs",
                               "Browsable documentation for Ice",
                               "Development/Libraries Development/Tools",
                               "",
                               ["ICE_LICENSE",
                                "LICENSE",
                                "doc"])

def usage():
    """Print usage/help information"""
    print "Usage: " + sys.argv[0] + " [options] [tag]"
    print
    print "Options:"
    print "-h    Show this message."
    print "--build-dir     Specify the directory where the distributions will be unpacked and built."
    print "--install-dir   Specify the directory where the distribution contents will be installed to."
    print "--install-root  Specify the root directory that will appear in the tarball."
    print "--sources       Specify the location of the sources directory.  If this is omitted makebindist"
    print "                will traverse ../icej ../icepy ../icecs, etc and make the distributions for you."
    print
    print "If no tag is specified, HEAD is used."

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

def trace(s):
    if verbose:
        print s
    
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
            trace("Path " + d + " exists and is ok, continuing")
        else:
            print "Directory " + d + " exists, but has incorrect permissions"
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
        os.system("cvs -d cvsint.mutablerealms.com:/home/cvsroot export -r " + cvsTag + " ice/include/IceUtil/Config.h")
    else:
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
    if cvsdir == "icepy":
        os.system("./makedist.py " + tag)
    else:
        os.system("./makedist.py -d " + tag)
    shutil.copy("dist/" + distro + ".tar.gz", sourceDir)
    os.chdir(cwd)

def extractDemos(buildDir, version, distro, demoDir):
    """Pulls the demo directory out of a distribution and massages its
       build system so it can be built against an installed version of
       Ice"""
    cwd = os.getcwd()
    os.chdir(buildDir + "/demotree")
    os.system("tar xvfz ../sources/" + distro + ".tar.gz " + distro + "/demo " + distro + "/config")
    shutil.move(distro + "/demo", buildDir + "/Ice-" + version + "-demos/" + demoDir)
    os.system("cp " + distro + "/config/*.* " + buildDir + "/Ice-" + version + "-demos/config")
    shutil.rmtree(buildDir + "/demotree/" + distro, True)
    os.chdir(cwd)

def archiveDemoTree(buildDir, version):
    cwd = os.getcwd()
    os.chdir(buildDir)
    os.system("tar cvfz Ice-" + version + "-demos.tar.gz Ice-" + version + "-demos")
    os.chdir(cwd)

def makeInstall(buildDir, installDir, distro, clean):
    """Make the distro in buildDir sources and install it to installDir."""
    cwd = os.getcwd()
    os.chdir(buildDir)
    if clean:
        shutil.rmtree(distro, True)
        
    if not os.path.exists(distro):
        os.system("tar xfvz sources/" + distro + ".tar.gz")
        
    os.chdir(distro)

    if distro.startswith("IceJ"):
        shutil.copy( buildDir + "/" + distro + "/lib/Ice.jar", installDir + "/lib")
        os.chdir(cwd)
        return

    os.system("perl -pi -e 's/^prefix.*$/prefix = \$\(RPM_BUILD_ROOT\)/' config/Make.rules")

    if distro.startswith("IcePy"):
        try:
            pyHome = os.environ["PYTHON_HOME"]
        except KeyError:
            pyHome = None
            
        if pyHome == None or pyHome == "":
            trace("PYTHON_HOME is not set, figuring it out and trying that")
            pyHome = sys.exec_prefix
            
        os.system("perl -pi -e 's/^PYTHON.HOME.*$/PYTHON\_HOME \?= "+ pyHome.replace("/", "\/") + "/' config/Make.rules")
    os.system("gmake RPM_BUILD_ROOT=" + installDir + " install")
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
        print "Stripping " + f
        os.system(stripCmd + f)

def printRPMHeader(ofile, version, release, installDir):
    """Used on Linux only.  Prints out the header portion of an RPM spec file for building the RPM"""
    ofile.write("%define _unpackaged_files_terminate_build 0\n")
    ofile.write("Summary: The Internet Communications Engine (ICE) is a modern alternative to object middleware such ")
    ofile.write("as CORBA\n")
    ofile.write("Name: Ice\n")
    ofile.write("Version: " + version + "\n")
    ofile.write("Release: " + release + "\n")
    ofile.write("License: GPL\n")
    ofile.write("""Group: Development/Libraries
Vendor: ZeroC Inc
URL: http://www.zeroc.com/index.html
Source0: http://www.zeroc.com/downloads/%{name}-%{version}.tar.gz
Source1: http://www.zeroc.com/downloads/%{name}J-%{version}.tar.gz
Source2: http://www.zeroc.com/downloads/%{name}Py-%{version}.tar.gz
Source3: http://www.zeroc.com/downloads/%{name}CS-%{version}.tar.gz
""")
    ofile.write("BuildRoot: " + installDir + "\n")
    ofile.write("Prefix: /usr\n")
    ofile.write("""
%description

%prep

%build

%install

%clean


""")

def main():
    try:
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
        buildDemos = True
        printSpecFile = False
        
        optionList, args = getopt.getopt(sys.argv[1:], "hvt:",
                                         [ "build-dir=", "install-dir=", "install-root=", "sources=",
                                           "verbose", "tag=", "noclean", "nobuild", "nodemos", "specfile"])
               
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
        elif o == "--nodemos":
            buildDemos = False
        elif o == "--specfile":
            printSpecFile = True

    if buildDir == None:
        trace("No build directory specified, defaulting to $HOME/tmp/icebuild")
        buildDir = os.environ.get('HOME') + "/tmp/icebuild"

    if installDir == None:
        trace("No install directory specified, default to $HOME/tmp/iceinstall")
        installDir = os.environ.get('HOME') + "/tmp/iceinstall"

    if build:
        #
        # We need to clean the directory out to keep obsolete files from being installed.
        #
        if os.path.exists(installDir):
            shutil.rmtree(installDir, True)

    directories = [buildDir, buildDir + "/sources", buildDir + "/demotree",  installDir]
    for d in directories:
        initDirectory(d)

    version, soVersion = getVersion(cvsTag, buildDir)

    #
    # Primarily for debugging spec file creation.
    #
    if printSpecFile:
        ofile = sys.stdout
        printRPMHeader(ofile, version, "1", installDir)
        for k, v in fileLists.iteritems():
            v.writeHdr(ofile)
            ofile.write("\n\n\n")
        for k, v in fileLists.iteritems():
            v.writeFiles(ofile, version, soVersion)
            ofile.write("\n")
        sys.exit(0)


    #
    # This last directory we have to wait until we've got the version number for the distribution.
    #
    shutil.rmtree(buildDir + "/Ice-" + version + "-demos", True)
    initDirectory(buildDir + "/Ice-" + version + "-demos/config")

    if build:
        collectSources = False
        if sources == None:
            sources = buildDir + "/sources"
            collectSources = True

        #
        # Ice must be first or building the other source distributions will fail.
        #
        sourceTarBalls = [ ("ice", "Ice-" + version, "demo_cpp"),
                           ("icej","IceJ-" + version, "demo_java"),
                           ("icecs","IceCS-" + version, "demo_cs"),
                           ("icepy","IcePy-" + version, "demo_py") ]

        os.environ['ICE_HOME'] = installDir + "/Ice-" + version
        currentLibraryPath = None
        try:
            currentLibraryPath = os.environ['LD_LIBRARY_PATH'] 
        except KeyError:
            currentLibraryPath = ""

        os.environ['LD_LIBRARY_PATH'] = installDir + "/Ice-" + version + "/lib:" + currentLibraryPath
        os.environ['PATH'] = installDir + "/Ice-" + version + "/bin:" + os.environ['PATH']

        for cvs, tarball, demoDir in sourceTarBalls:
            if collectSources:
                collectSourceDistributions(cvsTag, sources, cvs, tarball)
            extractDemos(buildDir, version, tarball, demoDir)
            makeInstall(buildDir, installDir + "/Ice-" + version, tarball, clean)

        archiveDemoTree(buildDir, version)

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
    if getPlatform() == "linux":
        ofile = open(buildDir + "/Ice-" + version + ".spec", "w")
        printRPMHeader(ofile, version, "1", installDir)
        for k, v in fileLists.iteritems():
            v.writeHdr(ofile)
            ofile.write("\n\n\n")
        for k, v in fileLists.iteritems():
            v.writeFiles(ofile, version, soVersion)
            ofile.write("\n")
        shutil.move(installDir + "/Ice-" + version, installDir + "/usr")
        cwd = os.getcwd()
        os.chdir(buildDir)
        ofile.flush()
        ofile.close()
        os.system("rpmbuild -bb Ice-" + version + ".spec")

    #
    # TODO: Cleanups?  I've left everything in place so that the process can be easily debugged.
    #


if __name__ == "__main__":
    main()
