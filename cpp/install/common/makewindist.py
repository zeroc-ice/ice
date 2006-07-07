#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import getopt, os, re, shutil, string, sys, zipfile
import logging, cStringIO, glob
import components
import textwrap

# 
# Current default third party library versions.
#
OpenSSLVer = '0.9.8b'
Bzip2Ver = '1.0.3'
STLPortVer = '4.6.2'
ExpatVer = '1.95.8'
DBVer = '4.3.29'

DistPrefixes = ["Ice-", "IceJ-", "IceCS-", "IcePy-", "IcePHP-", "IceVB-"]

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
    print "-i, --info		 Log information messages"
    print "-d, --debug 		 Log debug messages"
    print "-l, --logfile 	 Specify the destination log file"

def environmentCheck(target):
    """Warning: uses global environment."""
    required = ["SOURCES", "BUILD_DIR", "DB_HOME", "BZIP2_HOME", "EXPAT_HOME", "OPENSSL_HOME"]
    if target == "vc60":
	required.append("STLPORT_HOME")
    elif target == "vc80":
	required.extend(["PHP_BIN_HOME", "PHP_SRC_HOME", "PYTHON_HOME"])

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

def maxVersion(a, b):
    """Compares to version strings. The version strings should be trimmed of leading and trailing whitespace."""
    if a == b:
	return a

    avalues = a.split('.')
    bvalues = b.split('.')

    diff = len(avalues) - len(bvalues)
    if not diff == 0:
	if diff < 0:
	    for f in range(0, abs(diff)):
		avalues.append('0')
	else:
	    for f in range(0, abs(diff)):
		bvalues.append('0')

    for i in range(0, len(avalues)):
	if int(avalues[i]) > int(bvalues[i]):
	    return a
	elif int(avalues[i]) < int(bvalues[i]):
	    return b

    return a

def testMaxVersion():
    # Case format first, second, expected.
    cases = [ ("1.0", "1.0.0", "1.0"), ("0.0", "0.1", "0.1"), ("2.1.0", "2.0.1", "2.1.0"),
              ("2.1", "2.0.1", "2.1"), ("2.1.9", "2.1.12", "2.1.12")]
    for a, b, expected in cases:
	result = maxVersion(a, b)
	if not expected == result:
	    print "Expected %s from %s and %s, got %s" % (expected, a, b, result)
	    assert(False)
    print "testMaxVersion() succeeded"

def checkSources(sourceDir):
    """Scans a directory for source distributions. The version is keyed on the Ice for C++ distribution."""

    icezip = glob.glob(os.path.join(sourceDir, "Ice-*.zip"))
    if len(icezip) == 0:
	msg = "Source directory %s does not contain a zip archive for any version of Ice for C++" % sourceDir
	logging.error(msg)
	raise DistEnvironmentError(msg)

    keyVersion = '0.0.0'
    exp = re.compile("Ice-([0-9.]*).zip")
    current = None
    for d in icezip:
	m = exp.match(os.path.split(d)[1])
	if m == None:
	    print icezip
	current = m.group(1)
	keyVersion = maxVersion(keyVersion, current)

    print keyVersion
    prefixes = list(DistPrefixes)
    prefixes.remove("Ice-")
    for prefix in prefixes:
	if not os.path.exists(os.path.join(sourceDir, "%s%s.zip" % (prefix, keyVersion))):
	    msg = "Source directory %s does not contain archive for %s."
	    logging.error(msg)
	    raise DistEnvironmentError(msg)

    return keyVersion

def buildIceDists(stageDir, sourcesDir, sourcesVersion, installVersion):
    """Build all Ice distributions."""

    #
    # Update PATH, LIB and INCLUDE environment variables required for
    # building Ice for C++.
    #
    path = [
	os.path.join(stageDir, "berkeley", "dev", "bin"),
	os.path.join(stageDir, "berkeley", "runtime", "bin"),
	os.path.join(stageDir, "berkeley", "java", "bin"),
	os.path.join(stageDir, "expat", "runtime", "bin"),
	os.path.join(stageDir, "openssl", "runtime", "bin")
    ]
    if installVersion == "vc60":
	path.append(os.path.join(stageDir, "stlport", "dev", "bin"))
	path.append(os.path.join(stageDir, "stlport", "runtime", "bin"))
    prependEnvPathList('PATH', path)

    lib = [
	os.path.join(stageDir, "berkeley", "dev", "lib"),
	os.path.join(stageDir, "bzip2", "dev", "lib"),
	os.path.join(stageDir, "expat", "dev", "lib"),
	os.path.join(stageDir, "openssl", "dev", "lib"),
    ]
    if installVersion == "vc60":
	lib.append(os.path.join(stageDir, "stlport", "dev", "lib"))
    prependEnvPathList('LIB', lib)

    include = [
	os.path.join(stageDir, "berkeley", "dev", "include"),
	os.path.join(stageDir, "bzip2", "dev", "include"),
	os.path.join(stageDir, "expat", "dev", "include"),
	os.path.join(stageDir, "openssl", "dev", "include"),
    ]
    if installVersion == "vc60":
	include.append(os.path.join(stageDir, "stlport", "dev", "include", "stlport"))
    prependEnvPathList('INCLUDE', include)

    iceHome = os.environ['ICE_HOME']
    prependEnvPath('PATH', os.path.join(iceHome, "bin"))
    prependEnvPath('LIB', os.path.join(iceHome, "lib"))
    prependEnvPath('INCLUDE', os.path.join(iceHome, "include"))

    if installVersion in ["vc80", "vc71"]:
	#
	# Ice for C++ 
	#
	os.chdir(os.path.join(sourcesDir, "Ice-" + sourcesVersion))
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
	if installVersion == "vc71":
	    runprog("devenv all_11.sln /useenv /build Debug")
	else:
	    runprog("devenv all.sln /useenv /build Debug")

	#
	# Ice for Visual Basic
	#
	os.chdir(os.path.join(sourcesDir, "IceVB-" + sourcesVersion))
	print "Building in " + os.getcwd() + "..."
	runprog("devenv all.sln /useenv /build Debug")

    if installVersion == "vc80":

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
    elif installVersion == "vc60":
	#
	# Ice for C++ 
	#
	os.chdir(os.path.join(sourcesDir, "Ice-" + sourcesVersion))
	print "Building in " + os.getcwd() + "..."
	runprog('msdev all.dsw /useenv /make "all - Win32 Debug"')
	runprog('msdev all.dsw /useenv /make "all - Win32 Release"')
    elif installVersion in ["vc80_x64"]:
	#
	# Ice for C++ 
	#
	os.chdir(os.path.join(sourcesDir, "Ice-" + sourcesVersion))
	print "Building in " + os.getcwd() + "..."
	runprog('devenv all.sln /useenv /build Debug /project all')
	runprog('devenv all.sln /useenv /build Release /project all')

def list2english(l):
    if len(l) == 1:
	return l[0]
    elif len(l) == 2:
	return l[0] + " and " + list2english(l[1:])
    else:
	return l[0] + ", " + list2english(l[1:]) 

def convertLicensesToRTF(toolDir, installTarget):
    openssl = (os.path.join(os.environ["OPENSSL_HOME"], "LICENSE"), "OpenSSL", "OPENSSL_LICENSE.rtf")
    berkeleydb = (os.path.join(os.environ["DB_HOME"], "LICENSE"), "Berkeley DB", "BERKELEY_DB_LICENSE.rtf")
    expat = (os.path.join(os.environ["EXPAT_HOME"], "COPYING"), "Expat", "EXPAT_LICENSE.rtf")
    bzip2 = (os.path.join(os.environ["BZIP2_HOME"], "LICENSE"), "bzip2/libbzip2", "BZIP2_LICENSE.rtf")

    #
    # The license file isn't quite right for this.
    #
    section_header = "License agreement for %s:\n"
    line_string = "-------------------------------------------------------------------------------------------"
    rtfhdr = file(os.path.join(toolDir, "docs", "rtf.hdr")).readlines()
    rtfftr = file(os.path.join(toolDir, "docs", "rtf.footer")).readlines()
    
    core = [ berkeleydb, bzip2, openssl, expat ]

    collection = core
    jgoodies =[(os.path.join(os.environ["JGOODIES_FORMS"], "license.txt"), "JGoodies Forms", 
			    "JGOODIES_FORMS_LICENSE.rtf"),
	       (os.path.join(os.environ["JGOODIES_LOOKS"], "license.txt"), "JGoodies Looks", 
			    "JGOODIES_LOOKS_LICENSE.rtf")]
    if installTarget == "vc60":
	collection.append((os.path.join(os.environ["STLPORT_HOME"], "doc", "license.html"),  
	                   "STLport", "STLPORT_LICENSE.rtf"))
	collection.extend(jgoodies)
    elif installTarget in ["vc71", "vc80", "vc80_x64"]:
	collection.extend(jgoodies)

    third_party_sources_file_hdr = """Source Code
-----------

"""

    if not os.path.exists(os.path.join(toolDir, "docs")):
	os.mkdir(os.path.join(toolDir, "docs"))
    if not os.path.exists(os.path.join(toolDir, "docs", installTarget)):
	os.mkdir(os.path.join(toolDir, "docs", installTarget))
    names = []
    for e in collection:
	names.append(e[1])

    text = "The source distributions of " + list2english(names)
    text = text + " used to build this distribution can be downloaded at no cost from http://www.zeroc.com/download.html."
    licensefile = file(os.path.join(toolDir, "docs", installTarget, "THIRD_PARTY_SOURCES"), "w")

    #
    # textwrap module has got to be one of the coolest things since
    # sliced bread. 
    #
    licensefile.write(third_party_sources_file_hdr)
    licensefile.write(textwrap.fill(text, 75))
    licensefile.close()

    #
    # THIRD_PARTY_SOURCES is the file used by the Ice installer while
    # SOURCES is used by the third party installer.
    #
    shutil.copy(os.path.join(toolDir, "docs", installTarget, "THIRD_PARTY_SOURCES"), 
	    os.path.join(toolDir, "docs", installTarget, "SOURCES")) 

    licensefile = file(os.path.join(toolDir, "docs", installTarget, "LICENSE"), "w")
    for f in collection:
	contents = None
	if f[0].endswith(".html"):
	    # 
	    # Here's me wishing the Python standard library had a class
	    # for converting HTML to plain text. In the meantime, we'll
	    # have to leverage 'links' in cygwin.
	    #
	    pipe_stdin, pipe_stdout = os.popen2("cygpath %s" % f[0])
	    lines = pipe_stdout.readlines()
	    pipe_stdin.close()
	    pipe_stdout.close()
	    cygname = lines[0].strip()
	    pipe_stdin, pipe_stdout = os.popen2("links -dump %s" % cygname)
	    lines = pipe_stdout.readlines()
	    contents = lines[2:]
	else:
	    contents = file(f[0]).readlines()
	hdr = section_header % f[1]
	
	licensefile.write(hdr)
	licensefile.write(line_string[:len(hdr)] + "\n\n")
	licensefile.writelines(contents)
	licensefile.write("\n\n")
	rtffile = file(os.path.join(toolDir, "docs", installTarget, os.path.basename(f[2])), "w")
	rtffile.writelines(rtfhdr)
	rtffile.write(hdr + "\\par")
	rtffile.write(line_string[:len(hdr)] + "\\par\n")
	for l in contents:
	    rtffile.write(l.rstrip("\n") + "\\par\n")
	rtffile.writelines(rtfftr)
	rtffile.close()

    #
    # Technically, the JGoodies stuff isn't part of the Ice 6.0.0 target
    # but we include it in our third party installer as a convenience to
    # IceJ builders.
    #
    if installTarget == "vc60":
	for f in jgoodies:
	    contents = file(f[0]).readlines()
	    hdr = section_header % f[1]
	    rtffile = file(os.path.join(toolDir, "docs", installTarget, os.path.basename(f[2])), "w")
	    rtffile.writelines(rtfhdr)
	    rtffile.write(hdr + "\\par")
	    rtffile.write(line_string[:len(hdr)] + "\\par\n")
	    for l in contents:
		rtffile.write(l.rstrip("\n") + "\\par\n")
	    rtffile.writelines(rtfftr)
	    rtffile.close()

    licensefile.close()
    shutil.copyfile(os.path.join(toolDir, "docs", installTarget, "LICENSE"), \
	    os.path.join(toolDir, "docs", installTarget, "THIRD_PARTY_LICENSE"))
    lines = file(os.path.join(toolDir, "docs", installTarget, "LICENSE")).readlines()
    rtflicense = file(os.path.join(toolDir, "docs", installTarget, "LICENSE.rtf"), "w")
    rtflicense.writelines(rtfhdr)
    for l in lines:
	rtflicense.write(l.rstrip("\n") + "\\par\n")
    rtflicense.writelines(rtfftr)
    rtflicense.close()

def buildMergeModules(startDir, stageDir, sourcesVersion, installVersion):
    """Build third party merge modules."""
    modules = [
	("BerkeleyDBDevKit", "BERKELEYDB_DEV_KIT"),
	("BerkeleyDBRuntime", "BERKELEYDB_RUNTIME"),
	("BZip2DevKit", "BZIP2_DEV_KIT"),
	("BZip2Runtime", "BZIP2_RUNTIME"),
	("ExpatDevKit", "EXPAT_DEV_KIT"),
	("ExpatRuntime", "EXPAT_RUNTIME"),
	("OpenSSLDevKit", "OPENSSL_DEV_KIT"),
	("OpenSSLRuntime", "OPENSSL_RUNTIME"),
	("JGoodies", "JGOODIES_RUNTIME"),
	("BerkeleyDBJava", "BERKELEYDB_JAVA")
    ]
    if installVersion == "vc60":
	extras = [ ("STLPortDevKit", "STLPORT_DEV_KIT"), ("STLPortRuntime", "STLPORT_RUNTIME") ]
	modules.extend(extras)

    #
    # Build modules.
    #
    os.chdir(startDir)
    for project, release in modules:
	#
	# The -w -x flags indicate that the build should stop on any
	# warning or error. This is preferable since it catches staging
	# errors and forces us to keep our projects clean.
	#
	runprog(os.environ['INSTALLSHIELD_HOME'] + "\IsCmdBld -x -w -c COMP -a ZEROC -p " + project + ".ism -r " + release)

    #
    # Archive modules in the stage directory root.
    #

    #
    # <brent> Were we doing something special with third-party merge
    # modules at one point, like redistributing them?</brent>
    #
    
    zipPath = "ThirdPartyMergeModules-" + sourcesVersion + "-" + installVersion.upper() + ".zip"
    zip = zipfile.ZipFile(os.path.join(stageDir, zipPath), 'w')
    for project, release in modules:
	msm = project + "." + installVersion.upper() + ".msm"
	msmPath = os.path.join(os.getcwd(), project, "ZEROC", release, "DiskImages/DISK1", msm)
	zip.write(msmPath, os.path.basename(msmPath))
    zip.close()

def buildInstallers(startDir, stageDir, sourcesVersion, installVersion, installers):

    #
    # Build and copy to the stage directory root.
    #
    os.chdir(startDir)
    for project, release in installers:
	runprog(os.environ['INSTALLSHIELD_HOME'] + "\ISCmdBld -x -w -c COMP -a ZEROC -p " + project + ".ism -r " + release)
	msi = project + "-" + sourcesVersion + "-" + installVersion.upper() + ".msi"
	msiPath = os.path.join(os.getcwd(), project, "ZEROC", release, "DiskImages/DISK1", msi)
	shutil.copy(msiPath, stageDir)

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
		"logfile", "vc60", "vc71", "vc80", "vc80_x64", "sslhome=", "expathome=", "dbhome=", "stlporthome=", 
		"bzip2home=", "thirdparty="])
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
	    elif o == '--vc71':
		target = 'vc71'
	    elif o == '--vc80':
		target = 'vc80'
	    elif o == '--vc80_x64':
		target = 'vc80_x64'
	    elif o == '--sources':
		os.environ['SOURCES'] = a
	    elif o == '--buildDir':
		os.environ['BUILD_DIR'] = a
	    elif o == '--sslhome':
		os.environ['OPENSSL_HOME'] = a
	    elif o == '--expathome':
		os.environ['EXPAT_HOME'] = a
	    elif o == '--dbhome':
		os.environ['DB_HOME'] = a
	    elif o == '--stlporthome':
		os.environ['STLPORT_HOME'] = a
	    elif o == '--bzip2home':
		os.environ['BZIP2_HOME'] = a
	    elif o == '--thirdparty':
		os.environ['OPENSSL_HOME'] = os.path.join(a, 'openssl-%s' % OpenSSLVer)
		os.environ['BZIP2_HOME'] = os.path.join(a, 'bzip2-%s' % Bzip2Ver)
		os.environ['EXPAT_HOME'] = os.path.join(a, 'expat-%s' % ExpatVer)
		os.environ['DB_HOME'] = os.path.join(a, 'db-%s' % DBVer)
		os.environ['STLPORT_HOME'] = os.path.join(a, 'STLPort-%s' % STLPortVer)

	if debugLevel != logging.NOTSET:
	    if a != None:
		logging.basicConfig(level = debugLevel, format='%(asctime)s %(levelname)s %(message)s', filename = a)
	    else:
		logging.basicConfig(level = debugLevel, format='%(asctime)s %(levelname)s %(message)s')

	if target == None:
	    print 'The development target must be specified'
	    sys.exit(2)

	os.environ['target'] = target

	#
	# Where all the files will be staged so that the install projects
	# can find them.
	#
	targetDir = os.path.join(installDir, target)
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

	sourcesVersion = checkSources(os.environ['SOURCES'])

	#
	# XXX - implement 'refresher'
	#

	# 
	# Screw clean rules, run the ultimate clean!
	#
	if clean:
	    shutil.rmtree(buildDir)
	    os.mkdir(buildDir)

	for z in DistPrefixes:
            #
	    # TODO: See if this can be replaced by ZipFile and native
	    # Python code somehow.
	    #
	    filename = os.path.join(os.environ['SOURCES'], "%s%s.zip" % (z, sourcesVersion))
	    if not os.path.exists(os.path.join(os.environ['BUILD_DIR'], "%s%s" % (z, sourcesVersion))):
		runprog("unzip -q %s -d %s" % (filename, os.environ['BUILD_DIR']))

	os.environ['ICE_HOME'] = os.path.join(os.environ['BUILD_DIR'], "Ice-%s" % sourcesVersion)

	defaults = os.environ
	defaults['dbver'] = '43'
	defaults['version'] = sourcesVersion
	defaults['dllversion'] = sourcesVersion.replace('.', '')[:2]
	
	if target == "vc80_x64":
	    defaults['OutDir'] = "x64/"
	else:
	    defaults['OutDir'] = ''

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
	# Gather and generate license files.
	#
	convertLicensesToRTF(os.path.dirname(__file__), target)

	#
	# The third party packages need to be staged before building the
	# distributions. This ordering is important because it adds an
	# additional check that the third party packages that are
	# included in the installer are suitable for use with Ice.
	#
	components.stage(os.path.join(os.path.dirname(components.__file__), "components", "components.ini"),
		os.path.join(os.path.dirname(components.__file__), "components"), stageDir, "packages", defaults)


	#
	# Build the merge module projects.
	# 
	if installer:
	    buildMergeModules(targetDir, stageDir, sourcesVersion, target)
	    buildInstallers(targetDir, stageDir, sourcesVersion, target, [("ThirdParty", "THIRD_PARTY_MSI")])

	#
	# Build the Ice distributions.
	#
	if build:
	    buildIceDists(stageDir, os.environ['BUILD_DIR'], sourcesVersion, target)

	# 
	# Stage Ice!
	#
	components.stage(os.path.join(os.path.dirname(components.__file__), "components", "components.ini"),
		os.path.join(os.path.dirname(components.__file__), "components"), stageDir, "ice", defaults)

	#
	# Build the installer projects.
	#
	if installer:
	    buildInstallers(targetDir, stageDir, sourcesVersion, target, [("Ice", "ICE_MSI")])

    finally:
	#
	# Return the user to where they started.
	#
	os.chdir(startDir)

if __name__ == "__main__":
    main()
