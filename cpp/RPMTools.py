# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************
import os, sys, shutil, string, logging, compileall

#
# TODO: Setup a table for the dependencies so you don't have to 'flit'
# through the package descriptions to set the dependencies.
#

buildRequires = ('byacc >= 1.9',
                 'mono-core >= 1.1.9',
                 'mono-devel >= 1.1.9',
                 'python >= 2.4.1',
                 'python-devel >= 2.4.1',
                 'bzip2-devel >= 1.0.2',
                 'bzip2-libs >= 1.0.2',
                 'ant >= 1.6.2',
                 'expat-devel >= 1.9',
                 'expat >= 1.9',
                 'libstdc++ >= 3.2',
                 'gcc >= 3.2',
                 'gcc-c++ >= 3.2',
                 'tar',
                 'binutils >= 2.10',
                 'openssl >= 0.9.7f',
                 'openssl-devel >= 0.9.7f',
                 'readline >= 5.0',
                 'ncurses >= 5.4'
                 )

iceDescription = '''Ice is a modern alternative to object middleware
such as CORBA or COM/DCOM/COM+.  It is easy to learn, yet provides a
powerful network infrastructure for demanding technical applications. It
features an object-oriented specification language, easy to use C++,
Java, Python, PHP, C#, and Visual Basic mappings, a highly efficient
protocol, asynchronous method invocation and dispatch, dynamic transport
plug-ins, TCP/IP and UDP/IP support, SSL-based security, a firewall
solution, and much more.'''

#
# Represents the 'main' package of an RPM spec file.
# TODO: The package should really write the RPM header has well.
#
class Package:

    '''Encapsulates RPM spec file information to be used to generate a spec file on Linux and create RPMs for Ice.
    Root packages have the restriction that they cannot be noarch specific. If this becomes a problem in the future,
    the thing to do would be to remove the %ifnarch noarch/%endif pairs from Package and create a new subclass that
    writes these pairs and delegates the guts of the file populating to Package.'''

    def __init__(self, name, requires, summary, group, description, other, filelist, arch):
        self.name = name
        self.requires = requires
        self.summary = summary
        self.group = group
        self.description = description
        self.filelist = filelist
	self.other = other
	self.prepTextGen = []
	self.buildTextGen = []
	self.installTextGen = []
	self.arch = arch 
        
    def writeHdr(self, ofile, version, release, installDir, buildRequires):
	ofile.write('%define _unpackaged_files_terminate_build 0\n')
	ofile.write('Summary: ' + self.summary + '\n')
	ofile.write('Name: ' + self.name + '\n')
	ofile.write('Version: ' + version + '\n')
	ofile.write('Release: ' + release + '\n')
	if len(self.requires) != 0:
            if self.requires.find('%version%'):
                self.requires = self.requires.replace('%version%', version)
	    ofile.write('Requires: ' + self.requires + '\n')
	ofile.write('License: GPL\n')
	ofile.write('Group:' + self.group + '\n')
	ofile.write('Vendor: ZeroC, Inc\n')
	ofile.write('URL: http://www.zeroc.com/\n')

	#
	# major.minor is part of the URL, this needs to be parameterized.
	#
        minorVer = version[0:3]
	ofile.write('Source0: http://www.zeroc.com/download/Ice/' + minorVer + '/Ice-%{version}.tar.gz\n')
	ofile.write('Source1: http://www.zeroc.com/download/Ice/' + minorVer + '/IceJ-%{version}.tar.gz\n')
	ofile.write('Source2: http://www.zeroc.com/download/Ice/' + minorVer + '/IcePy-%{version}.tar.gz\n')
	ofile.write('Source3: http://www.zeroc.com/download/Ice/' + minorVer + '/IceCS-%{version}.tar.gz\n')
	ofile.write('Source4: http://www.zeroc.com/download/Ice/' + minorVer + '/Ice-%{version}-demos.tar.gz\n')
	ofile.write('Source5: http://www.zeroc.com/download/Ice/' + minorVer + '/README.Linux-RPM\n')
	ofile.write('Source6: http://www.zeroc.com/download/Ice/' + minorVer + '/ice.ini\n')
	ofile.write('Source7: http://www.zeroc.com/download/Ice/' + minorVer + '/configure.5.0.4.gz\n')
	ofile.write('Source8: http://www.zeroc.com/download/Ice/' + minorVer + '/php-5.0.4.tar.bz2\n')
	ofile.write('Source9: http://www.zeroc.com/download/Ice/' + minorVer + '/IcePHP-%{version}.tar.gz\n')
	ofile.write('Source10: http://www.zeroc.com/download/Ice/' + minorVer + '/iceproject.xml\n')
	ofile.write('\n')
	if len(installDir) != 0:
	    ofile.write('BuildRoot: ' + installDir + '\n')
	else:
	    ofile.write('BuildRoot: /var/tmp/Ice-' + version + '-' + release + '-buildroot\n')
        ofile.write('\n')
        for f in buildRequires:
            ofile.write('BuildRequires: ' + f  + '\n')
	ofile.write('\n')
	ofile.write('Provides: %s-%s\n' % (self.name, self.arch))
	ofile.write('%description\n')
	ofile.write(self.description)
	ofile.write('\n')
	ofile.write('%prep\n')
	for g in self.prepTextGen:
	    g(ofile, version)
	ofile.write('\n')
	ofile.write('%build\n')
	for g in self.buildTextGen:
	    g(ofile, version)
	ofile.write('\n')
	ofile.write('%install\n')
	for g in self.installTextGen:
	    g(ofile, version)
	ofile.write('\n')
	ofile.write('%clean\n')
	ofile.write('\n')
	ofile.write('%changelog\n')
	ofile.write('* Tue Nov 15 2005 ZeroC Staff\n')
	ofile.write('- See source distributions or the ZeroC website for more information\n')
	ofile.write('  about the changes in this release\n') 
	ofile.write('\n')

    def writeFileList(self, ofile, version, intVersion, installDir):
        ofile.write('%defattr(644, root, root, 755)\n\n')
        for perm, f in self.filelist:
            prefix = ''
	    
	    #
	    # Select an RPM spec file attribute depending on the type of
	    # file or directory we've specified.
	    #
            if perm == 'exe' or perm == 'lib':
                prefix = '%attr(755, root, root) '
	    elif perm == 'cfg':
		prefix = '%config'
	    elif perm == 'xdir':
		prefix = '%dir '

            if f.find('%version%'):
                f = f.replace('%version%', version)

            if perm == 'lib' and f.endswith('.VERSION'):
		fname = os.path.splitext(f)[0]
                ofile.write(prefix + '/usr/' + fname + '.' + version + '\n')
                ofile.write(prefix + '/usr/' + fname + '.' + str(intVersion) + '\n')
	    elif perm == 'cfg':
		ofile.write(f + '\n')
	    else:
		ofile.write(prefix + '/usr/' + f + '\n')
        ofile.write('\n')    

    def writePostInstall(self, ofile, version, intVersion, installDir):
	pass

    def writePostUninstall(self, ofile, version, intVersion, installDir):
	pass

    def writeFiles(self, ofile, version, intVersion, installDir):
        ofile.write('%files\n')
	ofile.write('\n%ifnarch noarch\n')
        self.writeFileList(ofile, version, intVersion, installDir)
	ofile.write('\n%endif\n')
	ofile.write('\n')

	ofile.write('%post\n')
	self.writePostInstall(ofile, version, intVersion, installDir)

	ofile.write('%postun\n')
	self.writePostUninstall(ofile, version, intVersion, installDir)
	ofile.write('\n')

    def addPrepGenerator(self, gen):
	self.prepTextGen.append(gen)

    def addBuildGenerator(self, gen):
	self.buildTextGen.append(gen)

    def addInstallGenerator(self, gen):
	self.installTextGen.append(gen)

#
# Represents subpackages in an RPM spec file.
#
class Subpackage(Package):
    def writeHdr(self, ofile, version, release, installDir, buildRequires):
        ofile.write('\n%%ifarch %s\n' % self.arch)	
        ofile.write('%package ' + self.name + '\n')
        ofile.write('Summary: ' + self.summary + '\n')
        ofile.write('Group: ' + self.group + '\n')
	if len(self.requires) != 0:
            if self.requires.find('%version%'):
                self.requires = self.requires.replace('%version%', version)
	    ofile.write('Requires: ' + self.requires + '\n')
	if len(self.other) != 0:
	    ofile.write(self.other + '\n')
        ofile.write('%description ' + self.name + '\n')
        ofile.write(self.description)
	ofile.write('\n%endif\n')

    def writeFiles(self, ofile, version, intVersion, installDir):
        ofile.write('\n%%ifarch %s\n' % self.arch)	
        ofile.write('%files ' + self.name + '\n')
        self.writeFileList(ofile, version, intVersion, installDir)

	ofile.write('%post ' + self.name + '\n')
	self.writePostInstall(ofile, version, intVersion, installDir)

	ofile.write('%postun ' + self.name + '\n')
	self.writePostUninstall(ofile, version, intVersion, installDir)
	ofile.write('\n%endif\n')


class DotNetPackage(Subpackage):
    def writePostInstall(self, ofile, version, intVersion, installDir):
	ofile.write('\n%ifarch noarch\n')
	ofile.write('''
for f in icecs glacier2cs iceboxcs icegridcs icepatch2cs icestormcs;
do
    sed -i.bak -e "s/^mono_root.*$/mono_root = \/usr/" /usr/lib/pkgconfig/$f.pc ; 
done
	''')
	ofile.write('\n%endif\n')

#
# NOTE: File transforms should be listed before directory transforms.
#
transforms = [ ('file', 'ice.ini', 'etc/php.d/ice.ini'),
	       ('dir', 'lib', 'usr/lib'),
	       ('file', 'usr/lib/icephp.so', 'usr/lib/php/modules/icephp.so'),
	       ('file', 'usr/lib/Ice.jar', 'usr/lib/Ice-%version%/Ice.jar' ),
	       ('file', 'usr/lib/IceGridGUI.jar', 'usr/lib/Ice-%version%/IceGridGUI.jar' ),
	       ('file', 'bin/icecs.dll', 'usr/lib/mono/gac/icecs/%version%.0__1f998c50fec78381/icecs.dll'),
	       ('file', 'bin/glacier2cs.dll',
		       'usr/lib/mono/gac/glacier2cs/%version%.0__1f998c50fec78381/glacier2cs.dll'),
	       ('file', 'bin/iceboxcs.dll',
		       'usr/lib/mono/gac/iceboxcs/%version%.0__1f998c50fec78381/iceboxcs.dll'),
	       ('file', 'bin/icegridcs.dll',
		       'usr/lib/mono/gac/icegridcs/%version%.0__1f998c50fec78381/icegridcs.dll'),
	       ('file', 'bin/icepatch2cs.dll',
		       'usr/lib/mono/gac/icepatch2cs/%version%.0__1f998c50fec78381/icepatch2cs.dll'),
	       ('file', 'bin/icestormcs.dll',
		       'usr/lib/mono/gac/icestormcs/%version%.0__1f998c50fec78381/icestormcs.dll'),
	       ('dir', 'ant', 'usr/lib/Ice-%version%/ant'),
	       ('dir', 'config', 'usr/share/doc/Ice-%version%/config'),
	       ('dir', 'slice', 'usr/share/slice'),
	       ('dir', 'bin', 'usr/bin'),
	       ('dir', 'include', 'usr/include'),
	       ('dir', 'python', 'usr/lib/Ice-%version%/python'),
               ('dir', 'doc', 'usr/share/doc/Ice-%version%/doc'),
               ('file', 'README', 'usr/share/doc/Ice-%version%/README'),
               ('file', 'ICE_LICENSE', 'usr/share/doc/Ice-%version%/ICE_LICENSE'),
               ('file', 'LICENSE', 'usr/share/doc/Ice-%version%/LICENSE')
               ]

x64_transforms = [ 
	       ('dir', 'config', 'usr/share/doc/Ice-%version%/config'),
	       ('dir', 'slice', 'usr/share/slice'),
               ('dir', 'doc', 'usr/share/doc/Ice-%version%/doc'),
	       ('dir', 'bin', 'usr/bin'),
	       ('dir', 'include', 'usr/include'),
	       ('dir', 'lib64', 'usr/lib64'),
	       ('file', 'usr/lib64/IceGridGUI.jar', 'usr/lib64/Ice-%version%/IceGridGUI.jar' ),
               ('file', 'README', 'usr/share/doc/Ice-%version%/README'),
               ('file', 'ICE_LICENSE', 'usr/share/doc/Ice-%version%/ICE_LICENSE'),
               ('file', 'LICENSE', 'usr/share/doc/Ice-%version%/LICENSE')
               ]

#
# fileLists is an in-memory representation of the package contents of
# the Ice spec file.
# 
fileLists64 = [
    Package('ice',
            '',
	    'The Ice base runtime and services',
            'System Environment/Libraries',
	    iceDescription,
	    'Provides: ice-x86_64',
            [('xdir', 'share/doc/Ice-%version%'),
             ('doc', 'share/doc/Ice-%version%/ICE_LICENSE'),
             ('doc', 'share/doc/Ice-%version%/LICENSE'),
             ('doc', 'share/doc/Ice-%version%/README'),
             ('exe', 'bin/dumpdb'),
             ('exe', 'bin/transformdb'),
             ('exe', 'bin/glacier2router'),
             ('exe', 'bin/icebox'),
             ('exe', 'bin/iceboxadmin'),
             ('exe', 'bin/icecpp'),
             ('exe', 'bin/icepatch2calc'),
             ('exe', 'bin/icepatch2client'),
             ('exe', 'bin/icepatch2server'),
             ('exe', 'bin/icestormadmin'),
             ('exe', 'bin/slice2docbook'), 
             ('exe', 'bin/icegridadmin'), 
             ('exe', 'bin/icegridnode'), 
             ('exe', 'bin/icegridregistry'), 
             ('lib', 'lib64/libFreeze.so.VERSION'),
             ('lib', 'lib64/libGlacier2.so.VERSION'),
             ('lib', 'lib64/libIceBox.so.VERSION'),
             ('lib', 'lib64/libIcePatch2.so.VERSION'),
             ('lib', 'lib64/libIce.so.VERSION'),
             ('lib', 'lib64/libIceSSL.so.VERSION'),
             ('lib', 'lib64/libIceStormService.so.VERSION'),
             ('lib', 'lib64/libIceStorm.so.VERSION'),
             ('lib', 'lib64/libIceUtil.so.VERSION'),
             ('lib', 'lib64/libIceXML.so.VERSION'),
             ('lib', 'lib64/libSlice.so.VERSION'),
             ('lib', 'lib64/libIceGrid.so.VERSION'),
	     ('dir', 'lib64/Ice-%version%/IceGridGUI.jar'),
             ('dir', 'share/slice'),
             ('dir', 'share/doc/Ice-%version%/doc'),
             ('xdir', 'share/doc/Ice-%version%/certs'),
	     ('file', 'share/doc/Ice-%version%/certs/cacert.pem'),
	     ('file', 'share/doc/Ice-%version%/certs/c_dh1024.pem'),
	     ('file', 'share/doc/Ice-%version%/certs/client_sslconfig.xml'),
	     ('file', 'share/doc/Ice-%version%/certs/server_sslconfig.xml'),
	     ('file', 'share/doc/Ice-%version%/certs/c_rsa1024_priv.pem'),
	     ('file', 'share/doc/Ice-%version%/certs/c_rsa1024_pub.pem'),
	     ('file', 'share/doc/Ice-%version%/certs/s_dh1024.pem'),
	     ('file', 'share/doc/Ice-%version%/certs/s_rsa1024_priv.pem'),
	     ('file', 'share/doc/Ice-%version%/certs/s_rsa1024_pub.pem'),
	     ('file', 'share/doc/Ice-%version%/certs/sslconfig.dtd'),
	     ('file', 'share/doc/Ice-%version%/certs/sslconfig.xml'),
	     ('xdir', 'share/doc/Ice-%version%/config'),
	     ('file', 'share/doc/Ice-%version%/config/templates.xml'),
	     ('file', 'share/doc/Ice-%version%/README.DEMOS')], 'x86_64'),
    Subpackage('c++-devel',
               'ice = %version%',
               'Tools and demos for developing Ice applications in C++',
               'Development/Tools',
	       iceDescription,
	       'Requires: ice-x86_64',
               [('exe', 'bin/slice2cpp'),
                ('exe', 'bin/slice2freeze'),
                ('dir', 'include'),
		('lib', 'lib64/libFreeze.so'),
		('lib', 'lib64/libGlacier2.so'),
		('lib', 'lib64/libIceBox.so'),
		('lib', 'lib64/libIceGrid.so'),
		('lib', 'lib64/libIcePatch2.so'),
		('lib', 'lib64/libIce.so'),
		('lib', 'lib64/libIceSSL.so'),
		('lib', 'lib64/libIceStormService.so'),
		('lib', 'lib64/libIceStorm.so'),
		('lib', 'lib64/libIceUtil.so'),
		('lib', 'lib64/libIceXML.so'),
		('lib', 'lib64/libSlice.so'),
		('xdir', 'share/doc/Ice-%version%'),
		('dir', 'share/doc/Ice-%version%/demo'),
		('exe', 'share/doc/Ice-%version%/demo/Freeze/backup/backup'),
		('exe', 'share/doc/Ice-%version%/demo/Freeze/backup/recover'),
		('xdir', 'share/doc/Ice-%version%/config'),
		('file', 'share/doc/Ice-%version%/config/Make.rules'),
		('file', 'share/doc/Ice-%version%/config/Make.rules.Linux'),
		], 'x86_64')
    ]

fileLists = [
    Package('ice',
            '',
	    'The Ice base runtime and services',
            'System Environment/Libraries',
	    iceDescription,
	    'Provides: ice-i386',
            [('xdir', 'share/doc/Ice-%version%'),
             ('doc', 'share/doc/Ice-%version%/ICE_LICENSE'),
             ('doc', 'share/doc/Ice-%version%/LICENSE'),
             ('doc', 'share/doc/Ice-%version%/README'),
             ('exe', 'bin/dumpdb'),
             ('exe', 'bin/transformdb'),
             ('exe', 'bin/glacier2router'),
             ('exe', 'bin/icebox'),
             ('exe', 'bin/iceboxadmin'),
             ('exe', 'bin/icecpp'),
             ('exe', 'bin/icepatch2calc'),
             ('exe', 'bin/icepatch2client'),
             ('exe', 'bin/icepatch2server'),
             ('exe', 'bin/icestormadmin'),
             ('exe', 'bin/slice2docbook'), 
             ('exe', 'bin/icegridadmin'), 
             ('exe', 'bin/icegridnode'), 
             ('exe', 'bin/icegridregistry'), 
             ('lib', 'lib/libFreeze.so.VERSION'),
             ('lib', 'lib/libGlacier2.so.VERSION'),
             ('lib', 'lib/libIceBox.so.VERSION'),
             ('lib', 'lib/libIcePatch2.so.VERSION'),
             ('lib', 'lib/libIce.so.VERSION'),
             ('lib', 'lib/libIceSSL.so.VERSION'),
             ('lib', 'lib/libIceStormService.so.VERSION'),
             ('lib', 'lib/libIceStorm.so.VERSION'),
             ('lib', 'lib/libIceUtil.so.VERSION'),
             ('lib', 'lib/libIceXML.so.VERSION'),
             ('lib', 'lib/libSlice.so.VERSION'),
             ('lib', 'lib/libIceGrid.so.VERSION'),
	     ('dir', 'lib/Ice-%version%/IceGridGUI.jar'),
             ('dir', 'share/slice'),
             ('dir', 'share/doc/Ice-%version%/doc'),
             ('xdir', 'share/doc/Ice-%version%/certs'),
	     ('file', 'share/doc/Ice-%version%/certs/cacert.pem'),
	     ('file', 'share/doc/Ice-%version%/certs/c_dh1024.pem'),
	     ('file', 'share/doc/Ice-%version%/certs/client_sslconfig.xml'),
	     ('file', 'share/doc/Ice-%version%/certs/server_sslconfig.xml'),
	     ('file', 'share/doc/Ice-%version%/certs/c_rsa1024_priv.pem'),
	     ('file', 'share/doc/Ice-%version%/certs/c_rsa1024_pub.pem'),
	     ('file', 'share/doc/Ice-%version%/certs/s_dh1024.pem'),
	     ('file', 'share/doc/Ice-%version%/certs/s_rsa1024_priv.pem'),
	     ('file', 'share/doc/Ice-%version%/certs/s_rsa1024_pub.pem'),
	     ('file', 'share/doc/Ice-%version%/certs/sslconfig.dtd'),
	     ('file', 'share/doc/Ice-%version%/certs/sslconfig.xml'),
	     ('xdir', 'share/doc/Ice-%version%/config'),
	     ('file', 'share/doc/Ice-%version%/config/templates.xml'),
	     ('file', 'share/doc/Ice-%version%/README.DEMOS')], 'i386'),
    Subpackage('c++-devel',
               'ice = %version%',
               'Tools and demos for developing Ice applications in C++',
               'Development/Tools',
	       iceDescription,
	       'Requires: ice-i386',
               [('exe', 'bin/slice2cpp'),
                ('exe', 'bin/slice2freeze'),
                ('dir', 'include'),
		('lib', 'lib/libFreeze.so'),
		('lib', 'lib/libGlacier2.so'),
		('lib', 'lib/libIceBox.so'),
		('lib', 'lib/libIceGrid.so'),
		('lib', 'lib/libIcePatch2.so'),
		('lib', 'lib/libIce.so'),
		('lib', 'lib/libIceSSL.so'),
		('lib', 'lib/libIceStormService.so'),
		('lib', 'lib/libIceStorm.so'),
		('lib', 'lib/libIceUtil.so'),
		('lib', 'lib/libIceXML.so'),
		('lib', 'lib/libSlice.so'),
		('xdir', 'share/doc/Ice-%version%'),
		('dir', 'share/doc/Ice-%version%/demo'),
		('exe', 'share/doc/Ice-%version%/demo/Freeze/backup/backup'),
		('exe', 'share/doc/Ice-%version%/demo/Freeze/backup/recover'),
		('xdir', 'share/doc/Ice-%version%/config'),
		('file', 'share/doc/Ice-%version%/config/Make.rules'),
		('file', 'share/doc/Ice-%version%/config/Make.rules.Linux'),
		], 'i386'),
    Subpackage('csharp-devel',
               'ice-dotnet = %version%',
               'Tools and demos for developing Ice applications in C#',
               'Development/Tools',
	       iceDescription,
	       'Requires: ice-i386',
               [('exe', 'bin/slice2cs'),
		('xdir', 'share/doc/Ice-%version%'),
		('xdir', 'share/doc/Ice-%version%/config'),
		('file', 'share/doc/Ice-%version%/config/Make.rules.cs'),
	        ('dir', 'share/doc/Ice-%version%/democs')], 'i386'),
    Subpackage('java-devel',
               'ice-java = %version%',
               'Tools and demos for developing Ice applications in Java',
               'Development/Tools',
	       iceDescription,
	       'Requires: ice-i386',
               [('exe', 'bin/slice2java'),
                ('exe', 'bin/slice2freezej'),
		('xdir', 'lib/Ice-%version%'),
		('dir', 'lib/Ice-%version%/ant'),
		('xdir', 'share/doc/Ice-%version%'),
		('xdir', 'share/doc/Ice-%version%/certs'),
	        ('file', 'share/doc/Ice-%version%/certs/certs.jks'),
	        ('file', 'share/doc/Ice-%version%/certs/client.jks'),
	        ('file', 'share/doc/Ice-%version%/certs/server.jks'),
		('xdir', 'share/doc/Ice-%version%/config'),
	        ('file', 'share/doc/Ice-%version%/config/build.properties'),
	        ('file', 'share/doc/Ice-%version%/config/common.xml'),
	        ('file', 'share/doc/Ice-%version%/config/iceproject.xml'),
		('dir', 'share/doc/Ice-%version%/demoj')], 'i386'),
    Subpackage('python',
               'ice = %version%, python >= 2.4.1',
               'The Ice runtime for Python applications',
               'System Environment/Libraries',
	       iceDescription,
	       'Requires: ice-i386',
               [('dir', 'lib/Ice-%version%/python')], 'i386'),
    Subpackage('python-devel',
               'ice-python = %version%',
               'Tools and demos for developing Ice applications in Python',
               'Development/Tools',
	       iceDescription,
	       'Requires: ice-i386',
               [('exe', 'bin/slice2py'),
		('xdir', 'share/doc/Ice-%version%'),
	        ('dir', 'share/doc/Ice-%version%/demopy')], 'i386'),
    Subpackage('php',
	       'ice = %version%, php = 5.0.4',
	       'The Ice runtime for PHP applications',
	       'System Environment/Libraries',
	       iceDescription,
	       'Requires: ice-i386',
	       [('lib', 'lib/php/modules'), ('cfg', '/etc/php.d/ice.ini')], 'i386'),
    Subpackage('php-devel',
	       'ice = %version%, php = 5.0.4, ice-php = %version%',
	       'Demos for developing Ice applications in PHP',
	       'Development/Tools',
	       iceDescription,
	       'Requires: ice-i386',
	       [('dir', 'share/doc/Ice-%version%/demophp')], 'i386'),
    Subpackage('java',
	       'ice = %version%, db4-java >= 4.3.27',
	       'The Ice runtime for Java',
	       'System Environment/Libraries',
	       iceDescription,
	       '',
	       [ ('xdir', 'lib/Ice-%version%'),
	       ('dir', 'lib/Ice-%version%/Ice.jar')
	       ], 'noarch'),
    DotNetPackage('dotnet',
                  'ice = %version%, mono-core >= 1.1.9',
		  'The Ice runtime for C# applications',
		  'System Environment/Libraries',
		  iceDescription,
		  '',
		  [('dll', 'lib/mono/gac/glacier2cs/%version%.0__1f998c50fec78381/glacier2cs.dll'),
		  ('dll', 'lib/mono/gac/icecs/%version%.0__1f998c50fec78381/icecs.dll'),
		  ('dll', 'lib/mono/gac/iceboxcs/%version%.0__1f998c50fec78381/iceboxcs.dll'),
		  ('dll', 'lib/mono/gac/icegridcs/%version%.0__1f998c50fec78381/icegridcs.dll'),
		  ('dll', 'lib/mono/gac/icepatch2cs/%version%.0__1f998c50fec78381/icepatch2cs.dll'),
		  ('dll', 'lib/mono/gac/icestormcs/%version%.0__1f998c50fec78381/icestormcs.dll'),
		  ('file', 'lib/pkgconfig/icecs.pc'),
		  ('file', 'lib/pkgconfig/glacier2cs.pc'),
		  ('file', 'lib/pkgconfig/iceboxcs.pc'),
		  ('file', 'lib/pkgconfig/icegridcs.pc'),
		  ('file', 'lib/pkgconfig/icepatch2cs.pc'),
		  ('file', 'lib/pkgconfig/icestormcs.pc'),
		  ('exe', 'bin/iceboxnet.exe')], 'noarch')
    ]

def _transformDirectories(transforms, version, installDir):
    """Transforms a directory tree that was created with 'make installs'
    to an RPM friendly directory tree.  NOTE, this will not work on all
    transforms, there are certain types of transforms in certain orders
    that will break it."""
    cwd = os.getcwd()
    os.chdir(installDir)
    for type, source, dest in transforms:
        if dest.find('%version%'):
            dest = dest.replace('%version%', version)
	if source.find('%version%'):
	    source = source.replace('%version%', version)

        sourcedir = source
        destdir = dest

	if os.path.exists('./tmp'):
	    shutil.rmtree('./tmp')

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
		    print 'Renaming ' + source + ' to ' + dest
		    os.renames(source, dest)

	except OSError:
	    print "Exception occurred while trying to transform " + source + " to " + dest
	    raise

    os.chdir(cwd)

def createArchSpecFile(ofile, installDir, version, soVersion, buildReq = True):
    if not buildReq:
	reqs = []
    else:
	reqs = buildRequires
	
    for v in fileLists:
	v.writeHdr(ofile, version, "1", installDir, reqs)
	ofile.write("\n\n\n")
    for v in fileLists:
	v.writeFiles(ofile, version, soVersion, installDir)
	ofile.write("\n")

def createFullSpecFile(ofile, installDir, version, soVersion, buildReq = True):
    if not buildReq:
	reqs = []
    else:
	reqs = buildRequires

    fullFileList = fileLists 
    fullFileList[0].addPrepGenerator(writeUnpackingCommands)
    fullFileList[0].addBuildGenerator(writeBuildCommands)
    fullFileList[0].addInstallGenerator(writeInstallCommands)
    fullFileList[0].addInstallGenerator(writeTransformCommands)
    fullFileList[0].addInstallGenerator(writeDemoPkgCommands)

    for v in fullFileList:
	v.writeHdr(ofile, version, "1", '', reqs)
	ofile.write("\n\n\n")
    for v in fullFileList:
	v.writeFiles(ofile, version, soVersion, '')
	ofile.write("\n")

def createRPMSFromBinaries(buildDir, installDir, version, soVersion):
    if os.path.exists(installDir + "/rpmbase"):
	shutil.rmtree(installDir + "/rpmbase")
    shutil.copytree(installDir + "/Ice-" + version, installDir + "/rpmbase")
    installDir = installDir + '/rpmbase'
    compileall.compile_dir(installDir + '/python')

    _transformDirectories(transforms, version, installDir)
    os.system("tar xfz " + installDir + "/../Ice-" + version + "-demos.tar.gz -C " + installDir)

    ofile = open(buildDir + "/Ice-" + version + ".spec", "w")
    createArchSpecFile(ofile, installDir, version, soVersion, False)
    ofile.flush()
    ofile.close()
    #
    # Copy demo files so the RPM spec file can pick them up.
    #
    os.system("cp -pR " + installDir + "/Ice-" + version + "-demos/* " + installDir + "/usr/share/doc/Ice-" + version)
    if os.path.exists(installDir + "/Ice-" + version + "-demos"):
	shutil.rmtree(installDir + "/Ice-" + version + "-demos")
    cwd = os.getcwd()
    os.chdir(buildDir)

    # 
    # The first run will create all of the architecture specific
    # packages. The second run will create the noarch packages.
    #
    result = os.system("rpmbuild -bb Ice-%s.spec" % version)
    if result != 0:
	print 'unable to build arch specific rpms'
	sys.exit(1)
    result = os.system("rpmbuild --target noarch -bb Ice-%s.spec" % version)
    if result != 0:
	print 'unable to build noarch rpms'
	sys.exit(1)

    #
    # Create a spec file to be included in the SRPM that contains shell
    # code to do all of the necessary builds and transformations.
    #
    ofile = open(installDir + '/ice-' + version + '.spec', 'w')
    createFullSpecFile(ofile, installDir, version, soVersion)
    ofile.flush()
    ofile.close()
    result = os.system('rpmbuild -bs ' + installDir + '/ice-' + version + '.spec')
    if result != 0:
	print 'unable to build srpm'
	sys.exit(1)

#
# TODO - refactor so this doesn't have to be special cased.
# 
def createRPMSFromBinaries64(buildDir, installDir, version, soVersion):
    if os.path.exists(installDir + "/rpmbase"):
	shutil.rmtree(installDir + "/rpmbase")
    shutil.copytree(installDir + "/Ice-" + version, installDir + "/rpmbase")
    installDir = installDir + "/rpmbase"

    _transformDirectories(x64_transforms, version, installDir)
    os.system("tar xfz " + installDir + "/../Ice-" + version + "-demos.tar.gz -C " + installDir)

    ofile = open(buildDir + "/Ice-" + version + ".spec", "w")
    fileLists64[0].writeHdr(ofile, version, '1', installDir, [])
    fileLists64[1].writeHdr(ofile, version, '1', installDir, [])
    ofile.write('\n\n\n')
    fileLists64[0].writeFiles(ofile, version, soVersion, installDir)
    fileLists64[1].writeFiles(ofile, version, soVersion, installDir)
    ofile.write('\n')

    ofile.flush()
    ofile.close()
    #
    # Copy demo files so the RPM spec file can pick them up.
    #
    os.system("cp -pR " + installDir + "/Ice-" + version + "-demos/* " + installDir + "/usr/share/doc/Ice-" + version)
    if os.path.exists(installDir + "/Ice-" + version + "-demos"):
	shutil.rmtree(installDir + "/Ice-" + version + "-demos")
    cwd = os.getcwd()
    os.chdir(buildDir)
    os.system("rpmbuild -bb Ice-" + version + ".spec")

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
sed -i -e 's/^cvs_build.*$/cvs_build = no/' $RPM_BUILD_DIR/IceCS-%{version}/config/Make.rules.cs
%setup -q -n Ice-%{version}-demos -T -D -b 4 
cd $RPM_BUILD_DIR
tar xfz $RPM_SOURCE_DIR/IcePHP-%{version}.tar.gz
tar xfj $RPM_SOURCE_DIR/php-5.0.4.tar.bz2
rm -f $RPM_BUILD_DIR/php-5.0.4/ext/ice
ln -s $RPM_BUILD_DIR/IcePHP-%{version}/src/ice $RPM_BUILD_DIR/php-5.0.4/ext
cp $RPM_SOURCE_DIR/ice.ini $RPM_BUILD_DIR/IcePHP-%{version}
gzip -dc $RPM_SOURCE_DIR/configure.5.0.4.gz > $RPM_BUILD_DIR/php-5.0.4/configure
""")

def writeBuildCommands(ofile, version):
    ofile.write("""
cd $RPM_BUILD_DIR/Ice-%{version}
gmake OPTIMIZE=yes RPM_BUILD_ROOT=$RPM_BUILD_ROOT
cd $RPM_BUILD_DIR/IcePy-%{version}
gmake  OPTIMIZE=yes ICE_HOME=$RPM_BUILD_DIR/Ice-%{version} RPM_BUILD_ROOT=$RPM_BUILD_ROOT
cd $RPM_BUILD_DIR/IceCS-%{version}
export PATH=$RPM_BUILD_DIR/Ice-%{version}/bin:$PATH
export LD_LIBRARY_PATH=$RPM_BUILD_DIR/Ice-%{version}/lib:$LD_LIBRARY_PATH
gmake OPTIMIZE=yes ICE_HOME=$RPM_BUILD_DIR/Ice-%{version} RPM_BUILD_ROOT=$RPM_BUILD_ROOT
cd $RPM_BUILD_DIR/php-5.0.4
./configure --with-ice=shared,$RPM_BUILD_DIR/Ice-%{version}
sed -i -e 's/^EXTRA_CXXFLAGS.*$/EXTRA_CXXFLAGS = -DCOMPILE_DL_ICE/' $RPM_BUILD_DIR/php-5.0.4/Makefile
gmake
""")

def writeInstallCommands(ofile, version):
    ofile.write("""
rm -rf $RPM_BUILD_ROOT
cd $RPM_BUILD_DIR/Ice-%{version}
gmake RPM_BUILD_ROOT=$RPM_BUILD_ROOT install
cp -p $RPM_BUILD_DIR/IceJ-%{version}/lib/Ice.jar $RPM_BUILD_ROOT/lib/Ice.jar
cp -p $RPM_BUILD_DIR/IceJ-%{version}/lib/IceGridGUI.jar $RPM_BUILD_ROOT/lib/IceGridGUI.jar
cp -pR $RPM_BUILD_DIR/IceJ-%{version}/ant $RPM_BUILD_ROOT
cd $RPM_BUILD_DIR/IcePy-%{version}
gmake ICE_HOME=$RPM_BUILD_DIR/Ice-%{version} RPM_BUILD_ROOT=$RPM_BUILD_ROOT install
cd $RPM_BUILD_DIR/IceCS-%{version}
export PATH=$RPM_BUILD_DIR/Ice-%{version}/bin:$PATH
export LD_LIBRARY_PATH=$RPM_BUILD_DIR/Ice-%{version}/lib:$LD_LIBRARY_PATH
gmake NOGAC=yes ICE_HOME=$RPM_BUILD_DIR/Ice-%{version} RPM_BUILD_ROOT=$RPM_BUILD_ROOT install
cp $RPM_SOURCE_DIR/README.Linux-RPM $RPM_BUILD_ROOT/README
cp $RPM_SOURCE_DIR/ice.ini $RPM_BUILD_ROOT/ice.ini
cp $RPM_BUILD_DIR/php-5.0.4/modules/ice.so $RPM_BUILD_ROOT/lib/icephp.so
cp -pR $RPM_BUILD_DIR/Ice-%{version}-demos/config $RPM_BUILD_ROOT
cp $RPM_SOURCE_DIR/iceproject.xml $RPM_BUILD_ROOT/config
if test ! -d $RPM_BUILD_ROOT/usr/lib/pkgconfig ; 
then 
    mkdir $RPM_BUILD_ROOT/lib/pkgconfig
fi

for f in icecs glacier2cs iceboxcs icegridcs icepatch2cs icestormcs; 
do 
    cp $RPM_BUILD_DIR/IceCS-%{version}/bin/$f.dll $RPM_BUILD_ROOT/bin
    cp $RPM_BUILD_DIR/IceCS-%{version}/lib/pkgconfig/$f.pc $RPM_BUILD_ROOT/lib/pkgconfig 
done

""")

def writeTransformCommands(ofile, version):
    #
    #  XXX- this needs serious revisiting after changing how the
    #  transforms work to accomodate files for /etc.
    #
    ofile.write('#\n')
    ofile.write('# The following commands transform a standard Ice installation directory\n')
    ofile.write('# structure to a directory structure more suited to integrating into a\n')
    ofile.write('# Linux system.\n')
    ofile.write('#\n\n')
    for type, source, dest in transforms:
	dest = dest.replace('%version%', version)
	source = source.replace('%version%', version)
	if type == 'file':
	    ofile.write('# Rule 1\n')
	    ofile.write('mkdir -p $RPM_BUILD_ROOT/' + os.path.dirname(dest) + '\n')
	    ofile.write('mv $RPM_BUILD_ROOT/' + source + ' $RPM_BUILD_ROOT/' + dest + '\n')
	elif type == 'dir':
	    if os.path.dirname(dest) <> '' and source.split('/')[0] == dest.split('/')[0]:
		ofile.write('# Rule 2\n')
		ofile.write('mkdir -p $RPM_BUILD_ROOT/arraftmp\n')
		ofile.write('mv $RPM_BUILD_ROOT/usr/' + source + ' $RPM_BUILD_ROOT/arraftmp/' + source + '\n')
		ofile.write('mkdir -p $RPM_BUILD_ROOT/usr/' + os.path.dirname(dest) + '\n')
		ofile.write('mv $RPM_BUILD_ROOT/arraftmp/' + source + ' $RPM_BUILD_ROOT/' + dest + '\n')
		ofile.write('rm -rf $RPM_BUILD_ROOT/arraftmp\n')
	    elif os.path.dirname(dest) <> '':
		ofile.write('# Rule 3\n')
		ofile.write('mkdir -p $RPM_BUILD_ROOT/' + os.path.dirname(dest) + '\n')
		ofile.write('mv $RPM_BUILD_ROOT/' + source + ' $RPM_BUILD_ROOT/' + dest + '\n')
	    else:
		ofile.write('# Rule 4\n')
		ofile.write('mv $RPM_BUILD_ROOT/usr/' + source + ' $RPM_BUILD_ROOT/usr/' + dest + '\n')

def writeDemoPkgCommands(ofile, version):
    ofile.write('#\n')
    ofile.write('# Extract the contents of the demo packaged into the installed location.\n')
    ofile.write('#\n')
    ofile.write('mkdir -p $RPM_BUILD_ROOT/usr/share/doc/Ice-%{version}\n')
    ofile.write('tar xfz $RPM_SOURCE_DIR/Ice-%{version}-demos.tar.gz -C $RPM_BUILD_ROOT/usr/share/doc\n')
    ofile.write('cp -pR $RPM_BUILD_ROOT/usr/share/doc/Ice-%{version}-demos/* $RPM_BUILD_ROOT/usr/share/doc/Ice-%{version}\n')
    ofile.write('rm -rf $RPM_BUILD_ROOT/usr/share/doc/Ice-%{version}-demos\n')
	
if __name__ == "main":
    print 'Ice RPM Tools module'
