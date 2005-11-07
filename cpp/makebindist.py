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

# TODO:
# 
#   * Tidying and tracing.
#   * Python is used in some places for 'sed' like functionality. This
#     could be replaced by Python code.

def getIceVersion(file):
    """Extract the ICE version string from a file."""
    config = open(file, 'r')
    return  re.search('ICE_STRING_VERSION \"([0-9\.]*)\"', config.read()).group(1)

def getIceSoVersion(file):
    """Extract the ICE version ordinal from a file."""
    config = open(file, 'r')
    intVersion = int(re.search('ICE_INT_VERSION ([0-9]*)', config.read()).group(1))
    majorVersion = intVersion / 10000
    minorVersion = intVersion / 100 - 100 * majorVersion
    return '%d' % (majorVersion * 10 + minorVersion)

def getPlatform():
    """Determine the platform we are building and targetting for"""
    if sys.platform.startswith('win') or sys.platform.startswith('cygwin'):
        return 'win32'
    elif sys.platform.startswith('linux'):
	if readcommand('uname -p') == 'x86_64':
	    return 'linux64'
	else:
	    return 'linux'
    elif sys.platform.startswith('sunos'):
        return 'solaris'
    elif sys.platform.startswith('hp'):
        return 'hpux'
    elif sys.platform.startswith('darwin'):
        return 'macosx'
    elif sys.platform.startswith('aix'):
        return 'aix'
    else:
        return None

def getMakeRulesSuffix():
    '''Ice for C++ contains system specific rules for make. This
    function maps the system name to the appropriate file suffix.'''
    if sys.platform.startswith('linux'):
        return 'Linux'
    elif sys.platform.startswith('sunos'):
        return 'SunOS'
    elif sys.platform.startswith('hp'):
        return 'HP-UX'
    elif sys.platform.startswith('darwin'):
        return 'Darwin'
    elif sys.platform.startswith('aix'):
        return 'AIX'
    else:
        return None
    
def initDirectory(d):
    '''Check for the existance of the directory. If it isn't there make
    it.'''
    if os.path.exists(d):
        #
        # Make sure its a directory and has correct permissions.
        #
        if not os.path.isdir(d):
            print 'Path ' + d + ' exists but is not a directory.'
            sys.exit(1)
            
        if os.access(d, os.X_OK | os.R_OK | os.W_OK):
            logging.info('Path ' + d + ' exists and is ok, continuing')
        else:
            logging.warning('Directory ' + d + ' exists, but has incorrect permissions')
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
    if getPlatform() == 'aix':
        os.environ['LIBPATH'] = ''
    os.system('cvs -d cvs.zeroc.com:/home/cvsroot export -r ' + cvsTag + ' ice/include/IceUtil/Config.h')

    result = [ getIceVersion('ice/include/IceUtil/Config.h'), getIceSoVersion('ice/include/IceUtil/Config.h')]
    os.remove('ice/include/IceUtil/Config.h')
    os.removedirs('ice/include/IceUtil')
    os.chdir(cwd)
    return result

def fixVersion(filename, version):
    f = fileinput.input(filename, True)
    for line in f:
	print line.rstrip('\n').replace('@ver@', version)
    f.close()

def getInstallFiles(cvsTag, buildDir, version):
    """Gets the install sources for this revision"""
    cwd = os.getcwd()
    os.chdir(buildDir)
    os.system('rm -rf ' + buildDir + '/ice/install')
    os.system('cvs -d cvs.zeroc.com:/home/cvsroot export -r ' + cvsTag + ' ice/install/unix')
    os.system('cvs -d cvs.zeroc.com:/home/cvsroot export -r ' + cvsTag + ' ice/install/rpm')
    os.system('cvs -d cvs.zeroc.com:/home/cvsroot export -r ' + cvsTag + ' ice/install/thirdparty')
    snapshot = os.walk('./ice/install/unix')
    for dirInfo in snapshot:
	for f in dirInfo[2]:
	    fixVersion(os.path.join(dirInfo[0], f), version)

    os.chdir(cwd)
    return buildDir + '/ice/install'

def readcommand(cmd):
    pipe_stdin, pipe_stdout = os.popen2(cmd)
    lines = pipe_stdout.readlines()
    pipe_stdin.close()
    pipe_stdout.close()
    return lines[0].strip()

def collectSourceDistributions(tag, sourceDir, cvsdir, distro):
    '''
    The location for the source distributions is not supplied so we are
    going to assume we are being called from a CVS tree and we are going
    to go get them ourselves
    '''
    cwd = os.getcwd()
    os.chdir(cwd + "/../" + cvsdir)
    if len(tag) > 0:
	print 'Making disribution ' + cvsdir + ' with tag ' + tag
    if cvsdir in ['icepy', 'ice', 'icephp']:
        os.system("./makedist.py " + tag)
    else:
        os.system("./makedist.py " + tag)
    shutil.copy("dist/" + distro + ".tar.gz", sourceDir)
    os.chdir(cwd)

def editMakeRules(filename, version):
    '''
    Ice distributions contain files with useful build rules. However,
    these rules are source distribution specific. This script edits
    these files to make them appropriate to accompany binary
    distributions.
    '''
    state = 'header'
    reIceLocation = re.compile('^[a-z]*dir.*=\s*\$\(top_srcdir\)')

    makefile =  fileinput.input(filename, True)
    for line in makefile:
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

ifneq ($(shell test -f $(ICE_DIR)/bin/icestormadmin && echo 0),0)
    $(error Ice distribution not found, please set ICE_HOME!)
endif
"""
	    elif state == 'untilprefix':
		if line.startswith('prefix'):
		    state = 'done'
	#
        # Dependency files are all going to be bogus.  The makedepend
        # script doesn't seem to work properly for the slice files.
	#
        os.chdir("..")
        os.system("sh -c 'for f in `find . -name .depend` ; do echo \"\" > $f ; done'")
	makefile.close()

def updateIceVersion(filename, version):
    f = fileinput.input(filename, True)
    for line in f: 
	print line.rstrip('\n').replace('ICE_VERSION', version)
    f.close()

def obliterate(files):
    for f in files:
	if os.path.isdir(f):
	    shutil.rmtree(f)
	else:
	    os.remove(f)

def extractDemos(sources, buildDir, version, distro, demoDir):
    """Pulls the demo directory out of a distribution and massages its
       build system so it can be built against an installed version of
       Ice"""
    cwd = os.getcwd()
    os.chdir(buildDir + "/demotree")
    print "gzip -dc " + sources + "/" + distro + ".tar.gz | tar xf - " + distro + "/demo " + distro + "/config " + distro + "/certs"
    os.system("gzip -dc " + sources + "/" + distro + ".tar.gz | tar xf - " + distro + "/demo " + distro + "/config " + distro + "/certs")
	
    shutil.move(distro + "/demo", buildDir + "/Ice-" + version + "-demos/demo" + demoDir)

    #
    # 'System' copying of files here because its just easier!  We don't
    # need any configuration out of the Python tree.
    # 
    if not os.path.exists(buildDir + "/Ice-" + version + "-demos/certs"):
	os.mkdir(buildDir + "/Ice-" + version + "-demos/certs")

    os.system("cp -pR " + distro + "/certs/* " + buildDir + "/Ice-" + version + "-demos/certs")

    #
    # 'System' copying of files here because its just easier!  We don't
    # need any configuration out of the Python tree.
    # 
    if not demoDir == "py":
	os.system("cp " + distro + "/config/* " + buildDir + "/Ice-" + version + "-demos/config")

    #
    # Collect files to remove from the demo distribution.
    # 
    remove = []
    basepath = os.path.join(buildDir, 'Ice-' + version + '-demos', 'certs')

    for f in ['openssl', 'makecerts']:
	fullpath = os.path.join(basepath, f)
        if os.path.exists(fullpath):
	    remove.append(fullpath)

    if len(demoDir) == 0:
	basepath = os.path.join(buildDir, 'Ice-' + version + '-demos', 'demo')
	for f in ['IcePatch2', os.path.join('Ice', 'MFC')]:
	    fullpath = os.path.join(basepath, f)
            if os.path.exists(fullpath):
		remove.append(fullpath)

    basepath = os.path.join(buildDir, 'Ice-' + version + '-demos', 'config')

    if distro.startswith('Ice-'):
	editMakeRules(os.path.join(basepath, 'Make.rules'), version)
 
    elif distro.startswith('IceCS-'):
        editMakeRules(os.path.join(basepath, 'Make.rules.cs'), version)

    #
    # Remove collected files.
    #
    obliterate(remove)
        
    shutil.rmtree(os.path.join(buildDir, 'demotree', distro), True)
    os.chdir(cwd)

def archiveDemoTree(buildDir, version, installFiles):
    cwd = os.getcwd()
    os.chdir(buildDir)
    
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
    os.system("sh -c 'for f in `find Ice-" + version + "-demos -name \"*\.dsp\" ` ; do rm -rf $f ; done'")
    os.system("sh -c 'for f in `find Ice-" + version + "-demos -name \"*\.dsw\" ` ; do rm -rf $f ; done'")
    os.system("sh -c 'for f in `find Ice-" + version + "-demos/democs -name \"*.sln\" ` ; do rm -rf $f ; done'")
    os.system("sh -c 'for f in `find Ice-" + version + "-demos/democs -name \"*.csproj\" ` ; do rm -rf $f ; done'")

    os.system("tar cf Ice-" + version + "-demos.tar Ice-" + version + "-demos")
    os.system("gzip -9 Ice-" + version + "-demos.tar")
    os.chdir(cwd)

def makeInstall(sources, buildDir, installDir, distro, clean, version):
    """Make the distro in buildDir sources and install it to installDir."""
    cwd = os.getcwd()
    os.chdir(buildDir)
    if clean:
        shutil.rmtree(distro, True)
        
    if not os.path.exists(distro):
        os.system('gzip -dc ' + sources + '/' + distro + '.tar.gz | tar xf -')
        
    os.chdir(distro)

    #
    # Java does not have a 'make install' process, but comes complete
    # with the Jar already built.
    # 
    if distro.startswith('IceJ'):
        shutil.copy(buildDir + '/' + distro + '/lib/Ice.jar', installDir + '/lib')
	#
	# We really just want to copy the files, not move them.
	# Shelling out to a copy is easier (and more likely to always
	# work) than shutil.copytree().
	#
	os.system('cp -pR ' + buildDir + '/' + distro + '/ant ' + installDir)
	os.system('find ' + installDir + '/ant  -name "*.java" | xargs rm')
	destDir = os.path.join(installDir, 'config')
	if not os.path.exists(destDir):
	    os.mkdir(destDir)
        shutil.copy(os.path.join('config', 'common.xml'), destDir)
        shutil.copy(os.path.join('config', 'build.properties'), destDir)
	updateIceVersion(os.path.join(destDir, 'common.xml'), version)
        os.chdir(cwd)
        return

    if distro.startswith('IcePHP'):
        os.chdir(cwd)
	return

    if distro.startswith('IceCS'):
	os.system('perl -pi -e \'s/^prefix.*$/prefix = \$\(INSTALL_ROOT\)/\' config/Make.rules.cs')
    else:
	os.system('perl -pi -e \'s/^prefix.*$/prefix = \$\(INSTALL_ROOT\)/\' config/Make.rules')

    if distro.startswith('IcePy'):
        try:
            pyHome = os.environ['PYTHON_HOME']
        except KeyError:
            pyHome = None
            
        if pyHome == None or pyHome == '':
            logging.info('PYTHON_HOME is not set, figuring it out and trying that')
            pyHome = sys.exec_prefix
            
        os.system("perl -pi -e 's/^PYTHON.HOME.*$/PYTHON\_HOME \?= "+ pyHome.replace("/", "\/") + \
		"/' config/Make.rules")
        
    if not getPlatform().startswith('linux'):
	if distro.startswith('IcePy'):
	    os.system("perl -pi -e 's/^PYTHON.INCLUDE.DIR.*$/PYTHON_INCLUDE_DIR = " +
	              "\$\(PYTHON_HOME\)\/include\/\$\(PYTHON_VERSION\)/' config/Make.rules")
	    os.system("perl -pi -e 's/^PYTHON.LIB.DIR.*$/PYTHON_LIB_DIR = " + 
	              "\$\(PYTHON_HOME\)\/lib\/\$\(PYTHON_VERSION\)\/config/' config/Make.rules")

    #
    # We call make twice. The first time is a straight make and ensures
    # that we embed the correct default library search location in the
    # binaries. The second is a 'make install' that places the files in
    # the working install directory so the archive can be packaged up.
    #

    # 
    # XXX- Optimizations need to be turned on for the release.
    #
    os.system('gmake NOGAC=yes OPTIMIZE=yes INSTALL_ROOT=/opt/Ice-%s' % version)
    os.system('gmake NOGAC=yes OPTIMIZE=yes INSTALL_ROOT=%s install' % installDir)
    os.chdir(cwd)
    
def shlibExtensions(versionString, versionInt):
    """Returns a tuple containing the extensions for the shared library, and
       the 2 symbolic links (respectively)"""
    platform = getPlatform()
    if platform == 'hpux':
        return ['.sl.' + versionString, '.sl.' + versionInt, '.sl']
    elif platform == 'macosx':
        return ['.' + versionString + '.dylib', '.' + versionInt + '.dylib', '.dylib']
    else:
        return ['.so.' + versionString, '.so.' + versionInt, '.so']

def getPlatformLibExtension():
    platform = getPlatform()
    if platform == 'hpux':
	return '.sl'
    elif platform == 'macosx':
	return '.dylib'
    else:
	return '.so'

def getDBfiles(dbLocation):
    cwd = os.getcwd()
    os.chdir(dbLocation)
    pipe_stdin, pipe_stdout = os.popen2('find bin -name "*" -type f')
    lines = pipe_stdout.readlines()
    pipe_stdin.close()
    pipe_stdout.close()
   
    fileList = ['lib/db.jar']
    fileList.extend(lines)

    findCmd = ''
    if getPlatform() == 'solaris':
	findCmd = 'find lib -name "*'  + getPlatformLibExtension() + '" -type f -maxdepth 1'
    elif getPlatform() == 'macosx':
	findCmd = 'find lib \( -name "*'  + getPlatformLibExtension() + '" -or -name "*jnilib" \) -type f '
    else:
	findCmd = 'find lib -name "*'  + getPlatformLibExtension() + '" -type f'
    pipe_stdin, pipe_stdout = os.popen2(findCmd)
    lines = pipe_stdout.readlines()
    pipe_stdin.close()
    pipe_stdout.close()
    fileList.extend(lines)
    if getPlatform() == 'aix':
	fileList.append('lib/libdb_cxx.so')	

    os.chdir(cwd)
    return fileList

def copyExpatFiles(expatLocation, version):
    cwd = os.getcwd()
    os.chdir(expatLocation)

    fileList = []
    findCmd = ''
    if getPlatform() == 'solaris':
	findCmd = 'find lib -name "*'  + getPlatformLibExtension() + '" -type f -maxdepth 1'
    else:
	findCmd = 'find lib -name "*'  + getPlatformLibExtension() + '" -type f'
    pipe_stdin, pipe_stdout = os.popen2(findCmd)
    lines = pipe_stdout.readlines()
    pipe_stdin.close()
    pipe_stdout.close()
    fileList.extend(lines)

    linkList = []
    findCmd = ''
    if getPlatform() == 'solaris':
	findCmd = 'find lib -name "*'  + getPlatformLibExtension() + '" -type l -maxdepth 1'
    else:
	findCmd = 'find lib -name "*'  + getPlatformLibExtension() + '" -type l'
    pipe_stdin, pipe_stdout = os.popen2(findCmd)
    lines = pipe_stdout.readlines()
    pipe_stdin.close()
    pipe_stdout.close()
    fileList.extend(lines)

    for i in lines:
	if i != 'libexpat.' + getPlatformLibExtension():
	    linkList.append(i)

    os.chdir(cwd)

    if not os.path.exists('Ice-' + version + '/' + fileList[0].strip()):
	shutil.copy(expatLocation + '/' + fileList[0].strip(), 'Ice-' + version + '/' + fileList[0].strip())
	os.symlink(os.path.basename(fileList[0].strip()), 'Ice-' + version + '/' + linkList[0].strip())

def makePHPbinary(sources, buildDir, installDir, version, clean):
    """ Create the IcePHP binaries and install to Ice installation directory """

    platform = getPlatform()
    if not platform in ['linux', 'macosx']:
        return         
        
    #
    # We currently run configure each time even if clean=false.  This is
    # because a large part of the IcePHP build process is actually the
    # configure step.  This could probably be bypassed afterwards.
    #
    phpMatches = glob.glob(sources + '/php*.tar.[bg]z*')
    if len(phpMatches) == 0:
	print 'Unable to find PHP source tarball in %s' % sources
    	sys.exit(1)

    phpFile = ''
    phpVersion = ''
    #
    # There is more than one php archive in the sources directory.  Try
    # and determine which one is the newest version.  If you want a
    # specific version its best to remove the other distributions. 
    #
    if len(phpMatches) > 1:
	#
	# We need to decide which file to use.
	#
	newest = 0
	for f in phpMatches:
	    m = re.search('([0-9]+)\.([0-9]+)\.([0-9]?).*', f)
            verString = ''
	    for gr in m.groups():
		verString = verString + gr
		if len(phpVersion) == 0:
		    phpVersion = gr
		else:
		    phpVersion = phpVersion + '.'  + gr

	    #
	    # We want to make sure that version string have the same
	    # number of significant digits no matter how many version
	    # number components there are.
	    # 
	    while len(verString) < 5:
		verString = verString + '0'

	    intVersion = int(verString)
	    if intVersion > newest:
		phpFile = f
		newest = intVersion
    else:
	phpFile = phpMatches[0]
	m = re.search('([0-9]+)\.([0-9]+)\.([0-9]?).*', phpFile)

	for gr in m.groups():
	    if len(phpVersion) == 0:
		phpVersion = gr
	    else:
		phpVersion = phpVersion + '.'  + gr

    logging.info('Using PHP archive :' + phpFile)
    root, ext = os.path.splitext(phpFile)
    untarCmd = ''
    if ext.endswith('bz2'):
	untarCmd = 'bunzip2 -dc '
    else:
	untarCmd = 'gzip -dc '

    origWD = os.getcwd()
    os.chdir(buildDir)
    untarCmd = untarCmd + phpFile + ' | tar xf -  ' 
    os.system(untarCmd)
    os.chdir(origWD)

    # 
    # return the likely root directory name for the php distro.
    #
    phpDir = buildDir + '/php-' + phpVersion
    os.system('ln -sf ' + buildDir + '/IcePHP-' + version + '/src/ice ' + phpDir + '/ext')
    cwd = os.getcwd()
    os.chdir(phpDir)
    platform = getPlatform()

    if platform == 'solaris':
	os.environ['CC'] = 'cc'
	os.environ['CXX'] = 'CC'

    if platform == 'hpux':
	os.system('gzip -dc ' + buildDir + '/IcePHP-' + version + '/configure-hpux.gz > configure')
    elif platform == 'linux':
	os.system('gzip -dc ' + buildDir + '/ice/install/thirdparty/php/configure*.gz > configure')
    else:
	os.system('gzip -dc ' + buildDir + '/IcePHP-' + version + '/configure.gz > configure')

    if platform == 'hpux':
	#
	# Our HP-UX platform doesn't seem to have a libxml installed.
	#
	os.system('./configure --disable-libxml --with-ice=shared,' + installDir + '/Ice-' + version)
    else:
	#
	# Everything else should be dynamic and pretty much basic.
	#
	os.system('./configure --with-ice=shared,' + installDir + '/Ice-' + version)

    # 
    # Need to make some changes to the PHP distribution.
    #
    
    #
    # libtool changes
    #
    # This is almost universally an LD => CXX conversion. 
    #
    if not platform.startswith('linux'):
	if platform in ['solaris', 'macosx']:
	    libtool = fileinput.input('libtool', True)
	    for line in libtool :
		if line.startswith('archive_cmds='):
		    if platform == 'macosx':
			print line.replace('\$CC', '\$CXX').rstrip('\n')
		    else:
			print line.replace('\$LD', '\$CXX').rstrip('\n')
		else:
		    print line.rstrip('\n')

	    libtool.close()

    #
    # Makefile changes
    #
    xtraCXXFlags = True
    if platform.startswith('linux'):
        makefile = fileinput.input('Makefile', True)
        for line in makefile:
            if line.startswith('EXTRA_CXXFLAGS ='):
                xtraCXXFlags = False
                print line.rstrip('\n') + ' -DCOMPILE_DL_ICE'
            else:
                print line.strip('\n')

        makefile.close()

    elif platform == 'macosx':
        replacingCC = False
        makefile = fileinput.input('Makefile', True)
        for line in makefile: 
            if not replacingCC:
                if line.startswith('EXTRA_CXXFLAGS ='):
                    xtraCXXFlags = False
                    print line.rstrip('\n') + ' -DCOMPILE_DL_ICE'
                elif line.find('BUILD_CLI') != -1:
                    print line.replace('$(CC)', '$(CXX)').rstrip('\n')
                elif line.find('BUILD_CGI') != -1:
                    print line.replace('$(CC)', '$(CXX)').rstrip('\n')
                elif line.startswith('libphp5.la:'):
                    replacingCC = True
                    print line.strip('\n')
                elif line.startswith('libs/libphp5.bundle:'):
                    replacingCC = True
                    print line.strip('\n')
                else:
                    print line.rstrip('\n')
            else:
                if len(line.rstrip('\n').strip()) == 0:
                    replacingCC = False
                    print 
                else:
                    print line.strip('\n'). replace('$(CC)', '$(CXX)')

        makefile.close()
        
    if xtraCXXFlags:
        makefile = fileinput.input('Makefile', True)
        start = True
        for line in makefile:
            if start:
                print 'EXTRA_CXXFLAGS = -DCOMPILE_DL_ICE'
                start = False
            else:
                print line.rstrip('\n')
                
    makefile.close()

    os.system('gmake')

    if platform == 'macosx':
        phpModuleExtension = '.so'
    else:
        phpModuleExtension = getPlatformLibExtension()
        
    moduleName = '%s/modules/ice%s' % (phpDir, phpModuleExtension)
    shutil.copy(moduleName, '%s/Ice-%s/lib/icephp%s' % (installDir, version, phpModuleExtension))
        
    os.chdir(cwd)

def usage():
    """Print usage/help information"""
    print 'Usage: ' + sys.argv[0] + ' [options] [tag]'
    print
    print 'Options:'
    print '-h                     Show this message.'
    print '--build-dir=[path]     Specify the directory where the distributions'
    print '                       will be unpacked and built.'
    print '--install-dir=[path]   Specify the directory where the distribution'
    print '                       contents will be installed to.'
    print '--install-root=[path]  Specify the root directory that will appear'
    print '                       in the tarball.'
    print '--sources=[path]       Specify the location of the sources directory.'
    print '                       If this is omitted makebindist will traverse'
    print '                       ../icej ../icepy ../icecs, etc and make the'
    print '                       distributions for you.'
    print '-v, --verbose          Print verbose processing messages.'
    print '-t, --tag              Specify the CVS version tag for the packages.'
    print '--noclean              Do not clean up current sources where'
    print '                       applicable (some bits will still be cleaned.'
    print '--nobuild              Run through the process but don\'t build'
    print '                       anything new.'
    print '--specfile             Just print the RPM spec file and exit.'
    print '--usecvs		  Use contents of existing CVS directories'
    print '                       to create binary package (This option cannot'
    print '                       be used to create RPMS)'
    print 
    print 'The following options set the locations for third party libraries'
    print 'that may be required on your platform.  Alternatively, you can'
    print 'set these locations using environment variables of the form.  If'
    print 'you do not set locations through the enviroment or through the '
    print 'command line, default locations will be used (system defaults +'
    print 'the default locations indicated).'
    print 
    print 'LIBRARY_HOME=[path to library]'
    print
    print 'e.g. for bzip2'
    print 
    print 'export BZIP2_HOME=/opt/bzip2-1.0.3'
    print 
    print '--stlporthome=[path]   Specify location of the STLPort libraries, '
    print '                       if required.'
    print '--bzip2home=[path]     Specify location of the bzip2 libraries '
    print '                       (default=/opt/bzip2).'
    print '--dbhome=[path]        Specify location of Berkeley DB'
    print '                       (default=/opt/db).'
    print '--sslhome=[path]       Specify location of OpenSSL'
    print '                       (default=/opt/openssl).'
    print '--expathome=[path]	  Specify location of expat libraries '
    print '                       (default=/opt/expat).'
    print '--readlinehome=[path]  Specify readline library and location '
    print '                       (defaults to /opt/readline if set).'
    print
    print 'If no tag is specified, HEAD is used.'

def main():

    buildEnvironment = dict()
    buildDir = None
    installDir = None
    sources = None
    installRoot = None
    verbose = False
    cvsTag = 'HEAD'
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
        optionList, args = getopt.getopt(sys.argv[1:], 'hvt:',
                                         [ 'build-dir=', 'install-dir=', 'install-root=', 'sources=',
                                           'verbose', 'tag=', 'noclean', 'nobuild', 'specfile',
					   'stlporthome=', 'bzip2home=', 'dbhome=', 'sslhome=',
					   'expathome=', 'readlinehome=', 'usecvs'])
               
    except getopt.GetoptError:
        usage()
        sys.exit(2)

    for o, a in optionList:
        if o == '--build-dir':
            buildDir = a
        elif o == '--install-dir':
            installDir = a
        elif o == '--install-root':
            installRoot = a
        elif o == '--sources':
            sources = a
        elif o in ('-h', '--help'):
            usage()
            sys.exit()
        elif o in ('-v', '--verbose'):
            verbose = True
        elif o in ('-t', '--tag'):
            cvsTag = a
        elif o == '--noclean':
            clean = False
        elif o == '--nobuild':
            build = False
        elif o == '--specfile':
            printSpecFile = True
	elif o == '--stlporthome':
	    buildEnvironment['STLPORT_HOME'] = a
	elif o == '--bzip2home':
	    buildEnvironment['BZIP2_HOME'] = a
	elif o == '--dbhome':
	    buildEnvironment['DB_HOME'] = a
	elif o == '--sslhome':
	    buildEnvironment['OPENSSL_HOME'] = a
	elif o == '--expathome':
	    buildEnvironment['EXPAT_HOME'] = a
	elif o == '--readlinehome':
	    buildEnvironment['READLINE_HOME'] = a
	elif o == '--usecvs':
	    cvsMode = True

    if verbose:
	logging.getLogger().setLevel(logging.DEBUG)

    #
    # Configure environment.
    #
    if getPlatform() == 'aix':
	dylibEnvironmentVar = 'LIBPATH'
    elif getPlatform() == 'hpux':
	dylibEnvironmentVar = 'SHLIB_PATH'
    else:
	dylibEnvironmentVar = 'LD_LIBRARY_PATH'

    for k, v in buildEnvironment.iteritems():
	os.environ[k] = v
	if os.environ.has_key(dylibEnvironmentVar):
	    os.environ[dylibEnvironmentVar] = v + '/lib:' + os.environ[dylibEnvironmentVar] 

    if buildDir == None:
        print 'No build directory specified, defaulting to $HOME/tmp/icebuild'
        buildDir = os.environ.get('HOME') + '/tmp/icebuild'

    if cvsMode:
	print 'Using CVS mode'

    if installDir == None:
        print 'No install directory specified, default to $HOME/tmp/iceinstall'
        installDir = os.environ.get('HOME') + '/tmp/iceinstall'

    #
    # We need to clean the directory out to keep obsolete files from
    # being installed.  This needs to happen whether we are running with
    # noclean or not.
    #
    if build:
        if os.path.exists(installDir):
            shutil.rmtree(installDir, True)

    #
    # In CVS mode we are relying on the checked out CVS sources *are*
    # the build sources.
    #
    if cvsMode:
	directories = []
    else:
	directories = [buildDir, buildDir + '/sources', buildDir + '/demotree']

    directories.append(installDir)

    for d in directories:
        initDirectory(d)

    #
    # Determine location of binary distribution-only files.
    #
    installFiles = None
    if cvsMode:
	version = getIceVersion('include/IceUtil/Config.h')
	soVersion = getIceSoVersion('include/IceUtil/Config.h')
	installFiles = 'install'
    else:
	version, soVersion = getVersion(cvsTag, buildDir)
	installFiles = getInstallFiles(cvsTag, buildDir, version)

    if verbose:
        print 'Building binary distributions for Ice-' + version + ' on ' + getPlatform()
        print 'Using build directory: ' + buildDir
        print 'Using install directory: ' + installDir
        if getPlatform().startswith('linux'):
            print '(RPMs will be built)'
        print

    #
    # Primarily for debugging spec file creation.
    #
    if printSpecFile:
	createRPMSFromBinaries(sys.stdout, installDir, version, soVersion)
        sys.exit(0)

    if not cvsMode:
	#
	# These last build directories will have to wait until we've got
	# the version number for the distribution.
	#
	shutil.rmtree(buildDir + '/Ice-' + version + '-demos', True)
	initDirectory(buildDir + '/Ice-' + version + '-demos/config')

    if build and not cvsMode:
        collectSources = False
        if sources == None:
            sources = buildDir + '/sources'
            collectSources = clean

        #
        # Ice must be first or building the other source distributions will fail.
        #
        sourceTarBalls = [ ('ice', 'Ice-' + version, ''),
			   ('icephp','IcePHP-' + version, 'php'),
                           ('icej','IceJ-' + version, 'j') ]

	if not getPlatform() in ['aix', 'linux64']:
	    sourceTarBalls.append(('icepy','IcePy-' + version, 'py'))

	if getPlatform() == 'linux':
	    sourceTarBalls.append(('icecs','IceCS-' + version, 'cs'))

        os.environ['ICE_HOME'] = installDir + '/Ice-' + version
        currentLibraryPath = None
        try:
            currentLibraryPath = os.environ[dylibEnvironmentVar] 
        except KeyError:
            currentLibraryPath = ''

        os.environ[dylibEnvironmentVar] = installDir + '/Ice-' + version + '/lib:' + currentLibraryPath
        os.environ['PATH'] = installDir + '/Ice-' + version + '/bin:' + os.environ['PATH']

        for cvs, tarball, demoDir in sourceTarBalls:
            if collectSources:
                collectSourceDistributions(cvsTag, sources, cvs, tarball)
	    if getPlatform() == 'linux':
		extractDemos(sources, buildDir, version, tarball, demoDir)
		shutil.copy(installFiles + '/unix/README.DEMOS', buildDir + '/Ice-' + version + '-demos/README.DEMOS') 
            makeInstall(sources, buildDir, installDir + '/Ice-' + version, tarball, clean, version)

        #
        # Pack up demos
        #
	if getPlatform() == 'linux':
	    archiveDemoTree(buildDir, version, installFiles)
	    shutil.move(buildDir + '/Ice-' + version + '-demos.tar.gz', installDir + '/Ice-' + version + 
		    '-demos.tar.gz')

    elif cvsMode:
	collectSources = False

	#
	# TODO: Sanity check to make sure that the script is being run
	# from a location that it expects.
	#
	cvsDirs = [ 'ice', 'icej', 'icepy', 'icephp' ]
	if getPlatform() == 'linux':
	    cvsDirs.append('icecs')

        os.environ['ICE_HOME'] = os.getcwd()  
        currentLibraryPath = None
        try:
            currentLibraryPath = os.environ[dylibEnvironmentVar] 
        except KeyError:
            currentLibraryPath = ''

        os.environ[dylibEnvironmentVar] = installDir + '/Ice-' + version + '/lib:' + currentLibraryPath
        os.environ['PATH'] = installDir + '/Ice-' + version + '/bin:' + os.environ['PATH']

	for d in cvsDirs:
	    currentDir = os.getcwd()
	    os.chdir('../' + d)
	    print 'Going to directory ' + d
	    if d == 'icej':
		shutil.copy('lib/Ice.jar', installDir +'/Ice-' + version + '/lib')
		os.system('cp -pR ant ' + installDir + '/Ice-' + version)
		os.system('find ' + installDir + '/Ice-' + version + ' -name "*.java" | xargs rm')
	    else:
		os.system('perl -pi -e "s/^prefix.*$/prefix = \$\(INSTALL_ROOT\)/" config/Make.rules')
		os.system('gmake INSTALL_ROOT=' + installDir + '/Ice-' + version + ' install')
	    os.chdir(currentDir)

    #
    # Sources should have already been built and installed.  We
    # can pick the binaries up from the iceinstall directory.
    #
    binaries = glob.glob(installDir + '/Ice-' + version + '/bin/*')
    binaries.extend(glob.glob(installDir + '/Ice-' + version + '/lib/*' + shlibExtensions(version, soVersion)[0]))
    cwd = os.getcwd()
    os.chdir(installDir)
    if not getPlatform().startswith('linux'):
	#
	# Get third party libraries.
	#
	dbLocation = os.environ['DB_HOME']
	dbFiles = getDBfiles(dbLocation)
	for f in dbFiles:
            if not os.path.exists('Ice-' + version + '/' + f.strip()):
                shutil.copy(dbLocation + '/' + f.strip(), 'Ice-' + version + '/' + f.strip())

    if getPlatform() == 'macosx':
	copyExpatFiles(os.environ['EXPAT_HOME'], version)	

    if getPlatform() == 'hpux':
	ssl = os.environ['OPENSSL_HOME']
	os.system('cp ' + ssl + '/bin/* Ice-' + version + '/bin')
	os.system('cp -R ' + ssl + '/include/* Ice-' + version + '/include')
	os.system('cp -R ' + ssl + '/lib/* Ice-' + version + '/lib')
	os.system('rm -rf Ice-' + version + '/lib/libfips*')

    uname = readcommand('uname')
    platformSpecificFiles = [ 'README', 'SOURCES', 'THIRD_PARTY_LICENSE' ]
    for psf in platformSpecificFiles:
	cf = os.path.join(installFiles, 'unix', psf + '.' + uname)
	if os.path.exists(cf):
	    shutil.copy(cf, os.path.join('Ice-' + version, psf))

    shutil.copy(installFiles + os.path.join(installFiles, 'common', 'iceproject.xml'))

    makePHPbinary(sources, buildDir, installDir, version, clean)

    os.system('tar cf Ice-' + version + '-bin-' + getPlatform() + '.tar Ice-' + version)
    os.system('gzip -9 Ice-' + version + '-bin-' + getPlatform() + '.tar')
    os.chdir(cwd)

    #
    # If we are running on Linux, we need to create RPMs.  This will
    # probably blow up unless the user that is running the script has
    # massaged the permissions on /usr/src/redhat/.
    #
    if getPlatform().startswith('linux') and not cvsMode:
	shutil.copy(installFiles + '/unix/README.Linux-RPM', '/usr/src/redhat/SOURCES/README.Linux-RPM')
	shutil.copy(installFiles + '/unix/README.Linux-RPM', installDir + '/Ice-' + version + '/README')
	shutil.copy(installFiles + '/thirdparty/php/ice.ini', installDir + '/Ice-' + version + '/ice.ini')
	if getPlatform() == 'linux64':
	    #
	    # The demo archive isn't constructed on 64 bit linux so we
	    # need to rely on the archive being in the sources
	    # directory.
	    #
	    shutil.copy(sources + '/Ice-' + version + '-demos.tar.gz', '/usr/src/redhat/SOURCES')
	    shutil.copy(sources + '/Ice-' + version + '-demos.tar.gz ', installDir)
            iceArchives = glob.glob(sources + '/Ice*' + version + '.gz')
	    for f in iceArchives:
		shutil.copy(f, 'usr/src/redhat/SOURCES')
	    RPMTools.createRPMSFromBinaries64(buildDir, installDir, version, soVersion)
	else:
	    shutil.copy(installDir + '/Ice-' + version + '-demos.tar.gz', '/usr/src/redhat/SOURCES')
	    shutil.copy(sources + '/php-5.0.4.tar.bz2', '/usr/src/redhat/SOURCES')
	    shutil.copy(installFiles + '/thirdparty/php/ice.ini', '/usr/src/redhat/SOURCES')
	    shutil.copy(installFiles + '/thirdparty/php/configure.5.0.4.gz', 
		    '/usr/src/redhat/SOURCES')
            iceArchives = glob.glob(sources + '/Ice*' + version + '.gz')
	    for f in iceArchives:
		shutil.copy(f, 'usr/src/redhat/SOURCES')
	    RPMTools.createRPMSFromBinaries(buildDir, installDir, version, soVersion)

    #
    # TODO: Cleanups?  I've left everything in place so that the process
    # can be easily debugged.
    #

if __name__ == "__main__":
    main()
