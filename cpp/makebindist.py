#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************
import os, sys, shutil, re, string, getopt, glob, logging, fileinput

# TODO:
# 
#   * Tidying and tracing.
#    * use os.path.join() where appropriate instead of string
#    concatenation.
#   * Python is used in some places for 'sed' like functionality. This
#     could be replaced by Python code.
#  
#  XXX: runprog provides a way for to get fail on unexpected error codes from
#  external programs. I haven't figured out how to get commands with
#  pipes or redirections to work properly. Stay tuned.
#
    
def initDirectory(d):
    """Check for the existance of the directory. If it isn't there make
    it."""
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
    return  re.search('ICE_STRING_VERSION \"([0-9\.b]*)\"', config.read()).group(1)

def getIceMMVersion(file):
    """Extract the ICE major.minor version string from a file."""
    config = open(file, 'r')
    return re.search('ICE_STRING_VERSION \"([0-9]+\.[0-9]+)[\.b0-9]*\"', config.read()).group(1)

def getIceSoVersion(file):
    """Extract the ICE version ordinal from a file."""
    config = open(file, 'r')
    mm = re.search('ICE_STRING_VERSION \"([0-9]*\.[0-9b]*)[\.0-9]*\"', config.read()).group(1)
    verComponents = mm.split(".")
    return verComponents[0] + verComponents[1]

def getPlatform():
    """Determine the platform we are building and targetting for"""
    if sys.platform.startswith('win') or sys.platform.startswith('cygwin'):
        return 'win32'
    elif sys.platform.startswith('linux'):
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


def getPlatformString():
    """Determine the platform string used in the .tar.gz names"""
    if getPlatform() == 'linux':
        redhat = os.system('test -f /etc/redhat-release')
        if readcommand('uname -p') == 'x86_64':
            if redhat == 0:
                return 'rhel-x86_64'
            else:
                return 'sles-x86_64'
        else:
            if redhat == 0:
                return 'rhel-i386'
            else:
                return 'sles-i586'
    else:
        return getPlatform()

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

def getVersion(cvsTag, buildDir):
    """Extracts a source file from the repository and gets the version number from it"""
    cwd = os.getcwd()
    os.chdir(buildDir)
    if getPlatform() == 'aix':
        os.environ['LIBPATH'] = ''
    runprog('cvs -d cvs.zeroc.com:/home/cvsroot export -r ' + cvsTag + ' ice/include/IceUtil/Config.h')

    result = [ getIceVersion('ice/include/IceUtil/Config.h'), \
               getIceSoVersion('ice/include/IceUtil/Config.h'), \
               getIceMMVersion('ice/include/IceUtil/Config.h') ]

    os.remove('ice/include/IceUtil/Config.h')
    os.removedirs('ice/include/IceUtil')
    os.chdir(cwd)
    return result

def fixVersion(filename, version, mmVersion):
    f = fileinput.input(filename, True)
    for line in f:
        l = line.rstrip('\n')
        l = l.replace('@ver@', version)
        l = l.replace('@mmver@', mmVersion)
        print l
    f.close()

def getInstallFiles(cvsTag, buildDir, version, mmVersion):
    """Gets the install sources for this revision"""
    cwd = os.getcwd()
    try:
        os.chdir(buildDir)
        runprog('rm -rf ' + buildDir + '/ice/install')
        runprog('cvs -d cvs.zeroc.com:/home/cvsroot export -r ' + cvsTag + ' ice/install/unix')
        runprog('cvs -d cvs.zeroc.com:/home/cvsroot export -r ' + cvsTag + ' ice/install/common')
        runprog('cvs -d cvs.zeroc.com:/home/cvsroot export -r ' + cvsTag + ' ice/install/rpm')
        runprog('cvs -d cvs.zeroc.com:/home/cvsroot export -r ' + cvsTag + ' ice/install/thirdparty')
        fixVersion('ice/install/common/README.DEMOS', version, mmVersion)
        snapshot = os.walk('./ice/install/unix')
        for dirInfo in snapshot:
            for f in dirInfo[2]:
                fixVersion(os.path.join(dirInfo[0], f), version, mmVersion)
    finally:
        os.chdir(cwd)
    return buildDir + '/ice/install'

def getInstallFilesFromLocalDirectory(cvsTag, buildDir, version, mmVersion):
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
        fixVersion('ice/install/common/README.DEMOS', version, mmVersion)
        snapshot = os.walk('./ice/install/unix')
        for dirInfo in snapshot:
            for f in dirInfo[2]:
                fixVersion(os.path.join(dirInfo[0], f), version, mmVersion)
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
    reIceLocation = re.compile('^[a-z]*dir.*=\s*\$\(top_srcdir\)')

    makefile =  fileinput.input(filename, True)
    skipLine = 0
    for line in makefile:
        if (skipLine > 0):
            skipLine = skipLine - 1 
        elif line.startswith('slicedir'):
            print """
ifeq ($(ICE_DIR),/usr)
   slicedir = $(ICE_DIR)/share/Ice-$(VERSION)/slice
else
   slicedir = $(ICE_DIR)/slice
endif
"""
        elif line.startswith('embedded_runpath_prefix'):
            # 
            # embedded_runpath_prefix is moved down to after the version
            # information is set.
            #
            pass
        elif line.startswith('# Select an installation base directory'):
            #
            # Delete this comment
            #
            skipLine = 2
        elif line.startswith('# The "root directory" for runpath'):
            #
            # Delete this comment
            #
            skipLine = 2
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
            # Delete
            #
            pass
        elif line.startswith('SOVERSION'):
            print line.rstrip('\n')
            print """

#
# Checks for ICE_HOME environment variable.  If it isn't present,
# attempt to find an Ice installation in /usr or the default install
# location.
#
ifeq ($(ICE_HOME),)
    ICE_DIR = /usr
    ifneq ($(shell test -f $(ICE_DIR)/bin/icestormadmin && echo 0),0)
        NEXTDIR = /opt/Ice-$(VERSION)
        ifneq ($(shell test -f $(NEXTDIR)/bin/icestormadmin && echo 0),0)
           $(error Unable to locate Ice distribution, please set ICE_HOME!)
        else
            ICE_DIR = $(NEXTDIR)
        endif
    else
        NEXTDIR = /opt/Ice-$(VERSION)
        ifeq ($(shell test -f $(NEXTDIR)/bin/icestormadmin && echo 0),0)
           $(warning Ice distribution found in /usr and $(NEXTDIR)! Installation in "/usr" will be used by default. Use ICE_HOME to specify alternate Ice installation.)
        endif
    endif
else
    ICE_DIR = $(ICE_HOME)
    ifneq ($(shell test -f $(ICE_DIR)/bin/icestormadmin && echo 0),0)
       $(error Ice distribution not found in $(ICE_DIR), please verify ICE_HOME location!)
    endif
endif

ifneq ($(ICE_DIR),/usr)
   embedded_runpath_prefix  ?= /opt/Ice-$(VERSION_MAJOR).$(VERSION_MINOR)
endif

prefix = $(ICE_DIR)

"""
        elif line.startswith('prefix'):
            #
            # Delete prefix line
            #
            pass
        else:
            print line.rstrip('\n')

    #
    # Dependency files are all going to be bogus since they contain relative
    # paths to Ice headers. We need to adjust this.
    #
    os.chdir("..")
    runprog("for f in `find . -name .depend` ; do sed -i -e 's/\.\.\/\.\.\/\.\.\/\.\.\/slice/$(slicedir)/g' $f ; done")
    runprog("for f in `find . -name .depend` ; do sed -i -e 's/\.\.\/\.\.\/\.\.\/\.\./$(ICE_DIR)/g' $f ; done")
    runprog("for f in `find . -name .depend` ; do sed -i -e 's/\.\.\/\.\.\/\.\.\/slice/$(slicedir)/g' $f ; done")
    runprog("for f in `find . -name .depend` ; do sed -i -e 's/\.\.\/\.\.\/\.\./$(ICE_DIR)/g' $f ; done")
    makefile.close()

def editMakeRulesMak(filename, version):
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
            if reIceLocation.search(line) <> None:
                output = line.rstrip('\n').replace('top_srcdir', 'ICE_DIR', 10)
                print output
            elif line.startswith('install_'):
                #
                # Do nothing.
                #
                pass
            elif line.startswith('# If third party'):
                state = 'thirdparty'
            else:
                print line.rstrip('\n')
        elif state == 'header':
            #
            # Reading header.
            #
            print line.rstrip('\n')
            if line.strip() == "":
                state = 'untilprefix'
                print """
#
# Checks for ICE_HOME environment variable.
#

!if "$(ICE_HOME)" == ""
!error Ice distribution not found, please set ICE_HOME!
!endif

ICE_DIR = $(ICE_HOME)
prefix = $(ICE_DIR)
THIRDPARTY_HOME =

"""
        elif state == 'untilprefix':
            if line.startswith('prefix'):
                state = 'done'
        elif state == 'thirdparty':
            if line.startswith('# For VC80 and VC80'):
                print line.rstrip('\n')
                state = 'done'
    #
    # Dependency files are all going to be bogus since they contain relative
    # paths to Ice headers. We need to adjust this
    #
    os.chdir("..")
    runprog("for f in `find . -name .depend` ; do sed -i -e 's/\.\.\/\.\.\/\.\.\/\.\.\/slice/$(slicedir)/g' $f ; done")
    runprog("for f in `find . -name .depend` ; do sed -i -e 's/\.\.\/\.\.\/\.\.\/\.\./$(ICE_DIR)/g' $f ; done")
    runprog("for f in `find . -name .depend` ; do sed -i -e 's/\.\.\/\.\.\/\.\.\/slice/$(slicedir)/g' $f ; done")
    runprog("for f in `find . -name .depend` ; do sed -i -e 's/\.\.\/\.\.\/\.\./$(ICE_DIR)/g' $f ; done")
    makefile.close()

def editMakeRulesMakCS(filename, version):
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
            if reIceLocation.search(line) <> None:
                output = line.rstrip('\n').replace('top_srcdir', 'ICE_DIR', 10)
                print output
            elif line.startswith('install_'):
                #
                # Do nothing.
                #
                pass
            else:
                print line.rstrip('\n')
        elif state == 'header':
            #
            # Reading header.
            #
            print line.rstrip('\n')
            if line.strip() == "":
                state = 'untilprefix'
                print """
#
# Checks for ICE_HOME environment variable.
#

!if "$(ICE_HOME)" == ""
!error Ice distribution not found, please set ICE_HOME!
!endif

ICE_DIR = $(ICE_HOME)
prefix = $(ICE_DIR)

"""
        elif state == 'untilprefix':
            if line.startswith('prefix'):
                state = 'done'

    makefile.close()

def updateIceVersion(filename, version):
    print 'Updating ice version in ' + filename + ' to ' + version
    f = fileinput.input(filename, True)
    for line in f: 
        print line.rstrip('\n').replace('ICE_VERSION', version)
    f.close()

def obliterate(files):
    for f in files:
        if os.path.exists(f):
            if os.path.isdir(f):
                shutil.rmtree(f)
            else:
                os.remove(f)

def extractDemos(sources, buildDir, version, distro, demoDir):
    """Pulls the demo directory out of a distribution and massages its
       build system so it can be built against an installed version of
       Ice"""
    cwd = os.getcwd()
    os.chdir(os.path.join(buildDir, "demotree"))

    #
    # TODO: Some archives don't contain all of these elements. It might
    # be nicer to make the toExtract list more tailored for each
    # distribution.
    #
    toExtract = "%s/demo " % distro
    if demoDir == '':
        toExtract = toExtract + " %s/ICE_LICENSE" % distro
    if not demoDir == 'php':
        toExtract = toExtract + " %s/certs %s/config" % (distro, distro)
        
    runprog("gzip -dc " + os.path.join(sources, distro) + ".tar.gz | tar xf - " + toExtract, False)
        
    shutil.move(os.path.join(distro, "demo"), os.path.join(buildDir, "Ice-" + version + "-demos", "demo" + demoDir))
    if os.path.exists(os.path.join(buildDir, "demotree", distro, "ICE_LICENSE")):
        shutil.move(os.path.join(buildDir, "demotree", distro, "ICE_LICENSE"), \
                os.path.join(buildDir, "Ice-%s-demos" % version, "ICE_LICENSE"))

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

    if not demoDir in ['py', 'rb', 'vb'] and os.path.exists(srcConfigDir):
        for f in os.listdir(srcConfigDir):
            src = os.path.join(srcConfigDir, f)
            dest = os.path.join(destConfigDir, f)
            if not os.path.isdir(src) and not os.path.islink(src):
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

    basepath = os.path.join(buildDir, 'Ice-' + version + '-demos', 'config')

    if distro.startswith('Ice-'):
        editMakeRules(os.path.join(basepath, 'Make.rules'), version)
        editMakeRulesMak(os.path.join(basepath, 'Make.rules.mak'), version)
    elif distro.startswith('IceCS-'):
        editMakeRulesCS(os.path.join(basepath, 'Make.rules.cs'), version)
        editMakeRulesMakCS(os.path.join(basepath, 'Make.rules.mak.cs'), version)

    #
    # Remove collected files.
    #
    obliterate(remove)
        
    shutil.rmtree(os.path.join(buildDir, 'demotree', distro), True)
    os.chdir(cwd)

def archiveDemoTree(buildDir, version, installFiles):
    cwd = os.getcwd()
    os.chdir(os.path.join(buildDir, 'Ice-%s-demos' % version))
    filesToRemove = ['certs/makecerts.py', 'certs/ImportKey.java', 'certs/ImportKey.class', 'certs/seed.dat',
            'config/convertssl.py', 'config/upgradeicegrid.py', 'config/upgradeicestorm.py',
            'config/icegrid-slice.3.1.ice.gz', 'config/PropertyNames.def', 'config/makeprops.py', 
            'config/Makefile', 'config/Makefile.mak', 'config/TestUtil.py', 'config/IceGridAdmin.py', 
            'config/ice_ca.cnf', 'config/icegridgui.pro']
    obliterate(filesToRemove)
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
    # Remove config files.
    #
    runprog("sh -c 'for f in `find Ice-" + version + "-demos/democs -name \"*\.config\" ` ; do rm -f $f ; done'")
    runprog("sh -c 'for f in `find Ice-" + version + "-demos/demovb -name \"*\.config\" ` ; do rm -f $f ; done'")

    #
    # Remove Windows project files.
    #
    runprog("sh -c 'for f in `find Ice-" + version + "-demos -name \"*\.dsp\" ` ; do rm -rf $f ; done'")
    runprog("sh -c 'for f in `find Ice-" + version + "-demos -name \"*\.dsw\" ` ; do rm -rf $f ; done'")
    runprog("sh -c 'for f in `find Ice-" + version + "-demos -name \"*.sln\" ` ; do rm -rf $f ; done'")
    runprog("sh -c 'for f in `find Ice-" + version + "-demos -name \"*.csproj\" ` ; do rm -rf $f ; done'")
    runprog("sh -c 'for f in `find Ice-" + version + "-demos -name \"*.vbproj\" ` ; do rm -rf $f ; done'")
    runprog("sh -c 'for f in `find Ice-" + version + "-demos -name \"*.vcproj\" ` ; do rm -rf $f ; done'")
   
    runprog("tar cf Ice-" + version + "-demos.tar Ice-" + version + "-demos")
    runprog("gzip -9 Ice-" + version + "-demos.tar")
    runprog("zip -9r Ice-" + version + "-demos.zip Ice-" + version + "-demos")
    os.chdir(cwd)

def makeInstall(sources, buildDir, installDir, distro, clean, version, mmVersion):
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
        initDirectory(installDir)
        initDirectory(os.path.join(installDir, 'lib'))
        shutil.copy(buildDir + '/' + distro + '/lib/Ice.jar', installDir + '/lib')
        shutil.copy(buildDir + '/' + distro + '/lib/IceGridGUI.jar', installDir + '/lib')
        #
        # We really just want to copy the files, not move them.
        # Shelling out to a copy is easier (and more likely to always
        # work) than shutil.copytree().
        #
        runprog('cp -pR ' + buildDir + '/' + distro + '/ant ' + installDir)
        runprog('find ' + installDir + '/ant  -name "*.java" | xargs rm')
        initDirectory(os.path.join(installDir, 'help'))
        runprog('cp -pR ' + buildDir + '/' + distro + '/resources/IceGridAdmin ' + installDir + '/help')
        destDir = os.path.join(installDir, 'config')
        if not os.path.exists(destDir):
            os.mkdir(destDir)
        shutil.copy(os.path.join('config', 'build.properties'), destDir)
        os.chdir(cwd)
        return

    if distro.startswith('IcePy'):
        try:
            pyHome = os.environ['PYTHON_HOME']
        except KeyError:
            pyHome = None
            
        if pyHome == None or pyHome == '':
            logging.info('PYTHON_HOME is not set, figuring it out and trying that')
            pyHome = sys.exec_prefix
            os.environ['PYTHON_HOME'] = pyHome
            
    # 
    # XXX- Optimizations need to be turned on for the release.
    #
    try:
        runprog('gmake NOGAC=yes OPTIMIZE=yes prefix=%s embedded_runpath_prefix=/opt/Ice-%s install' % (installDir, mmVersion))
    except ExtProgramError:
        print "gmake failed for makeInstall(%s, %s, %s, %s, %s, %s, %s)" % (sources, buildDir, installDir, distro, str(clean), version, mmVersion) 
        raise

    runprog('rm -rf /opt/Ice-%s' % (mmVersion), False)

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
        findCmd = 'find lib* -name "*'  + getPlatformLibExtension() + '" -type f'
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
        findCmd = 'find lib* -name "*'  + getPlatformLibExtension() + '" -type f'
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
        findCmd = 'find lib* -name "*'  + getPlatformLibExtension() + '" -type l'
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

def fixInstallNames(version, mmVersion):

    #
    # Fix the install names of the 3rd party libraries.
    # 
    libs = ['libdb-*.dylib', 'libdb_cxx-*.dylib', 'libdb_java-*.jnilib', 'libexpat*.dylib']
    oldInstallNames = []
    for l in libs:
        p = os.popen('otool -D ' + 'Ice-' + version + '/lib/'  + l + ' | tail -1')
        oldInstallNames.append(p.readline().strip())
        p.close()

    allFiles = []
    p = os.popen('find Ice-' + version + '/bin -name "*" -type f')
    allFiles.extend(p.readlines())
    p.close()
    p = os.popen('find Ice-' + version + '/lib -name "*.dylib" -type f')
    allFiles.extend(p.readlines())
    p.close()

    binFiles = []
    for f in allFiles:
        f = f.strip()
        if not os.system('file -b ' + f + ' | grep -q "Mach-O"'):
            os.system('chmod u+w ' + f)
            binFiles.append(f)

    for oldName in oldInstallNames:
        libName = re.sub("\/opt\/.*\/(.*)", "\\1", oldName)
        newName = '/opt/Ice-' + mmVersion + '/' + libName

        os.system('install_name_tool -id ' + newName + ' Ice-' + version + '/lib/' + libName)
        for f in binFiles:
            os.system('install_name_tool -change ' + oldName + ' ' + newName + ' ' + f)


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
    print '                       (Note: makedist.py seems to only work on Linux.'
    print '                        To use makebindist.py on other UNIX platforms,'
    print '                        you must copy pre-made source distributions onto'
    print '                        the host and use this option to reference their'
    print '                        location.'
    print '-v, --verbose          Print verbose processing messages.'
    print '-t, --tag              Specify the CVS version tag for the packages.'
    print '--noclean              Do not clean up current sources where'
    print '                       applicable (some bits will still be cleaned.'
    print '--nobuild              Run through the process but don\'t build'
    print '                       anything new.'
    print '--usecvs               Use contents of existing CVS directories'
    print '                       to create binary package.'
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
    print '--expathome=[path]     Specify location of expat libraries '
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
    cvsTag = 'HEAD'
    clean = True
    build = True
    version = None
    mmVersion = None
    soVersion = 0
    verbose = False
    cvsMode = False    # Use CVS directories.
    offline = False

    #
    # Process args.
    #
    try:
        optionList, args = getopt.getopt(sys.argv[1:], 'hvt:',
                                         [ 'build-dir=', 'install-dir=', 'install-root=', 'sources=',
                                           'verbose', 'tag=', 'noclean', 'nobuild', 
                                           'stlporthome=', 'bzip2home=', 'dbhome=', 'sslhome=',
                                           'expathome=', 'readlinehome=', 'usecvs', 'offline', 'debug'])
               
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
    elif getPlatform() == 'macosx':
        dylibEnvironmentVar = 'DYLD_LIBRARY_PATH'
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
        mmVersion = getIceMMVersion('include/IceUtil/Config.h')
        installFiles = 'install'
    elif offline:
        version = getIceVersion('include/IceUtil/Config.h')
        soVersion = getIceSoVersion('include/IceUtil/Config.h')
        mmVersion = getIceMMVersion('include/IceUtil/Config.h')
        installFiles = getInstallFilesFromLocalDirectory(cvsTag, buildDir, version, mmVersion)
    else:
        version, soVersion, mmVersion = getVersion(cvsTag, buildDir)
        installFiles = getInstallFiles(cvsTag, buildDir, version, mmVersion)

    if verbose:
        print 'Building binary distributions for Ice-' + version + ' on ' + getPlatformString()
        print 'Using build directory: ' + buildDir
        print 'Using install directory: ' + installDir
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
            if not getPlatform().startswith("linux"):
                print "makedist.py is not supported on non-Linux platforms. Create the source"
                print "distributions on a Linux box, copy them to a location on this host and"
                print "specify their location with the --sources argument"
            sources = buildDir + '/sources'
            collectSources = clean

        #
        # Ice must be first or building the other source distributions will fail.
        #
        sourceTarBalls = [ ('ice', 'Ice-%s' % version, ''),
                           ('icej','IceJ-%s-java2' % version, 'j')]

        if not getPlatform() in ['aix', 'solaris', 'hpux', 'macosx']:
            sourceTarBalls.append(('icephp','IcePHP-' + version, 'php'))

        if not getPlatform() in ['aix', 'solaris', 'hpux']:
            sourceTarBalls.append(('icepy','IcePy-' + version, 'py'))

        if getPlatform() == 'linux':
            sourceTarBalls.append(('icecs','IceCS-' + version, 'cs'))
            if os.system('which ruby 2>/dev/null') == 0:
                sourceTarBalls.append(('icerb', 'IceRuby-%s' % version, 'rb'))
        
        os.environ['ICE_HOME'] = buildDir + '/Ice-' + version
        currentLibraryPath = None
        try:
            currentLibraryPath = os.environ[dylibEnvironmentVar] 
        except KeyError:
            currentLibraryPath = ''

        os.environ[dylibEnvironmentVar] = buildDir + '/Ice-' + version + '/lib:' + currentLibraryPath
        os.environ['PATH'] = buildDir + '/Ice-' + version + '/bin:' + os.environ['PATH']

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
        if getPlatform() != 'hpux' and getPlatform() != 'solaris' and getPlatform() != 'macosx':
            toCollect = list(sourceTarBalls)
            toCollect.append(('icevb', 'IceVB-' + version, 'vb'))
            for cvs, tarball, demoDir in toCollect:
                extractDemos(sources, buildDir, version, tarball, demoDir)
                shutil.copy("%s/common/README.DEMOS" % installFiles, "%s/Ice-%s-demos/README.DEMOS" % (buildDir, version)) 
            archiveDemoTree(buildDir, version, installFiles)
            shutil.move("%s/Ice-%s-demos.tar.gz" % (buildDir, version), "%s/Ice-%s-demos.tar.gz" % (installDir, version))
            shutil.move("%s/Ice-%s-demos.zip" % (buildDir, version), "%s/Ice-%s-demos.zip" % (installDir, version))

        #
        # Everything should be set for building stuff up now.
        #
        for cvs, tarball, demoDir in sourceTarBalls:
            makeInstall(sources, buildDir, "%s/Ice-%s" % (installDir, version), tarball, clean, version, mmVersion)         

        #
        # Put the Java2 jar in its directory
        #
        prevDir = os.getcwd()
        os.chdir("%s/Ice-%s/lib" % (installDir, version))
        os.mkdir("java2")
        shutil.move("Ice.jar", "java2/Ice.jar")
        os.system("gzip -dc %s/IceJ-%s-java5.tar.gz | tar xf - IceJ-%s-java5/lib/Ice.jar" % (sources, version, version))
        shutil.move("IceJ-%s-java5/lib/Ice.jar" % version, "Ice.jar")
        shutil.rmtree("IceJ-%s-java5" % version)
        os.chdir(prevDir)

    elif cvsMode:
        collectSources = False

        #
        # TODO: Sanity check to make sure that the script is being run
        # from a location that it expects.
        #
        cvsDirs = [ 'ice', 'icej', 'icephp' ]
        if getPlatform() == 'linux':
            cvsDirs.append('icecs', 'icepy', 'icerb')

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
                initDirectory(os.path.join(installDir, 'help'))
                runprog('cp -pR resources/IceGridAdmin ' + installDir + '/Ice-' + version + '/help')
                runprog('find ' + installDir + '/Ice-' + version + ' -name "*.java" | xargs rm')
            else:
                runprog('gmake prefix=' + installDir + '/Ice-' + version + ' install')
            os.chdir(currentDir)

    #
    # Sources should have already been built and installed.  We
    # can pick the binaries up from the iceinstall directory.
    #
    binaries = glob.glob(installDir + '/Ice-' + version + '/bin/*')
    binaries.extend(glob.glob(installDir + '/Ice-' + version + '/lib/*' + shlibExtensions(version, soVersion)[0]))
    cwd = os.getcwd()
    os.chdir(installDir)

  
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
#       runprog('cp -R ' + ssl + '/include Ice-' + version)
        runprog('cp ' + ssl + '/lib/lib*.sl.* Ice-' + version + '/lib')
        runprog('rm -rf Ice-' + version + '/lib/libfips*')
        runprog('rm -rf Ice-' + version + '/lib/engines')
        runprog('rm -rf Ice-' + version + '/lib/pkgconfig')
        runprog('rm -f Ice-' + version + '/lib/*.a')

    uname = readcommand('uname')
    platformSpecificFiles = [ 'README', 'SOURCES', 'THIRD_PARTY_LICENSE' ]
    for psf in platformSpecificFiles:
        platform = uname
        if psf == 'README':
            if platform == 'Linux':
                if os.system('test -f /etc/redhat-release') == 0:
                    platform = 'RHEL'
                else:
                    platform = 'SLES'
        cf = os.path.join(installFiles, 'unix', psf + '.' + platform)
        if os.path.exists(cf):
            shutil.copy(cf, os.path.join('Ice-' + version, psf))

    # 
    # Remove build files from binary distribution. 
    #
    runprog("rm -f Ice-%s/config/build.properties" % (version))
    runprog("rm -f Ice-%s/config/Make.rules*" % (version))

    # 
    # Remove doc from binary distribution. 
    #
    runprog("rm -rf Ice-%s/doc" % (version))

    #
    # Fix install names on Mac OS X   
    #
    if getPlatform() == 'macosx':
        fixInstallNames(version, mmVersion)

    runprog('tar cf Ice-' + version + '-bin-' + getPlatformString() + '.tar Ice-' + version)
    runprog('gzip -9 Ice-' + version + '-bin-' + getPlatformString() + '.tar')
    os.chdir(cwd)

    #
    # TODO: Cleanups?  I've left everything in place so that the process
    # can be easily debugged.
    #

if __name__ == "__main__":
    main()
