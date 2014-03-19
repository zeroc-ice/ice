#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

iceVersion = '3.5.1'

#
# Current default third party library versions.
#
OpenSSLVer = '0.9.8r'
Bzip2Ver = '1.0.6'
STLPortVer = '4.6.2'
ExpatVer = '2.0.1'
DBVer = '4.8.30'
MCPPVer = '2.7.2'
LooksVer = '2.4.1'
FormsVer = '1.4.1'
CommonVer = '1.2.0'
QtVer = '4.5.3'


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
    print "-i, --info            Log information messages"
    print "-d, --debug           Log debug messages"
    print "-l, --logfile         Specify the destination log file"

def environmentCheck():
    """Warning: uses global environment."""
    required = ["DB_HOME", "BZIP2_HOME", "EXPAT_HOME", "OPENSSL_HOME", "MCPP_HOME", "STLPORT_HOME", "JGOODIES_FORMS_HOME", "JGOODIES_LOOKS_HOME", "QT_HOME"]

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

def list2english(l):
    if len(l) == 1:
        return l[0]
    elif len(l) == 2:
        return l[0] + " and " + list2english(l[1:])
    else:
        return l[0] + ", " + list2english(l[1:])

def convertLicensesToRTF(toolDir):
    openssl = ([os.path.join(os.environ["OPENSSL_HOME"], "LICENSE")], "OpenSSL", "OPENSSL_LICENSE.rtf")
    berkeleydb = ([os.path.join(os.environ["DB_HOME"], "LICENSE")], "Berkeley DB", "BERKELEY_DB_LICENSE.rtf")
    expat = ([os.path.join(os.environ["EXPAT_HOME"], "COPYING.txt")], "Expat", "EXPAT_LICENSE.rtf")
    bzip2 = ([os.path.join(os.environ["BZIP2_HOME"], "LICENSE")], "bzip2/libbzip2", "BZIP2_LICENSE.rtf")
    mcpp = ([os.path.join(os.environ["MCPP_HOME"], "LICENSE")], "MCPP", "MCPP_LICENSE.rtf")
    looks = ([os.path.join(os.environ["JGOODIES_LOOKS_HOME"], "license.txt")], "JGoodies Looks", 
    	     "JGOODIES_LOOKS_LICENSE.rtf")
    forms = ([os.path.join(os.environ["JGOODIES_FORMS_HOME"], "license.txt")], "JGoodies Forms",
             "JGOODIES_FORMS_LICENSE.rtf")
    common = ([os.path.join(os.environ["JGOODIES_COMMON_HOME"], "license.txt")], "JGoodies Common",
             "JGOODIES_COMMON_LICENSE.rtf")
    stlport = ([os.path.join(os.environ["STLPORT_HOME"], "doc", "license.html")], "STLport", "STLPORT_LICENSE.rtf")
    qt = ([os.path.join(os.environ["QT_HOME"], "LICENSE.LGPL"), os.path.join(os.environ["QT_HOME"], "LGPL_EXCEPTION.txt")], "Qt", "QT_LICENSE.rtf")

    #
    # The license file isn't quite right for this.
    #
    section_header = "License agreement for %s:\n"
    line_string = "-------------------------------------------------------------------------------------------"
    rtfhdr = file(os.path.join(toolDir, "docs", "rtf.hdr")).readlines()
    rtfftr = file(os.path.join(toolDir, "docs", "rtf.footer")).readlines()

    collection = [ berkeleydb, bzip2, openssl, expat, mcpp, looks, forms, common, qt, stlport ]

    third_party_sources_file_hdr = """Source Code
-----------

"""

    if not os.path.exists(os.path.join(toolDir, "docs")):
        os.mkdir(os.path.join(toolDir, "docs"))
    if not os.path.exists(os.path.join(toolDir, "docs", "thirdparty")):
        os.mkdir(os.path.join(toolDir, "docs", "thirdparty"))
    names = []
    for e in collection:
        names.append(e[1])

    text = "The source distributions of " + list2english(names)
    text = text + " used to build this distribution can be downloaded at no cost from http://www.zeroc.com/download.html."
    licensefile = file(os.path.join(toolDir, "docs", "thirdparty", "SOURCES.txt"), "w")

    #
    # textwrap module has got to be one of the coolest things since
    # sliced bread.
    #
    licensefile.write(third_party_sources_file_hdr)
    licensefile.write(textwrap.fill(text, 75))
    licensefile.close()

    licensefile = file(os.path.join(toolDir, "docs", "thirdparty", "LICENSE.txt"), "w")
    for f in collection:
        contents = None
	for lf in f[0]:
            if lf.endswith(".html"):
                #
                # Here's me wishing the Python standard library had a class
                # for converting HTML to plain text. In the meantime, we'll
                # have to leverage 'links' in cygwin.
                #
                pipe_stdin, pipe_stdout = os.popen2("cygpath %s" % lf)
                lines = pipe_stdout.readlines()
                pipe_stdin.close()
                pipe_stdout.close()
                cygname = lines[0].strip()
                pipe_stdin, pipe_stdout = os.popen2("links -dump %s" % cygname)
                lines = pipe_stdout.readlines()
		if contents == None:
		    contents = lines[2:]
		else:
                    contents = contents + lines[2:]
            else:
		if contents == None:
		    contents = file(lf).readlines()
		else:
                    contents = contents + file(lf).readlines()
        hdr = section_header % f[1]

        licensefile.write(hdr)
        licensefile.write(line_string[:len(hdr)] + "\n\n")
        licensefile.writelines(contents)
        licensefile.write("\n\n")
        rtffile = file(os.path.join(toolDir, "docs", "thirdparty", os.path.basename(f[2])), "w")
        rtffile.writelines(rtfhdr)
        rtffile.write(hdr + "\\par")
        rtffile.write(line_string[:len(hdr)] + "\\par\n")
        for l in contents:
            rtffile.write(l.rstrip("\n") + "\\par\n")
        rtffile.writelines(rtfftr)
        rtffile.close()

    licensefile.close()
    lines = file(os.path.join(toolDir, "docs", "thirdparty", "LICENSE.txt")).readlines()
    rtflicense = file(os.path.join(toolDir, "docs", "thirdparty", "LICENSE.rtf"), "w")
    rtflicense.writelines(rtfhdr)
    for l in lines:
        rtflicense.write(l.rstrip("\n") + "\\par\n")
    rtflicense.writelines(rtfftr)
    rtflicense.close()

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
                sys.argv[1:], "dhil:", [ "help", "info", "debug", "logfile", "bcc2009", "bcc2010", "sslhome=",
		"expathome=", "dbhome=", "stlporthome=", "bzip2home=", "mcpphome=",
	        "jgoodiesformshome=", "jgoodieslookshome=", "thirdparty="])
        except getopt.GetoptError:
            usage()
            sys.exit(2)

        #
        # Set a few defaults.
        #
        debugLevel = logging.NOTSET
        logFile = None

        for o, a in optionList:
            if o in ("-h", "--help"):
                usage()
                sys.exit()
            elif o in ('-l', '--logfile'):
                logFile = a
            elif o in ('-d', '--debug'):
                debugLevel = logging.DEBUG
            elif o in ('-', '--info'):
                debugLevel = logging.INFO
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
                os.environ['JGOODIES_FORMS_HOME'] = a
            elif o == '--jgoodieslookshome':
                os.environ['JGOODIES_LOOKS_HOME'] = a
            elif o == '--thirdparty':
                os.environ['OPENSSL_HOME'] = os.path.join(a, 'VC9', 'openssl-%s' % OpenSSLVer)
                os.environ['BZIP2_HOME'] = os.path.join(a, 'VC9', 'bzip2-%s' % Bzip2Ver)
                os.environ['EXPAT_HOME'] = os.path.join(a, 'VC9', 'expat-%s' % ExpatVer)
                os.environ['DB_HOME'] = os.path.join(a, 'VC9', 'db-%s.NC' % DBVer)
                os.environ['STLPORT_HOME'] = os.path.join(a, 'VC6', 'STLPort-%s' % STLPortVer)
                os.environ['MCPP_HOME'] = os.path.join(a, 'VC9', 'mcpp-%s' % MCPPVer)
                os.environ['JGOODIES_FORMS_HOME'] = os.path.join(a, 'VC9', 'forms-%s' % FormsVer)
                os.environ['JGOODIES_LOOKS_HOME'] = os.path.join(a, 'VC9', 'looks-%s' % LooksVer)
                os.environ['QT_HOME'] = os.path.join(a, 'VC9', 'qt-win-opensource-src-%s' % QtVer)

        if debugLevel != logging.NOTSET:
            if a != None:
                logging.basicConfig(level = debugLevel, format='%(asctime)s %(levelname)s %(message)s', filename = a)
            else:
                logging.basicConfig(level = debugLevel, format='%(asctime)s %(levelname)s %(message)s')

        #
        # Where all the files will be staged so that the install projects
        # can find them.
        #
        targetDir = os.path.join(installDir, "src", "windows")

        logging.info("Install Tool: " + installDir)
        logging.info("Target Directory: " + targetDir)

        logging.info('Starting windows installer creation.')

        environmentCheck()

        logging.debug(environToString(os.environ))

        #
        # Gather and generate license files.
        #
        convertLicensesToRTF(resources)

	#
        # Build the MSI
        #
        os.chdir(targetDir)
        runprog("AdvancedInstaller.com /build ThirdParty.aip ")
        msi = "Ice-" + iceVersion + "-ThirdParty.msi"
        shutil.move("ThirdParty.msi", msi)


    finally:
        #
        # Return the user to where they started.
        #
        os.chdir(startDir)

if __name__ == "__main__":
    main()
