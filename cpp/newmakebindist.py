#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************
import os, sys, shutil, re, string, getopt, glob, logging

#
# TODO:
#
#  * Tidying and tracing.
#  * Testing on platforms besides Linux.
#
# NOTES:
#  There are python-ese ways to do some of the things I've shelled out to do.
#  We might want to convert some of these things, but a lot of things can be
#  done with one line in a shell.
#

#
# defaults.
#
verbose = False

#
# Represents the 'main' package of an RPM spec file.
# TODO: The package should really write the RPM header has well.
#
class Package:
    """Encapsulates RPM spec file information to be used to generate a spec file on Linux
       and create RPMs for Ice"""
    def __init__(self, name, requires, summary, group, description, other, filelist):
        self.name = name
        self.requires = requires
        self.summary = summary
        self.group = group
        self.description = description
        self.filelist = filelist
	self.other = other
        
    def writeHdr(self, ofile, version, release, installDir):
        ofile.write("\n")

	ofile.write("%define _unpackaged_files_terminate_build 0\n")
	ofile.write("Summary: " + self.summary + "\n")
	ofile.write("Name: " + self.name + "\n")
	ofile.write("Version: " + version + "\n")
	ofile.write("Release: " + release + "\n")
	if self.requires <> "":
	    ofile.write("Requires: " + self.requires + "\n")
	ofile.write("License: GPL\n")
	ofile.write("Group: Development/Libraries\n")
	ofile.write("Vendor: ZeroC Inc\n")
	ofile.write("URL: http://www.zeroc.com/index.html\n")
	ofile.write("Source0: http://www.zeroc.com/downloads/%{name}-%{version}.tar.gz\n")
	ofile.write("Source1: http://www.zeroc.com/downloads/%{name}J-%{version}.tar.gz\n")
	ofile.write("Source2: http://www.zeroc.com/downloads/%{name}Py-%{version}.tar.gz\n")
	ofile.write("Source3: http://www.zeroc.com/downloads/%{name}CS-%{version}.tar.gz\n")
	ofile.write("\n")
	ofile.write("BuildRoot: " + installDir + "\n")
	ofile.write("Prefix: /usr\n")
	ofile.write("\n")
	ofile.write("%description\n")
	ofile.write("\n")
	ofile.write("%prep\n")
	ofile.write("\n")
	ofile.write("%build\n")
	ofile.write("\n")
	ofile.write("%install\n")
	ofile.write("\n")
	ofile.write("%clean\n")
	ofile.write("\n")
	ofile.write("\n")

    def writeFileList(self, ofile, version, intVersion):
        ofile.write("%defattr(644, root, root, 755)\n\n")
        for perm, f in self.filelist:
            prefix = ""
	    
	    #
	    # Select an RPM spec file attribute depending on the type of
	    # file or directory we've specified.
	    #
            if perm == "exe" or perm == "lib":
                prefix = "%attr(755, root, root) "
	    elif perm == "xdir":
		prefix = "%dir "

            if f.find("%version%"):
                f = f.replace("%version%", version)

            if perm == "lib" and f.endswith(".VERSION"):
		fname = os.path.splitext(f)[0]
                ofile.write(prefix + "/usr/" + fname + "." + version + "\n")
                ofile.write(prefix + "/usr/" + fname + "." + str(intVersion) + "\n")
	    else:
		ofile.write(prefix + "/usr/" + f + "\n")

        ofile.write("\n")    

    def writeFiles(self, ofile, version, intVersion):
        ofile.write("%files\n")
        self.writeFileList(ofile, version, intVersion)

#
# Represents subpackages in an RPM spec file.
#
class Subpackage(Package):
    def writeHdr(self, ofile, version, release, installDir):
        ofile.write("%package " + self.name + "\n")
        ofile.write("Summary: " + self.summary + "\n")
        ofile.write("Group: " + self.group + "\n")
        if not self.requires == "":
            ofile.write("Requires: " + self.requires + "\n")
	if not self.other == "":
	    ofile.write(self.other + "\n")
        ofile.write("%description " + self.name + "\n")
        ofile.write(self.description)

    def writeFiles(self, ofile, version, intVersion):
        ofile.write("%files " + self.name + "\n")
        self.writeFileList(ofile, version, intVersion)

#
# NOTE: File transforms should be listed before directory transforms.
#
transforms = [ ("lib/Ice.jar", "lib/Ice-%version%/Ice.jar" ),
	       ("slice", "share/slice"),
	       ("ant", "lib/Ice-%version%/ant"),
	       ("python", "lib/Ice-%version%/python"),
               ("doc", "share/doc/Ice-%version%"),
               ("ICE_LICENSE", "share/doc/Ice-%version%/ICE_LICENSE"),
               ("LICENSE", "share/doc/Ice-%version%/LICENSE")
               ]

#
# fileLists is an in-memory representation of the package contents of
# the Ice spec file.
# 
fileLists = [
    Package("ice",
            "",
	    "The Internet Communications Engine (ICE) is a modern alternative to object middleware",
            "Development/Libraries Development/Tools System Environment/Libraries",
            "",
	    "",
            [("doc", "share/doc/Ice-%version%/ICE_LICENSE"),
             ("doc", "share/doc/Ice-%version%/LICENSE"),
             ("exe", "bin/dumpdb"),
             ("exe", "bin/transformdb"),
             ("exe", "bin/glacier2router"),
             ("exe", "bin/icebox"),
             ("exe", "bin/iceboxadmin"),
             ("exe", "bin/icepackadmin"),
             ("exe", "bin/icecpp"),
             ("exe", "bin/icepacknode"),
             ("exe", "bin/icepackregistry"),
             ("exe", "bin/icepatch2calc"),
             ("exe", "bin/icepatch2client"),
             ("exe", "bin/icepatch2server"),
             ("exe", "bin/icestormadmin"),
             ("exe", "bin/slice2docbook"), 
             ("lib", "lib/libFreeze.so.VERSION"),
             ("lib", "lib/libGlacier2.so.VERSION"),
             ("lib", "lib/libIceBox.so.VERSION"),
             ("lib", "lib/libIcePack.so.VERSION"),
             ("lib", "lib/libIcePatch2.so.VERSION"),
             ("lib", "lib/libIce.so.VERSION"),
             ("lib", "lib/libIceSSL.so.VERSION"),
             ("lib", "lib/libIceStormService.so.VERSION"),
             ("lib", "lib/libIceStorm.so.VERSION"),
             ("lib", "lib/libIceUtil.so.VERSION"),
             ("lib", "lib/libIceXML.so.VERSION"),
             ("lib", "lib/libSlice.so.VERSION"),
             ("dir", "share/slice"),
             ("dir", "share/doc/Ice-%version%/images"),
             ("dir", "share/doc/Ice-%version%/manual"),
	     ("xdir", "share/doc/Ice-%version%/config"),
	     ("file", "share/doc/Ice-%version%/config/Make.rules"),
	     ("xdir", "share/doc/Ice-%version%/certs")]),
    Subpackage("c++-devel",
               "",
               "Ice tools, files and libraries for developing Ice applications in C++",
               "Development/Libraries Development/Tools",
               "",
	       "",
               [("exe", "bin/slice2cpp"),
                ("exe", "bin/slice2freeze"),
                ("dir", "include"),
		("lib", "lib/libFreeze.so"),
		("lib", "lib/libGlacier2.so"),
		("lib", "lib/libIceBox.so"),
		("lib", "lib/libIcePack.so"),
		("lib", "lib/libIcePatch2.so"),
		("lib", "lib/libIce.so"),
		("lib", "lib/libIceSSL.so"),
		("lib", "lib/libIceStormService.so"),
		("lib", "lib/libIceStorm.so"),
		("lib", "lib/libIceUtil.so"),
		("lib", "lib/libIceXML.so"),
		("lib", "lib/libSlice.so"),
		("dir", "share/doc/Ice-%version%/demo"),
		("file", "share/doc/Ice-%version%/config/Make.cxx.rules"),
		("file", "share/doc/Ice-%version%/config/makedepend.py"),
		("file", "share/doc/Ice-%version%/config/makecerts"),
		("file", "share/doc/Ice-%version%/config/makeprops.py"),
		("file", "share/doc/Ice-%version%/config/Make.rules.Linux"),
		("file", "share/doc/Ice-%version%/config/server.cnf"),
		("file", "share/doc/Ice-%version%/config/client.cnf"),
		("file", "share/doc/Ice-%version%/config/generic.cnf"),
		("file", "share/doc/Ice-%version%/config/ice_ca.cnf"),
	        ("file", "share/doc/Ice-%version%/certs/cacert.pem"),
	        ("file", "share/doc/Ice-%version%/certs/c_dh1024.pem"),
	        ("file", "share/doc/Ice-%version%/certs/client_sslconfig.xml"),
	        ("file", "share/doc/Ice-%version%/certs/server_sslconfig.xml"),
	        ("file", "share/doc/Ice-%version%/certs/c_rsa1024_priv.pem"),
	        ("file", "share/doc/Ice-%version%/certs/c_rsa1024_pub.pem"),
	        ("file", "share/doc/Ice-%version%/certs/s_dh1024.pem"),
	        ("file", "share/doc/Ice-%version%/certs/s_rsa1024_priv.pem"),
	        ("file", "share/doc/Ice-%version%/certs/s_rsa1024_pub.pem"),
	        ("file", "share/doc/Ice-%version%/certs/sslconfig.dtd"),
	        ("file", "share/doc/Ice-%version%/certs/sslconfig.xml")
		]),
    Subpackage("csharp",
               "ice mono-core >= 1.0.6",
               "Ice runtime for C\# applications",
               "Development/Libraries Development/Tools",
               "",
	       "",
               [("lib", "lib/glacier2cs.dll"), ("lib", "lib/icecs.dll"), ("lib", "lib/icepackcs.dll"),
                ("lib", "lib/icepatch2cs.dll"), ("lib", "lib/icestormcs.dll")]),
    Subpackage("csharp-devel",
               "ice-csharp mono-devel >= 1.0.6",
               "Ice tools for developing Ice applications in C\#",
               "Development/Libraries Development/Tools",
               "",
	       "",
               [("exe", "bin/slice2cs"),
		("file", "share/doc/Ice-%version%/config/Make.cs.rules"),
	        ("dir", "share/doc/Ice-%version%/democs")]),
    Subpackage("java-devel",
               "ice-java",
               "Ice tools developing Ice applications in Java",
               "Development/Libraries Development/Tools",
               "",
	       "",
               [("exe", "bin/slice2java"),
                ("exe", "bin/slice2freezej"),
		("dir", "lib/Ice-%version%/ant"),
	        ("file", "share/doc/Ice-%version%/certs/certs.jks"),
	        ("file", "share/doc/Ice-%version%/certs/client.jks"),
	        ("file", "share/doc/Ice-%version%/certs/server.jks"),
	        ("file", "share/doc/Ice-%version%/certs/makecerts"),
	        ("dir", "share/doc/Ice-%version%/certs/openssl"),
	        ("dir", "share/doc/Ice-%version%/certs/cpp"),
	        ("file", "share/doc/Ice-%version%/config/build.properties"),
	        ("file", "share/doc/Ice-%version%/config/common.cvs.xml"),
	        ("file", "share/doc/Ice-%version%/config/common.opt.xml"),
	        ("file", "share/doc/Ice-%version%/config/common.rpm.xml"),
	        ("file", "share/doc/Ice-%version%/config/common.src.xml"),
	        ("file", "share/doc/Ice-%version%/config/common.xml"),
		("dir", "share/doc/Ice-%version%/demoj")]),
    Subpackage("python",
               "ice python >= 2.3.4",
               "Ice runtime for Python applications",
               "Development/Libraries",
               "",
	       "",
               [("lib", "lib/IcePy.so"), ("dir", "lib/Ice-%version%/python")]),
    Subpackage("python-devel",
               "ice-python",
               "Ice tools for developing Ice applications in Python",
               "Development/Libraries Development/Tools",
               "",
	       "",
               [("exe", "bin/slice2py"),
	        ("dir", "share/doc/Ice-%version%/demopy")])
    ]

noarchFileList = [
    Package("ice-java",
	    "ice db4-java >= 4.2.52",
	    "The Internet Communications Engine (ICE) is a modern alternative to object middleware",
	    "Development/Libraries",
	    "",
	    "BuildArch: noarch",
	    [("dir", "lib/Ice-%version%/Ice.jar")
	    ])
    ]

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
    if cvsdir == "icepy" or cvsdir == "ice":
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
    os.system("tar xvfz ../sources/" + distro + ".tar.gz " + distro + "/demo " + distro + "/config " \
	    + distro + "/certs")
    shutil.move(distro + "/demo", buildDir + "/Ice-" + version + "-demos/demo" + demoDir)

    #
    # 'System' copying of files here because its just easier!
    # 
    os.system("cp " + distro + "/config/* " + buildDir + "/Ice-" + version + "-demos/config")
    if not os.path.exists(buildDir + "/Ice-" + version + "-demos/certs"):
	os.mkdir(buildDir + "/Ice-" + version + "-demos/certs")

    os.system("cp -aR " + distro + "/certs/* " + buildDir + "/Ice-" + version + "-demos/certs")

    #
    # The following hunks spawn a perl process to do an in-place edit
    # of the root Make.rules file.  The changes remove the reliance on
    # top_srcdir for the location of files that are not in the demo
    # package.  The '\\x24' string is the $ metacharacter.  For some
    # reason the perl interpreter is treating the $ metacharacter
    # differently in the replacement text if grouping and capturing is
    # used.  Some of the regular expressions could probably be written
    # a bit better.
    #

    #
    # C++ specific build modifications.
    #
    if demoDir == "":
        tcwd = os.getcwd()
        os.chdir(buildDir + "/Ice-" + version + "-demos/config")
	shutil.move(os.getcwd() + "/Make.rules", os.getcwd() + "/Make.cxx.rules")
        script = "perl -pi -e 's/^prefix.*$/ifeq (\$(ICE_HOME),)\n   ICE_DIR  \= \/usr\nelse\n"
        script = script + "   ICE_DIR \= \$(ICE_HOME)\n"
        script = script + "endif\n/' Make.cxx.rules"
        os.system(script)

        script = "perl -pi -e 's/^([a-z]*dir.*=)\s*\$\(top_srcdir\)\/([A-Za-z]*)$/$1 \\x24\(ICE_DIR\)\/$2/'" + \
	         " Make.cxx.rules"
        os.system(script)

        script = "perl -pi -e 's/^slicedir.*$/ifeq (\$(ICE_DIR),\/usr)\n    slicedir \= \$(ICE_DIR)\/share\/slice\n"
        script = script + "else\n    slicedir \= \$(ICE_DIR)\/slice\nendif\n/' Make.cxx.rules"
        os.system(script)
        
        # Dependency files are all going to be bogus.  The makedepend
        # script doesn't seem to work properly for the slice files.
        os.chdir("..")
        os.system("sh -c 'for f in `find . -name .depend` ; do echo \"\" > $f ; done'")
        
        os.chdir(tcwd)
    #
    # C# specific build modifications
    #
    elif demoDir == "cs":
        tcwd = os.getcwd()
        os.chdir(buildDir + "/Ice-" + version + "-demos/config")
	shutil.move(os.getcwd() + "/Make.rules", os.getcwd() + "/Make.cs.rules")
        script = "perl -pi -e 's/^slice_home.*$/ifeq (\$(ICE_HOME),)\n   ICE_DIR  \= \/usr\nelse\n"
        script = script + "   ICE_DIR \= \$(ICE_HOME)\n"
        script = script + "endif\n/' Make.cs.rules"
        os.system(script)

        script = "perl -pi -e 's/^((?:lib|bin)dir.*=)\s*\$\(top_srcdir\)\/([A-Za-z]*).*$/$1 \\x24\(ICE_DIR\)\/$2/' "
        script = script + "Make.cs.rules"
        os.system(script)

        
        script = "perl -pi -e 's/^slicedir.*slice_home.*$/ifeq (\$(ICE_DIR),\/usr)\n"
        script = script + "   slicedir \:= \\x24\(ICE_DIR\)\/share\/slice\nelse\n"
        script = script + "   slicedir \:= \\x24\(ICE_DIR\)\/slice\nendif\n/' Make.cs.rules"
        os.system(script)

        # Dependency files are all going to be bogus.  The makedepend
        # script doesn't seem to work properly for the slice files.
        os.chdir("..")
        os.system("sh -c 'for f in `find . -name .depend` ; do echo \"\" > $f ; done'")

        os.chdir(tcwd)
    elif demoDir == "j":
        tcwd = os.getcwd()
        os.chdir(buildDir + "/Ice-" + version + "-demos/config")
	#
	# The RPM configuration is the only one that cares about Ice
	# version numbers.
	#
	os.system("perl -pi -e 's/ICE_VERSION/" + version + "/' common.rpm.xml")
        os.chdir(tcwd)
        
    shutil.rmtree(buildDir + "/demotree/" + distro, True)
    os.chdir(cwd)

def archiveDemoTree(buildDir, version):
    cwd = os.getcwd()
    os.chdir(buildDir)
    ofile = open("Ice-" + version + "-demos/config/Make.rules", "w+")

    #
    # Strictly speaking I dislike this method of writing strings, but in
    # a way its more readable for this kind of output.
    #
    ofile.write("""
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

ifeq ($(findstring democs, $(CURDIR)), democs)
   include $(top_srcdir)/config/Make.cs.rules
else
   include $(top_srcdir)/config/Make.cxx.rules
endif
""")
    ofile.close()
    
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
	os.system("cp -aR " + buildDir + "/" + distro + "/ant " + installDir)
        os.chdir(cwd)
        return

    os.system("perl -pi -e 's/^prefix.*$/prefix = \$\(RPM_BUILD_ROOT\)/' config/Make.rules")

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
    os.system("gmake OPTIMIZE=yes RPM_BUILD_ROOT=" + installDir + " install")
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

def missingPathParts(source, dest):
    print "Calculating :  " + source + " and " + dest
        
    startPath = dest.find(source)

    result = dest[0:startPath]
    #
    # There is no common element, we'll need to create the whole destination tree.
    #
    if startPath == -1:
        result = dest
    #
    # The common element is at the head, but we already know the path doesn't exists
    # so we need to remaining path elements
    #
    elif startPath == 0:
        result = dest
    #
    # If the common element is not at the tail of destination, then we probably
    # need to create the whole path
    #
    elif startPath + len(source) + 1 < len(dest):
        result = dest
        
    print "Making " + result
    return result

def transformDirectories(transforms, version, installDir):
    """Transforms a directory tree that was created with 'make installs' to an RPM friendly
       directory tree.  NOTE, this will not work on all transforms, there are certain
       types of transforms in certain orders that will break it."""
    cwd = os.getcwd()
    os.chdir(installDir + "/Ice-" + version)
    for source, dest in transforms:
        if dest.find("%version%"):
            dest = dest.replace("%version%", version)

        sourcedir = source
        destdir = dest

	if os.path.exists("./tmp"):
	    shutil.rmtree("./tmp")
	try:
	    if not os.path.isdir(sourcedir):
		os.renames(source, dest)
	    else:
		# 
		# This is a special problem.  What this implies is that
		# we are trying to move the contents of a directory into
		# a subdirectory of itself.  The regular shutil.move()
		# won't cut it.
		# 
		if os.path.isdir(sourcedir) and sourcedir.split("/")[0] == destdir.split("/")[0]:
		    os.renames(sourcedir, "./tmp/" + sourcedir)
		    os.renames("./tmp/" + sourcedir, destdir)	
		else:
		    os.renames(source, dest)

	except OSError:
	    print "Exception occurred while trying to transform " + source + " to " + dest
	    raise

    os.chdir(cwd)

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
    print "-v, --verbose   Print verbose processing messages."
    print "-t, --tag       Specify the CVS version tag for the packages."
    print "--noclean       Do not clean up current sources where applicable (some bits will still be cleaned."
    print "--nobuild       Run through the process but don't build anything new."
    print "--specfile      Just print the RPM spec file and exit."
    print
    print "If no tag is specified, HEAD is used."

def main():

    #
    # Process args.
    #
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
        printSpecFile = False
        
        optionList, args = getopt.getopt(sys.argv[1:], "hvt:",
                                         [ "build-dir=", "install-dir=", "install-root=", "sources=",
                                           "verbose", "tag=", "noclean", "nobuild", "specfile"])
               
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

    if buildDir == None:
        logging.info("No build directory specified, defaulting to $HOME/tmp/icebuild")
        buildDir = os.environ.get('HOME') + "/tmp/icebuild"

    if installDir == None:
        logging.info("No install directory specified, default to $HOME/tmp/iceinstall")
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
        ofile = sys.stdout
        for v in fileLists:
            v.writeHdr(ofile, version, "1", installDir)
            ofile.write("\n\n\n")
        for v in fileLists:
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
        if sources == None and clean:
            sources = buildDir + "/sources"
            collectSources = True

        #
        # Ice must be first or building the other source distributions will fail.
        #
        sourceTarBalls = [ ("ice", "Ice-" + version, ""),
                           ("icej","IceJ-" + version, "j"),
                           ("icecs","IceCS-" + version, "cs"),
                           ("icepy","IcePy-" + version, "py") ]

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

        #
        # Pack up demos
        #
        archiveDemoTree(buildDir, version)
        shutil.move(buildDir + "/Ice-" + version + "-demos.tar.gz", installDir + "/Ice-" + version + "-demos.tar.gz")

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
        transformDirectories(transforms, version, installDir)
        ofile = open(buildDir + "/Ice-" + version + ".spec", "w")
        for v in fileLists:
            v.writeHdr(ofile, version, "1", installDir)
            ofile.write("\n\n\n")
        for v in fileLists:
            v.writeFiles(ofile, version, soVersion)
            ofile.write("\n")
	os.system("tar xvfz " + installDir + "/Ice-" + version + "-demos.tar.gz -C " + installDir)
        shutil.move(installDir + "/Ice-" + version, installDir + "/usr")
	#
	# Copy demo files so the RPM spec file can pick them up.
	#
	os.system("cp -aR " + installDir + "/Ice-" + version + "-demos/* " + installDir + "/usr/share/doc/Ice-" + version)
	shutil.rmtree(installDir + "/Ice-" + version + "-demos")
        cwd = os.getcwd()
        os.chdir(buildDir)
        ofile.flush()
        ofile.close()
        os.system("rpmbuild -bb Ice-" + version + ".spec")

	# 
	# Build noarch RPMs
	#
	ofile = open(buildDir + "/IceJ-" + version + ".spec", "w")
	for v in noarchFileList:
            v.writeHdr(ofile, version, "1", installDir)
	    ofile.write("\n\n\n")
	for v in noarchFileList:
	    v.writeFiles(ofile, version, soVersion)
	    ofile.write("\n")
	ofile.flush()
	ofile.close()
	os.system("rpmbuild --target noarch -bb IceJ-" + version + ".spec")

    #
    # TODO: Cleanups?  I've left everything in place so that the process can be easily debugged.
    #

if __name__ == "__main__":
    main()
