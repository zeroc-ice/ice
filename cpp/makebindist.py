#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
#  
#  XXX: runprog provides a way for to get fail on unexpected error codes from
#  external programs. I haven't figured out how to get commands with
#  pipes or redirections to work properly. Stay tuned.
#

class ExtProgramError:
    def __init__(self, error = None):
	self.msg = error
	
    def __str__(self):
	return repr(self.msg)

def runprog(commandstring, haltOnError = True):
    #commandtuple = commandstring.split()
    #result = os.spawnvpe(os.P_WAIT, commandtuple[0], commandtuple, os.environ)
    result = os.system(commandstring)
    if result != 0:
	msg = 'Command %s returned error code %d' % (commandstring, result)
	if haltOnError: 
	    raise ExtProgramError(msg)

def copyfiles(srcDir, destDir):
    '''Copy the contents of one directory to another (non-recursive)'''
    for f in os.listdir(srcDir):
	src = os.path.join(srcDir, f)
	dest = os.path.join(destDir, f)
	if not os.path.isdir(src) and not os.path.islink(src):
	    shutil.copy(src, dest)

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
    runprog('cvs -d cvs.zeroc.com:/home/cvsroot export -r ' + cvsTag + ' ice/include/IceUtil/Config.h')

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
    try:
	os.chdir(buildDir)
	runprog('rm -rf ' + buildDir + '/ice/install')
	runprog('cvs -d cvs.zeroc.com:/home/cvsroot export -r ' + cvsTag + ' ice/install/unix')
	runprog('cvs -d cvs.zeroc.com:/home/cvsroot export -r ' + cvsTag + ' ice/install/common')
	runprog('cvs -d cvs.zeroc.com:/home/cvsroot export -r ' + cvsTag + ' ice/install/rpm')
	runprog('cvs -d cvs.zeroc.com:/home/cvsroot export -r ' + cvsTag + ' ice/install/thirdparty')
	snapshot = os.walk('./ice/install/unix')
	for dirInfo in snapshot:
	    for f in dirInfo[2]:
		fixVersion(os.path.join(dirInfo[0], f), version)
    finally:
	os.chdir(cwd)
    return buildDir + '/ice/install'

def getInstallFilesFromLocalDirectory(cvsTag, buildDir, version):
    '''Gets the install files from an existing CVS directory, has the
    advantage of working even if CVS is down allowing the install-O to
    continue working!'''
    cwd = os.getcwd()
    try:
	os.chdir(buildDir)
	target = os.path.join(buildDir, 'ice', 'install')
	if os.path.exists(target):
	    shutil.rmtree(target)
	iceloc = os.path.abspath(os.path.join(os.environ['ICE_HOME'],'install'))
	os.makedirs(target)
	for f in ['unix', 'common', 'rpm', 'thirdparty']:
	    shutil.copytree(os.path.join(iceloc, f), os.path.join(target, f)) 
	snapshot = os.walk('./ice/install/unix')
	for dirInfo in snapshot:
	    for f in dirInfo[2]:
		fixVersion(os.path.join(dirInfo[0], f), version)
    finally:
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

    # 
    # The sources collected by the makebindist.py script are *NOT*
    # suitable for release as they do not all contain the documentation.
    #
    if cvsdir in ["ice", "icephp"]:
        runprog("./makedist.py " + tag)
    else:
        runprog("./makedist.py -d " + tag)
	
    shutil.copy("dist/" + distro + ".tar.gz", sourceDir)
    os.chdir(cwd)

def editMakeRulesCS(filename, version):
    '''
    The make rules in the C# distribution need some simple tweaking to
    make them suitable for inclusion in the demo distribution.
    '''
    reIceLocation = re.compile('^[a-z]*dir.*=\s*\$\(top_srcdir\)')
    makefile = fileinput.input(filename, True)
    for line in makefile:
	if reIceLocation.search(line) <> None:
	    print line.rstrip('\n').replace('top_srcdir', 'ICE_DIR', 10)
	elif line.startswith('prefix'):
	    print 'prefix = $(ICE_DIR)'
	else:
	    print line.rstrip('\n')
    makefile.close()

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
		output = line.rstrip('\n').replace('top_srcdir', 'ICE_DIR', 10)
		if line.startswith('libdir'):
		    print 'ifeq ($(LP64),yes)'
		    print '    ' + output + '$(lp64suffix)'
		    print 'else'
		    print '    '  + output
		    print 'endif'
		elif line.startswith('bindir'):
		    print output
		    #
		    # NOTE!!! Magic occurs...
		    # It simplifies building the demos for the user if
		    # we try to detect whether the executables are 64
		    # bit and, if so, set LP64=yes automagically.
		    #
		    print ''
		    print '#'
		    print '# If LP64 is unset, sample an Ice executable to see if it is 64 bit'
		    print '# and set automatically. This avoids having to set LP64 if there is'
		    print '# Ice installation in a well-known location.'
		    print '#'
		    print 'ifeq ($(LP64),)'
		    print '    ifneq ($(shell file $(bindir)/slice2cpp | grep 64-bit),)'
		    print '        LP64=yes'	    
		    print '    endif'
		    print 'endif'
		    print ''
		else:
		    print output

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
    ifneq ($(shell test -f $(ICE_DIR)/bin/icestormadmin && echo 0),0)
$(error Ice distribution not found, please set ICE_HOME!)
    endif
else
    ICE_DIR = $(ICE_HOME)
    ifneq ($(shell test -d $(ICE_DIR)/slice && echo 0),0)
$(error Ice distribution not found, please set ICE_HOME!)
    endif
endif

prefix = $(ICE_DIR)

"""
	elif state == 'untilprefix':
	    if line.startswith('prefix'):
		state = 'done'
    #
    # Dependency files are all going to be bogus.  The makedepend
    # script doesn't seem to work properly for the slice files.
    #
    os.chdir("..")
    runprog("sh -c 'for f in `find . -name .depend` ; do echo \"\" > $f ; done'")
    makefile.close()

def updateIceVersion(filename, version):
    print 'Updating ice version in ' + filename + ' to ' + version
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

    #
    # TODO: Some archives don't contain all of these elements. It might
    # be nicer to make the toExtract list more tailored for each
    # distribution.
    #
    toExtract = "%s/demo %s/config %s/certs" % (distro, distro, distro)
	
    runprog("gzip -dc " + sources + "/" + distro + ".tar.gz | tar xf - " + toExtract, False)
	
    shutil.move(distro + "/demo", buildDir + "/Ice-" + version + "-demos/demo" + demoDir)

    #
    # 'System' copying of files here because its just easier!  We don't
    # need any configuration out of the Python tree.
    # 
    if not os.path.exists(buildDir + "/Ice-" + version + "-demos/certs"):
	os.mkdir(buildDir + "/Ice-" + version + "-demos/certs")
	
    if not os.path.exists(buildDir + "/Ice-" + version + "-demos/config"):
	os.mkdir(buildDir + "/Ice-" + version + "-demos/config")

    if os.path.exists('%s/certs' % distro):
	runprog("cp -pR " + distro + "/certs " + buildDir + "/Ice-" + version + "-demos")

    srcConfigDir = '%s/%s/config' % (os.getcwd(), distro)
    destConfigDir = '%s/Ice-%s-demos/config' % (buildDir, version)

    if not demoDir in ['py', 'vb'] and os.path.exists(srcConfigDir):
	for f in os.listdir(srcConfigDir):
	    src = os.path.join(srcConfigDir, f)
	    dest = os.path.join(destConfigDir, f)
	    if not os.path.isdir(f) and not os.path.islink(f):
		shutil.copy(src, dest)

    if demoDir == 'j':
	updateIceVersion(os.path.join(destConfigDir, 'common.xml'), version)

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
	editMakeRulesCS(os.path.join(basepath, 'Make.rules.cs'), version)

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
    runprog("sh -c 'for f in `find Ice-" + version + "-demos/demoj -name classes -type d` ; do rm -rf $f/* ; done'")

    #
    # Remove generated source files.
    #
    runprog("sh -c 'for f in `find Ice-" + version + "-demos/demoj -name generated -type d` ; do rm -rf $f/* ; done'")

    #
    # Remove Windows project files.
    #
    runprog("sh -c 'for f in `find Ice-" + version + "-demos -name \"*\.dsp\" ` ; do rm -rf $f ; done'")
    runprog("sh -c 'for f in `find Ice-" + version + "-demos -name \"*\.dsw\" ` ; do rm -rf $f ; done'")
    runprog("sh -c 'for f in `find Ice-" + version + "-demos/democs -name \"*.sln\" ` ; do rm -rf $f ; done'")
    runprog("sh -c 'for f in `find Ice-" + version + "-demos/democs -name \"*.csproj\" ` ; do rm -rf $f ; done'")

    runprog("tar cf Ice-" + version + "-demos.tar Ice-" + version + "-demos")
    runprog("gzip -9 Ice-" + version + "-demos.tar")
    os.chdir(cwd)

def makeInstall(sources, buildDir, installDir, distro, clean, version):
    """Make the distro in buildDir sources and install it to installDir."""
    cwd = os.getcwd()
    os.chdir(buildDir)
    if clean:
        shutil.rmtree(distro, True)
        
    if not os.path.exists(distro):
	filename = sources + '/' + distro + '.tar'
        runprog('gzip -d %s.gz' % filename)
	runprog('tar xf %s' % filename)
        runprog('gzip -9 %s' % filename)
        
    os.chdir(distro)

    #
    # Java does not have a 'make install' process, but comes complete
    # with the Jar already built.
    # 
    if distro.startswith('IceJ'):
	if not os.path.exists(os.path.join(installDir, 'lib')):
	    os.mkdir(os.path.join(installDir, 'lib'))
	if not os.path.exists(os.path.join(installDir, 'lib', 'java5')):
	    os.mkdir(os.path.join(installDir, 'lib', 'java5'))
	shutil.copy(buildDir + '/' + distro + '/lib/Ice.jar', installDir + '/lib')
	shutil.copy(buildDir + '/' + distro + '/lib/IceGridGUI.jar', installDir + '/lib')
	shutil.copy(buildDir + '/' + distro + '/lib/java5/Ice.jar', installDir + '/lib/java5')
	#
	# We really just want to copy the files, not move them.
	# Shelling out to a copy is easier (and more likely to always
	# work) than shutil.copytree().
	#
	runprog('cp -pR ' + buildDir + '/' + distro + '/ant ' + installDir)
	runprog('find ' + installDir + '/ant  -name "*.java" | xargs rm')
	destDir = os.path.join(installDir, 'config')
	if not os.path.exists(destDir):
	    os.mkdir(destDir)
        shutil.copy(os.path.join('config', 'build.properties'), destDir)
        os.chdir(cwd)
        return

    if distro.startswith('IcePHP'):
        os.chdir(cwd)
	return

    if distro.startswith('IceCS'):
	runprog('perl -pi -e \'s/^prefix.*$/prefix = \$\(INSTALL_ROOT\)/\' config/Make.rules.cs')
    else:
	runprog('perl -pi -e \'s/^prefix.*$/prefix = \$\(INSTALL_ROOT\)/\' config/Make.rules')

    if distro.startswith('IcePy'):
        try:
            pyHome = os.environ['PYTHON_HOME']
        except KeyError:
            pyHome = None
            
        if pyHome == None or pyHome == '':
            logging.info('PYTHON_HOME is not set, figuring it out and trying that')
            pyHome = sys.exec_prefix
            
        runprog("perl -pi -e 's/^PYTHON.HOME.*$/PYTHON\_HOME \?= "+ pyHome.replace("/", "\/") + \
		"/' config/Make.rules")
        
    if not getPlatform().startswith('linux'):
	if distro.startswith('IcePy'):
	    runprog("perl -pi -e 's/^PYTHON.INCLUDE.DIR.*$/PYTHON_INCLUDE_DIR = " +
	              "\$\(PYTHON_HOME\)\/include\/\$\(PYTHON_VERSION\)/' config/Make.rules")
	    runprog("perl -pi -e 's/^PYTHON.LIB.DIR.*$/PYTHON_LIB_DIR = " + 
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
    try:
	runprog('gmake NOGAC=yes OPTIMIZE=yes INSTALL_ROOT=/opt/Ice-%s' % version)
	runprog('gmake NOGAC=yes OPTIMIZE=yes INSTALL_ROOT=%s install' % installDir)
    except ExtProgramError:
	print "gmake failed for makeInstall(%s, %s, %s, %s, %s, %s)" % (sources, buildDir, installDir, distro, str(clean), version) 
	raise

    if distro.startswith('IceCS'):
	assemblies = ["glacier2cs", "iceboxcs", "icecs", "icegridcs", "icepatch2cs", "icestormcs"]

	# 
	# We are relying on pkgconfig to deal with some mono library
	# configuration, so we need to copy these files into place.
	#
	if not os.path.exists("%s/lib/pkgconfig" % installDir):
	    os.makedirs("%s/lib/pkgconfig" % installDir)

	for a in assemblies:
	    shutil.copy("bin/%s.dll" % a, "%s/bin/%s.dll" % (installDir, a))
	    shutil.copy("lib/pkgconfig/%s.pc" % a, "%s/lib/pkgconfig" % installDir)

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
    if not platform in ['linux', 'macosx', 'linux64']:
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
	uncompress = 'bunzip2 -d '
	compress = 'bzip2 -9 '
    else:
	uncompress = 'gzip -d '
	compress = 'gzip -9 '

    origWD = os.getcwd()
    os.chdir(buildDir)
    runprog(uncompress + phpFile)
    runprog('tar xf %s' % root)
    runprog(compress + root)
    os.chdir(origWD)

    # 
    # return the likely root directory name for the php distro.
    #
    phpDir = buildDir + '/php-' + phpVersion
    runprog('ln -sf ' + buildDir + '/IcePHP-' + version + '/src/ice ' + phpDir + '/ext')
    cwd = os.getcwd()
    os.chdir(phpDir)
    platform = getPlatform()

    if platform == 'solaris':
	os.environ['CC'] = 'cc'
	os.environ['CXX'] = 'CC'

    if platform == 'hpux':
	runprog('gzip -dc ' + buildDir + '/IcePHP-' + version + '/configure-hpux.gz > configure', False)
#   elif platform.startswith('linux'):
#	runprog('gzip -dc ' + buildDir + '/ice/install/thirdparty/php/configure*.gz > configure', False)
		
    else:
	runprog('gzip -dc ' + buildDir + '/IcePHP-' + version + '/configure.gz > configure', False)

    if platform == 'hpux':
	#
	# Our HP-UX platform doesn't seem to have a libxml installed.
	#
	runprog('./configure --disable-libxml --with-ice=shared,' + installDir + '/Ice-' + version)
    else:
	#
	# Everything else should be dynamic and pretty much basic.
	#
	runprog('./configure --with-ice=shared,' + installDir + '/Ice-' + version)

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
	    elif platform == 'linux64' and line.startswith('ICE_SHARED_LIBADD'):
		print line.rstrip('\n').replace("Ice-%s/lib" % version, "Ice-%s/lib64" % version)
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

    runprog('gmake')

    if platform == 'macosx':
        phpModuleExtension = '.so'
    else:
        phpModuleExtension = getPlatformLibExtension()
        
    moduleName = '%s/modules/ice%s' % (phpDir, phpModuleExtension)
    if platform == 'linux64':
	shutil.copy(moduleName, '%s/Ice-%s/lib64/icephp%s' % (installDir, version, phpModuleExtension))
    else:
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
    offline = False

    #
    # Process args.
    #
    try:
        optionList, args = getopt.getopt(sys.argv[1:], 'hvt:',
                                         [ 'build-dir=', 'install-dir=', 'install-root=', 'sources=',
                                           'verbose', 'tag=', 'noclean', 'nobuild', 'specfile',
					   'stlporthome=', 'bzip2home=', 'dbhome=', 'sslhome=',
					   'expathome=', 'readlinehome=', 'usecvs', 'offline'])
               
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
	elif o == '--offline':
	    offline = True
	elif o == '--usecvs':
	    cvsMode = True

    if verbose:
	logging.getLogger().setLevel(logging.DEBUG)

    if offline and sources == None:
	logging.error("You must specify a location for the sources if running in offline mode")
	sys.exit(1)

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
    # Primarily for debugging spec file creation.
    #
    if printSpecFile:
	version, soVersion = getVersion(cvsTag, buildDir)
	RPMTools.createFullSpecFile(sys.stdout, installDir, version, soVersion)
        sys.exit(0)


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
    elif offline:
	version = getIceVersion('include/IceUtil/Config.h')
	soVersion = getIceSoVersion('include/IceUtil/Config.h')
	installFiles = getInstallFilesFromLocalDirectory(cvsTag, buildDir, version)
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

	if not getPlatform() in ['aix']:
	    sourceTarBalls.append(('icepy','IcePy-' + version, 'py'))

	if getPlatform() == "linux":
	    sourceTarBalls.append(('icecs','IceCS-' + version, 'cs'))

        os.environ['ICE_HOME'] = installDir + '/Ice-' + version
        currentLibraryPath = None
        try:
            currentLibraryPath = os.environ[dylibEnvironmentVar] 
        except KeyError:
            currentLibraryPath = ''

        os.environ[dylibEnvironmentVar] = installDir + '/Ice-' + version + '/lib:' + currentLibraryPath
        os.environ['PATH'] = installDir + '/Ice-' + version + '/bin:' + os.environ['PATH']

	#
	# Collect all of the distributions first. This prevents having
	# to go through costly builds before finding out that one of the
	# distributions doesn't build.
	#
	if collectSources:
	    toCollect = list(sourceTarBalls)
	    toCollect.append(('icevb', 'IceVB-' + version, 'vb'))
	    for cvs, tarball, demoDir in toCollect:
                collectSourceDistributions(cvsTag, sources, cvs, tarball)

	print '''
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
>>                                                                      <<
>>                   Sources have been collected!                       <<
>>                                                                      <<
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<'''

	#
	# Package up demo distribution.
	#
	if getPlatform() == 'linux':
	    toCollect = list(sourceTarBalls)
	    for cvs, tarball, demoDir in toCollect:
		extractDemos(sources, buildDir, version, tarball, demoDir)
		shutil.copy("%s/unix/README.DEMOS" % installFiles, "%s/Ice-%s-demos/README.DEMOS" % (buildDir, version)) 
		shutil.copy("%s/Ice-%s/ICE_LICENSE" % (buildDir, version), "%s/Ice-%s-demos/ICE_LICENSE" % (buildDir, version))
	    archiveDemoTree(buildDir, version, installFiles)
	    shutil.move("%s/Ice-%s-demos.tar.gz" % (buildDir, version), "%s/Ice-%s-demos.tar.gz" % (installDir, version))

	#
	# Everything should be set for building stuff up now.
	#
        for cvs, tarball, demoDir in sourceTarBalls:
            makeInstall(sources, buildDir, "%s/Ice-%s" % (installDir, version), tarball, clean, version)	    

    elif cvsMode:
	collectSources = False

	#
	# TODO: Sanity check to make sure that the script is being run
	# from a location that it expects.
	#
	cvsDirs = [ 'ice', 'icej', 'icephp' ]
	if getPlatform() == 'linux':
	    cvsDirs.append('icecs', 'icepy')

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
		shutil.copy('lib/IceGridGUI.jar', installDir +'/Ice-' + version + '/lib')
		runprog('cp -pR ant ' + installDir + '/Ice-' + version)
		runprog('find ' + installDir + '/Ice-' + version + ' -name "*.java" | xargs rm')
	    else:
		runprog('perl -pi -e "s/^prefix.*$/prefix = \$\(INSTALL_ROOT\)/" config/Make.rules')
		runprog('gmake INSTALL_ROOT=' + installDir + '/Ice-' + version + ' install')
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
	shutil.copy('%s/bin/openssl' % ssl, 'Ice-%s/bin' % version)
	runprog('cp -R ' + ssl + '/include Ice-' + version)
	runprog('cp -R ' + ssl + '/lib Ice-' + version)
	runprog('rm -rf Ice-' + version + '/lib/libfips*')
	runprog('rm -rf Ice-' + version + '/lib/engines')
	runprog('rm -rf Ice-' + version + '/lib/pkgconfig')
	runprog('rm -f Ice-' + version + '/lib/*.a')

    uname = readcommand('uname')
    platformSpecificFiles = [ 'README', 'SOURCES', 'THIRD_PARTY_LICENSE' ]
    for psf in platformSpecificFiles:
	cf = os.path.join(installFiles, 'unix', psf + '.' + uname)
	if os.path.exists(cf):
	    shutil.copy(cf, os.path.join('Ice-' + version, psf))

    shutil.copy(os.path.join(installFiles, 'common', 'iceproject.xml'), os.path.join('Ice-' + version, 'config'))

    makePHPbinary(sources, buildDir, installDir, version, clean)

    runprog('tar cf Ice-' + version + '-bin-' + getPlatform() + '.tar Ice-' + version)
    runprog('gzip -9 Ice-' + version + '-bin-' + getPlatform() + '.tar')
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
	    # I need to pull the pkgconfig files out of the IceCS
	    # archive and place them in the installed lib64 directory.
	    # 

	    cwd = os.getcwd()
	    os.chdir(buildDir)
	    distro = "IceCS-%s" % version
	    shutil.rmtree("IceCS-%s" % version, True)
	    if not os.path.exists(distro):
		filename = os.path.join(sources, '%s.tar.gz' % distro)
		runprog('tar xfz %s' % filename)
	    os.chdir(distro)
	    if not os.path.exists(os.path.join(installDir, 'Ice-%s' % version, 'lib64')):
		os.mkdir(os.path.join(installDir, 'Ice-%s' % version, 'lib64'))
	    shutil.copytree(os.path.join('lib', 'pkgconfig'), os.path.join(installDir, 'Ice-%s' % version, 'lib64', 'pkgconfig'))
	    os.chdir(cwd)


	    #
	    # The demo archive isn't constructed on 64 bit linux so we
	    # need to rely on the archive being in the sources
	    # directory.
	    # 
	    # XXX shutil.copy() has a bug that causes the second copy to
	    # fail... maybe it forgot to close the file in the first
	    # copy? I've changed these to using the external copy for
	    # the time being.
	    #
	    runprog('cp ' + sources + '/Ice-' + version + '-demos.tar.gz /usr/src/redhat/SOURCES')
	    runprog('cp ' + sources + '/Ice-' + version + '-demos.tar.gz '  + installDir)
            iceArchives = glob.glob(sources + '/Ice*' + version + '.gz')
	    for f in iceArchives:
		shutil.copy(f, 'usr/src/redhat/SOURCES')
	    RPMTools.createRPMSFromBinaries64(buildDir, installDir, version, soVersion)
	else:
	    shutil.copy(installDir + '/Ice-' + version + '-demos.tar.gz', '/usr/src/redhat/SOURCES')
	    shutil.copy(sources + '/php-5.1.4.tar.bz2', '/usr/src/redhat/SOURCES')
	    shutil.copy(installFiles + '/thirdparty/php/ice.ini', '/usr/src/redhat/SOURCES')
	    shutil.copy(buildDir + '/IcePHP-' + version + '/configure.gz', 
		    '/usr/src/redhat/SOURCES')
	    shutil.copy(installFiles + '/common/iceproject.xml', '/usr/src/redhat/SOURCES')
            iceArchives = glob.glob(sources + '/Ice*' + version + '*.gz')
	    for f in iceArchives:
		shutil.copy(f, '/usr/src/redhat/SOURCES')
	    RPMTools.createRPMSFromBinaries(buildDir, installDir, version, soVersion)

    #
    # TODO: Cleanups?  I've left everything in place so that the process
    # can be easily debugged.
    #

if __name__ == "__main__":
    main()
