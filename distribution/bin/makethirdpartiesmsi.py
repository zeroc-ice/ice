#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import getopt, os, re, shutil, string, sys, zipfile, fileinput
import logging, cStringIO, glob
import textwrap

resources = os.path.join(os.path.abspath(os.path.dirname(sys.argv[0])), "..", "src", "windows")
sys.path.append(resources)
import components

iceVersion = '3.4b'

#
# Current default third party library versions.
#
OpenSSLVer = '0.9.8g'
Bzip2Ver = '1.0.5'
STLPortVer = '4.6.2'
ExpatVer = '2.0.1'
DBVer = '4.6.21'
MCPPVer = '2.7.2'

timeStampingURL = 'http://timestamp.verisign.com/scripts/timstamp.dll'

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
    print "    --clean           Clean compiled or staged files."
    print "-i, --info            Log information messages"
    print "-d, --debug           Log debug messages"
    print "-l, --logfile         Specify the destination log file"

def environmentCheck(target):
    """Warning: uses global environment."""
    required = ["DB_HOME", "BZIP2_HOME", "EXPAT_HOME", "OPENSSL_HOME", "MCPP_HOME"]
    if target == "vc60":
        required.append("STLPORT_HOME")

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

def getIceVersion(file):
    """Extract the ICE version string from a file."""
    config = open(file, 'r')
    return  re.search('ICE_STRING_VERSION \"([0-9\.b]*)\"', config.read()).group(1)


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
    expat = (os.path.join(os.environ["EXPAT_HOME"], "COPYING.txt"), "Expat", "EXPAT_LICENSE.rtf")
    bzip2 = (os.path.join(os.environ["BZIP2_HOME"], "LICENSE"), "bzip2/libbzip2", "BZIP2_LICENSE.rtf")
    mcpp = (os.path.join(os.environ["MCPP_HOME"], "LICENSE"), "MCPP", "MCPP_LICENSE.rtf")

    #
    # The license file isn't quite right for this.
    #
    section_header = "License agreement for %s:\n"
    line_string = "-------------------------------------------------------------------------------------------"
    rtfhdr = file(os.path.join(toolDir, "docs", "rtf.hdr")).readlines()
    rtfftr = file(os.path.join(toolDir, "docs", "rtf.footer")).readlines()

    core = [ berkeleydb, bzip2, openssl, expat, mcpp ]

    collection = core
    if installTarget != "bcc2007" and installTarget != "bcc2009":
        jgoodies =[(os.path.join(os.environ["JGOODIES_FORMS"], "license.txt"), "JGoodies Forms",
                    "JGOODIES_FORMS_LICENSE.rtf"),
                   (os.path.join(os.environ["JGOODIES_LOOKS"], "license.txt"), "JGoodies Looks",
                    "JGOODIES_LOOKS_LICENSE.rtf")]
        if installTarget == "vc60":
            collection.append((os.path.join(os.environ["STLPORT_HOME"], "doc", "license.html"),
                               "STLport", "STLPORT_LICENSE.rtf"))
            collection.extend(jgoodies)
        elif installTarget in ["vc71", "vc80", "vc90"]:
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

def buildMergeModules(startDir, stageDir, iceVersion, installVersion):
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
        ("MCPPDevKit", "MCPP_DEV_KIT"),
    ]

    if installVersion != "bcc2007" and installVersion != "bcc2009":
        javaExtras = [ ("JGoodies", "JGOODIES_RUNTIME"),  ("BerkeleyDBJava", "BERKELEYDB_JAVA") ]
        modules.extend(javaExtras)

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

    zipPath = "ThirdPartyMergeModules-" + iceVersion + "-" + installVersion.upper() + ".zip"
    zip = zipfile.ZipFile(os.path.join(stageDir, zipPath), 'w')
    compiler = installVersion.upper()
    if compiler == "BCC2007" or compiler == "BCC2009":
        compiler = "BCC"
    for project, release in modules:
        msm = project + "." + compiler + ".msm"
        msmPath = os.path.join(os.getcwd(), project, "ZEROC", release, "DiskImages/DISK1", msm)
        zip.write(msmPath, os.path.basename(msmPath))
    zip.close()

def buildInstallers(startDir, stageDir, iceVersion, installVersion, installers):

    installVersion = installVersion.upper()

    #
    # Build msi
    #
    os.chdir(startDir)
    for project, release in installers:
        runprog(os.environ['INSTALLSHIELD_HOME'] + "\IsCmdBld -x -w -c COMP -a ZEROC -p " + project + ".ism -r " + release)
        if project == "Ice":
            msi = project + "-" + iceVersion + "-" + installVersion + ".msi"
        else:
            msi = "Ice-" + iceVersion + "-" + project + "-" + installVersion + ".msi"
        msiPath = os.path.join(os.getcwd(), project, "ZEROC", release, "DiskImages/DISK1", msi)
        runprog('signtool sign /f ' + os.environ['PFX_FILE'] + ' /p ' + os.environ['PFX_PASSWORD'] 
                + ' /t ' + timeStampingURL
                + ' /d ' + msi
                + ' /du http://www.zeroc.com'
                + ' ' + msiPath)

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
                "logfile", "vc60", "vc80", "vc90", "bcc2007", "bcc2009", "sslhome=", "expathome=", "dbhome=", "stlporthome=",
                "bzip2home=", "mcpphome=", "jgoodiesformshome=", "jgoodieslookshome=", "pfxfile=", "pfxpassword="])
        except getopt.GetoptError:
            usage()
            sys.exit(2)

        #
        # Set a few defaults.
        #
        clean = False
        debugLevel = logging.NOTSET
        logFile = None
        target = None

        for o, a in optionList:
            if o in ("-h", "--help"):
                usage()
                sys.exit()
            elif o == "--clean":
                clean = True
            elif o in ('-l', '--logfile'):
                logFile = a
            elif o in ('-d', '--debug'):
                debugLevel = logging.DEBUG
            elif o in ('-', '--info'):
                debugLevel = logging.INFO
            elif o == '--vc60':
                target = 'vc60'
            elif o == '--vc80':
                target = 'vc80'
            elif o == '--vc90':
                target = 'vc90'
            elif o == '--bcc2007':
                target = 'bcc2007'
            elif o == '--bcc2009':
                target = 'bcc2009'
            elif o == '--pfxfile':
                os.environ['PFX_FILE'] = a
            elif o == '--pfxpassword':
                os.environ['PFX_PASSWORD'] = a
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
            elif o == '--mcpphome':
                os.environ['MCPP_HOME'] = a
            elif o == '--jgoodiesformshome':
                os.environ['JGOODIES_FORMS'] = a
            elif o == '--jgoodieslookshome':
                os.environ['JGOODIES_LOOKS'] = a
            elif o == '--thirdparty':
                os.environ['OPENSSL_HOME'] = os.path.join(a, 'openssl-%s' % OpenSSLVer)
                os.environ['BZIP2_HOME'] = os.path.join(a, 'bzip2-%s' % Bzip2Ver)
                os.environ['EXPAT_HOME'] = os.path.join(a, 'expat-%s' % ExpatVer)
                os.environ['DB_HOME'] = os.path.join(a, 'db-%s' % DBVer)
                os.environ['STLPORT_HOME'] = os.path.join(a, 'STLPort-%s' % STLPortVer)
                os.environ['MCPP_HOME'] = os.path.join(a, 'mcpp-%s' % MCPPVer)

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
        targetDir = os.path.join(installDir, "src", "windows", target)
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

        logging.debug(environToString(os.environ))

        defaults = os.environ
        defaults['dbver'] = '46'
        defaults['version'] = iceVersion
       
        defaults['OutDir'] = ''
        defaults['timeStampingURL'] = timeStampingURL

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
        convertLicensesToRTF(resources, target)

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
       
        buildMergeModules(targetDir, stageDir, iceVersion, target)
        buildInstallers(targetDir, stageDir, iceVersion, target, [("ThirdParty", "THIRD_PARTY_MSI")])

    finally:
        #
        # Return the user to where they started.
        #
        os.chdir(startDir)

if __name__ == "__main__":
    main()
