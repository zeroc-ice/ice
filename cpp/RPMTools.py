# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************
import os, sys, shutil, string, logging

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
	self.prepTextGen = None
	self.buildTextGen = None
	self.installTextGen = None
        
    def writeHdr(self, ofile, version, release, installDir):
	ofile.write("%define _unpackaged_files_terminate_build 0\n")
	ofile.write("Summary: " + self.summary + "\n")
	ofile.write("Name: " + self.name + "\n")
	ofile.write("Version: " + version + "\n")
	ofile.write("Release: " + release + "\n")
	if self.requires <> "":
            if self.requires.find("%version%"):
                self.requires = self.requires.replace("%version%", version)
	    ofile.write("Requires: " + self.requires + "\n")
	ofile.write("License: GPL\n")
	ofile.write("Group: Development/Libraries\n")
	ofile.write("Vendor: ZeroC Inc\n")
	ofile.write("URL: http://www.zeroc.com/index.html\n")
	ofile.write("Source0: http://www.zeroc.com/downloads/Ice-%{version}.tar.gz\n")
	ofile.write("Source1: http://www.zeroc.com/downloads/IceJ-%{version}.tar.gz\n")
	ofile.write("Source2: http://www.zeroc.com/downloads/IcePy-%{version}.tar.gz\n")
	ofile.write("Source3: http://www.zeroc.com/downloads/IceCS-%{version}.tar.gz\n")
	ofile.write("Source4: http://www.zeroc.com/downloads/Ice-%{version}-demos.tar.gz\n")
	ofile.write("\n")
	if installDir <> "":
	    ofile.write("BuildRoot: " + installDir + "\n")
	else:
	    ofile.write('BuildRoot: /var/tmp/Ice-' + version + '-' + release + '-buildroot\n')
	ofile.write("\n")
	ofile.write("%description\n")
	ofile.write("\n")
	ofile.write("%prep\n")
	if self.prepTextGen <> None:
	    for g in self.prepTextGen:
		g(ofile, version)
	ofile.write("\n")
	ofile.write("%build\n")
	if self.buildTextGen <> None:
	    for g in self.buildTextGen:
		g(ofile, version)
	ofile.write("\n")
	ofile.write("%install\n")
	if self.installTextGen <> None:
	    for g in self.installTextGen:
		g(ofile, version)
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

    def addPrepGenerator(self, gen):
	if self.prepTextGen == None:
	    self.prepTextGen = []
	self.prepTextGen.append(gen)

    def addBuildGenerator(self, gen):
	if self.buildTextGen == None:
	    self.buildTextGen = []
	self.buildTextGen.append(gen)

    def addInstallGenerator(self, gen):
	if self.installTextGen == None:
	    self.installTextGen = []
	self.installTextGen.append(gen)

#
# Represents subpackages in an RPM spec file.
#
class Subpackage(Package):
    def writeHdr(self, ofile, version, release, installDir):
        ofile.write("%package " + self.name + "\n")
        ofile.write("Summary: " + self.summary + "\n")
        ofile.write("Group: " + self.group + "\n")
	if self.requires <> "":
            if self.requires.find("%version%"):
                self.requires = self.requires.replace("%version%", version)
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
transforms = [ ("file", "lib/Ice.jar", "lib/Ice-%version%/Ice.jar" ),
	       ("dir", "slice", "share/slice"),
	       ("dir", "ant", "lib/Ice-%version%/ant"),
	       ("dir", "python", "lib/Ice-%version%/python"),
               ("dir", "doc", "share/doc/Ice-%version%"),
               ("file", "ICE_LICENSE", "share/doc/Ice-%version%/ICE_LICENSE"),
               ("file", "LICENSE", "share/doc/Ice-%version%/LICENSE")
               ]

#
# fileLists is an in-memory representation of the package contents of
# the Ice spec file.
# 
fileLists = [
    Package("ice",
            "",
	    "The Internet Communications Engine (ICE) is a modern alternative to object middleware such as CORBA or DCOM.",
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
		("file", "share/doc/Ice-%version%/config/Make.rules"),
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
               "ice = %version%, mono-core >= 1.0.6",
               "Ice runtime for C\# applications",
               "Development/Libraries Development/Tools",
               "",
	       "",
               [("lib", "lib/glacier2cs.dll"), ("lib", "lib/icecs.dll"), ("lib", "lib/icepackcs.dll"),
                ("lib", "lib/icepatch2cs.dll"), ("lib", "lib/icestormcs.dll")]),
    Subpackage("csharp-devel",
               "ice-csharp = %version%, mono-devel >= 1.0.6",
               "Ice tools for developing Ice applications in C\#",
               "Development/Libraries Development/Tools",
               "",
	       "",
               [("exe", "bin/slice2cs"),
		("file", "share/doc/Ice-%version%/config/Make.rules.cs"),
	        ("dir", "share/doc/Ice-%version%/democs")]),
    Subpackage("java-devel",
               "ice-java = %version%",
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
	        ("file", "share/doc/Ice-%version%/config/common.xml"),
		("dir", "share/doc/Ice-%version%/demoj")]),
    Subpackage("python",
               "ice = %version%, python >= 2.3.4",
               "Ice runtime for Python applications",
               "Development/Libraries",
               "",
	       "",
               [("lib", "lib/IcePy.so.VERSION"), ("dir", "lib/Ice-%version%/python")]),
    Subpackage("python-devel",
               "ice-python = %version%",
               "Ice tools for developing Ice applications in Python",
               "Development/Libraries Development/Tools",
               "",
	       "",
               [("exe", "bin/slice2py"),
	        ("lib", "lib/IcePy.so"),
	        ("dir", "share/doc/Ice-%version%/demopy")])
    ]

noarchFileList = [
    Package("ice-java",
	    "ice = %version%, db4-java >= 4.2.52",
	    "The Internet Communications Engine (ICE) is a modern alternative to object middleware",
	    "Development/Libraries",
	    "",
	    "BuildArch: noarch",
	    [("dir", "lib/Ice-%version%/Ice.jar")
	    ])
    ]

#
# XXX - not used anywhere?
#
def _missingPathParts(source, dest):
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

def _transformDirectories(transforms, version, installDir):
    """Transforms a directory tree that was created with 'make installs' to an RPM friendly
       directory tree.  NOTE, this will not work on all transforms, there are certain
       types of transforms in certain orders that will break it."""
    cwd = os.getcwd()
    os.chdir(installDir + "/Ice-" + version)
    for type, source, dest in transforms:
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

def createArchSpecFile(ofile, installDir, version, soVersion):
    for v in fileLists:
	v.writeHdr(ofile, version, "1", installDir)
	ofile.write("\n\n\n")
    for v in fileLists:
	v.writeFiles(ofile, version, soVersion)
	ofile.write("\n")

def createNoArchSpecFile(ofile, installDir, version, soVersion):
    for v in noarchFileList:
	v.writeHdr(ofile, version, "1", installDir)
	ofile.write("\n\n\n")
    for v in noarchFileList:
	v.writeFiles(ofile, version, soVersion)
	ofile.write("\n")

def createFullSpecFile(ofile, installDir, version, soVersion):
    fullFileList = []
    fullFileList.extend(fileLists)
    fullFileList.append(
	    Subpackage("java",
		"ice = %version%, db4-java >= 4.2.52",
		"Ice runtime for Ice Java applciations",
		"Development/Libraries",
		"",
		"",
		[("dir", "lib/Ice-%version%/Ice.jar")
		]))
    fullFileList[0].addPrepGenerator(writeUnpackingCommands)
    fullFileList[0].addBuildGenerator(writeBuildCommands)
    fullFileList[0].addInstallGenerator(writeInstallCommands)
    fullFileList[0].addInstallGenerator(writeTransformCommands)
    fullFileList[0].addInstallGenerator(writeDemoPkgCommands)

    for v in fullFileList:
	v.writeHdr(ofile, version, "1", installDir)
	ofile.write("\n\n\n")
    for v in fullFileList:
	v.writeFiles(ofile, version, soVersion)
	ofile.write("\n")

def createRPMSFromBinaries(buildDir, installDir, version, soVersion):
    _transformDirectories(transforms, version, installDir)

    ofile = open(buildDir + "/Ice-" + version + ".spec", "w")
    createSpecFile(ofile, installDir, version, soVersion)
    ofile.flush()
    ofile.close()

    os.system("tar xvfz " + installDir + "/Ice-" + version + "-demos.tar.gz -C " + installDir)
    shutil.move(installDir + "/Ice-" + version, installDir + "/usr")
    #
    # Copy demo files so the RPM spec file can pick them up.
    #
    os.system("cp -pR " + installDir + "/Ice-" + version + "-demos/* " + installDir + "/usr/share/doc/Ice-" + version)
    if os.path.exists(installDir + "/Ice-" + version + "-demos"):
	shutil.rmtree(installDir + "/Ice-" + version + "-demos")
    cwd = os.getcwd()
    os.chdir(buildDir)
    os.system("rpmbuild -bb Ice-" + version + ".spec")

    # 
    # Build noarch RPMs
    #
    ofile = open(buildDir + "/IceJ-" + version + ".spec", "w")
    createNoArchSpecFile(ofile, installDir, version, soVersion)
    ofile.flush()
    ofile.close()
    os.system("rpmbuild --target noarch -bb IceJ-" + version + ".spec")

def writeUnpackingCommands(ofile, version):
    ofile.write('%setup -n Ice-%{version} -q -T -D -b 0\n')
    ofile.write("""#
# The Ice make system does not allow the prefix directory to be specified
# through an environment variable or a command line option.  So we edit some
# files in place with sed.
#
sed -i -e 's/^prefix.*$/prefix = $\(RPM_BUILD_ROOT\)/' $RPM_BUILD_DIR/Ice-%{version}/config/Make.rules
%setup -q -n IceJ-%{version} -T -D -b 1
%setup -q -n IcePy-%{version} -T -D -b 2
sed -i -e 's/^prefix.*$/prefix = $\(RPM_BUILD_ROOT\)/' $RPM_BUILD_DIR/IcePy-%{version}/config/Make.rules
%setup -q -n IceCS-%{version} -T -D -b 3 
sed -i -e 's/^prefix.*$/prefix = $\(RPM_BUILD_ROOT\)/' $RPM_BUILD_DIR/IceCS-%{version}/config/Make.rules.cs
""")

def writeBuildCommands(ofile, version):
    ofile.write("""
cd $RPM_BUILD_DIR/Ice-%{version}
gmake RPM_BUILD_ROOT=$RPM_BUILD_ROOT/usr
cd $RPM_BUILD_DIR/IcePy-%{version}
gmake ICE_HOME=$RPM_BUILD_DIR/Ice-%{version} RPM_BUILD_ROOT=$RPM_BUILD_ROOT/usr
cd $RPM_BUILD_DIR/IceCS-%{version}
export PATH=$RPM_BUILD_DIR/Ice-%{version}/bin:$PATH
export LD_LIBRARY_PATH=$RPM_BUILD_DIR/Ice-%{version}/lib:$LD_LIBRARY_PATH
gmake ICE_HOME=$RPM_BUILD_DIR/Ice-%{version} RPM_BUILD_ROOT=$RPM_BUILD_ROOT/usr
""")

def writeInstallCommands(ofile, version):
    ofile.write("""
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr
mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/lib
mkdir -p $RPM_BUILD_ROOT/usr/include
mkdir -p $RPM_BUILD_ROOT/usr/doc
cd $RPM_BUILD_DIR/Ice-%{version}
gmake RPM_BUILD_ROOT=$RPM_BUILD_ROOT/usr install
cp -p $RPM_BUILD_DIR/IceJ-%{version}/lib/Ice.jar $RPM_BUILD_ROOT/usr/lib/Ice.jar
cp -pR $RPM_BUILD_DIR/IceJ-%{version}/ant $RPM_BUILD_ROOT/usr
cd $RPM_BUILD_DIR/IcePy-%{version}
gmake ICE_HOME=$RPM_BUILD_DIR/Ice-%{version} RPM_BUILD_ROOT=$RPM_BUILD_ROOT/usr install
cd $RPM_BUILD_DIR/IceCS-%{version}
export PATH=$RPM_BUILD_DIR/Ice-%{version}/bin:$PATH
export LD_LIBRARY_PATH=$RPM_BUILD_DIR/Ice-%{version}/lib:$LD_LIBRARY_PATH
gmake ICE_HOME=$RPM_BUILD_DIR/Ice-%{version} RPM_BUILD_ROOT=$RPM_BUILD_ROOT/usr install
""")

def writeTransformCommands(ofile, version):
    ofile.write('#\n')
    ofile.write('# The following commands transform a standard Ice installation directory\n')
    ofile.write('# structure to a directory structure more suited to integrating into a\n')
    ofile.write('# Linux system.\n')
    ofile.write('#\n\n')
    for type, source, dest in transforms:
	dest = dest.replace('%version%', version)
	if type == 'file':
	    ofile.write('mkdir -p $RPM_BUILD_ROOT/usr/' + os.path.dirname(dest) + '\n')
	    ofile.write('mv $RPM_BUILD_ROOT/usr/' + source + ' $RPM_BUILD_ROOT/usr/' + dest + '\n')
	elif type == 'dir':
	    if os.path.dirname(dest) <> '' and source.split('/')[0] == dest.split('/')[0]:
		ofile.write('mkdir -p $RPM_BUILD_ROOT/arraftmp\n')
		ofile.write('mv $RPM_BUILD_ROOT/usr/' + source + ' $RPM_BUILD_ROOT/arraftmp/' + source + '\n')
		ofile.write('mkdir -p $RPM_BUILD_ROOT/usr/' + os.path.dirname(dest) + '\n')
		ofile.write('mv $RPM_BUILD_ROOT/arraftmp/' + source + ' $RPM_BUILD_ROOT/usr/' + dest + '\n')
		ofile.write('rm -rf $RPM_BUILD_ROOT/arraftmp\n')
	    elif os.path.dirname(dest) <> '':
		ofile.write('mkdir -p $RPM_BUILD_ROOT/usr/' + os.path.dirname(dest) + '\n')
		ofile.write('mv $RPM_BUILD_ROOT/usr/' + source + ' $RPM_BUILD_ROOT/usr/' + dest + '\n')
	    else:
		ofile.write('mv $RPM_BUILD_ROOT/usr/' + source + ' $RPM_BUILD_ROOT/usr/' + dest + '\n')

def writeDemoPkgCommands(ofile, version):
    ofile.write('#\n')
    ofile.write('# Extract the contents of the demo packaged into the installed location.\n')
    ofile.write('#\n')
    ofile.write('mkdir -p $RPM_BUILD_ROOT/usr/share/doc/Ice-%{version}\n')
    ofile.write('tar xvfz $RPM_SOURCE_DIR/Ice-%{version}-demos.tar.gz -C $RPM_BUILD_ROOT/usr/share/doc\n')
    ofile.write('cp -pR $RPM_BUILD_ROOT/usr/share/doc/Ice-%{version}-demos/* $RPM_BUILD_ROOT/usr/share/doc/Ice-%{version}\n')
    ofile.write('rm -rf $RPM_BUILD_ROOT/usr/share/doc/Ice-%{version}-demos\n')
	
if __name__ == "main":
    print 'Ice RPM Tools module'
