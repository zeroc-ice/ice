#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, fnmatch, re, getopt, atexit, shutil, subprocess, threading, getpass

iceVersion = "3.6.0"
debVersion = "3.6.0"
mmVersion = "3.6"

i386Packages = [
    "libzeroc-freeze%(mmver)s_%(debver)s-1000_i386.deb" % { "mmver": mmVersion, "debver": debVersion },
    "libzeroc-ice%(mmver)s_%(debver)s-1000_i386.deb" % { "mmver": mmVersion, "debver": debVersion },
    "libzeroc-ice-dev_%(debver)s-1000_i386.deb" % { "debver": debVersion },
    "libzeroc-ice-java_%(debver)s-1000_i386.deb" % { "debver": debVersion },
    "libzeroc-icestorm%(mmver)s_%(debver)s-1000_i386.deb" % { "mmver": mmVersion, "debver": debVersion },
    "php5-zeroc-ice_%(debver)s-1000_i386.deb" % { "debver": debVersion },
    "php5-zeroc-ice-dev_%(debver)s-1000_i386.deb" % { "debver": debVersion },
    "zeroc-glacier2_%(debver)s-1000_i386.deb" % { "debver": debVersion },
    "zeroc-ice%(mmver)s_%(debver)s-1000_i386.changes" % { "mmver": mmVersion, "debver": debVersion },
    "zeroc-icebox_%(debver)s-1000_i386.deb" % { "debver": debVersion },
    "zeroc-icegrid_%(debver)s-1000_i386.deb" % { "debver": debVersion },
    "zeroc-icepatch2_%(debver)s-1000_i386.deb" % { "debver": debVersion },
    "zeroc-ice-utils_%(debver)s-1000_i386.deb" % { "debver": debVersion }]

amd64Packages = [
    "libzeroc-freeze%(mmver)s_%(debver)s-1000_amd64.deb" % { "mmver": mmVersion, "debver": debVersion },
    "libzeroc-ice%(mmver)s_%(debver)s-1000_amd64.deb" % { "mmver": mmVersion, "debver": debVersion },
    "libzeroc-ice-dev_%(debver)s-1000_amd64.deb" % { "debver": debVersion },
    "libzeroc-ice-java_%(debver)s-1000_amd64.deb" % { "debver": debVersion },
    "libzeroc-icestorm%(mmver)s_%(debver)s-1000_amd64.deb" % { "mmver": mmVersion, "debver": debVersion },
    "php5-zeroc-ice_%(debver)s-1000_amd64.deb" % { "debver": debVersion },
    "php5-zeroc-ice-dev_%(debver)s-1000_amd64.deb" % { "debver": debVersion },
    "zeroc-glacier2_%(debver)s-1000_amd64.deb" % { "debver": debVersion },
    "zeroc-ice%(mmver)s_%(debver)s-1000_amd64.changes" % { "mmver": mmVersion, "debver": debVersion },
    "zeroc-icebox_%(debver)s-1000_amd64.deb" % { "debver": debVersion },
    "zeroc-icegrid_%(debver)s-1000_amd64.deb" % { "debver": debVersion },
    "zeroc-icepatch2_%(debver)s-1000_amd64.deb" % { "debver": debVersion },
    "zeroc-ice-utils_%(debver)s-1000_amd64.deb" % { "debver": debVersion }]

noarchPackages = [
    "zeroc-ice%(mmver)s_%(debver)s-1000.debian.tar.gz" % { "mmver": mmVersion, "debver": debVersion },
    "zeroc-ice%(mmver)s_%(debver)s-1000.dsc" % { "mmver": mmVersion, "debver": debVersion },
    "zeroc-ice%(mmver)s_%(debver)s.orig.tar.gz" % { "mmver": mmVersion, "debver": debVersion },
    "zeroc-ice-utils-java_%(debver)s-1000_all.deb" % { "debver": debVersion },
    "zeroc-ice-slice_%(debver)s-1000_all.deb" % { "debver": debVersion },
    "zeroc-ice-all-runtime_%(debver)s-1000_all.deb" % { "mmver": mmVersion, "debver": debVersion },
    "zeroc-ice-all-dev_%(debver)s-1000_all.deb" % { "debver": debVersion }]


def runCommand(cmd, verbose = False):
    if verbose:
        print(cmd)
    if os.system(cmd) != 0:
        sys.exit(1)

aptftpConf = """
APT::FTPArchive::Release {
  Origin "ZeroC Ice";
  Label "ZeroC Ice";
  Suite "unstable";
  Codename "%(codename)s";
  Architectures "all amd64 i386";
  Components "main";
  Description "Apt repository for ZeroC Ice";
};
"""
aptgenerateConf = """
Dir::ArchiveDir ".";
Dir::CacheDir ".";
TreeDefault::Directory "pool/%(codename)s/main/";
TreeDefault::SrcDirectory "pool/%(codename)s/main/";
Default::Packages::Extensions ".deb";
Default::Packages::Compress ". gzip bzip2";
Default::Sources::Compress ". gzip bzip2";
Default::Contents::Compress ". gzip bzip2";

BinDirectory "dists/%(codename)s/main" {
  Packages "dists/%(codename)s/main/binary-i386/Packages";
  Packages "dists/%(codename)s/main/binary-amd64/Packages";
  Contents "dists/%(codename)s/main/Contents-i386";
  Contents "dists/%(codename)s/main/Contents-amd64";
  SrcPackages "dists/%(codename)s/main/source/Sources";
};

Tree "dists/%(codename)s" {
  Sections "main";
  Architectures "amd64 i386 source";
};
"""

repoLayout = [
    "dists/%(codename)s/main/binary-i386",
    "dists/%(codename)s/main/source",
    "dists/%(codename)s/main/binary-amd64",
    "pool/%(codename)s/main/i/ice" + mmVersion]

buildCommand = """
%(sshCommand)s %(sshUser)s@%(sshBuildHost)s 'rm -rf %(buildDir)s && mkdir %(buildDir)s' && \\
%(scpCommand)s %(distFiles)s %(sshUser)s@%(sshBuildHost)s:/home/%(sshUser)s/%(buildDir)s && \\
%(scpCommand)s %(sourceDist)s %(sshUser)s@%(sshBuildHost)s:/home/%(sshUser)s/%(buildDir)s && \\
%(scpCommand)s %(keystore)s %(sshUser)s@%(sshBuildHost)s:/home/%(sshUser)s/%(buildDir)s/keystore.jks && \\
%(sshCommand)s %(sshUser)s@%(sshBuildHost)s \\
    'cd %(buildDir)s && tar zxf %(distFiles)s && SIGNJARS=yes JARSIGNER_KEYSTORE=/home/%(sshUser)s/%(buildDir)s/keystore.jks JARSIGNER_KEYSTORE_PASSWORD="%(storepass)s" python distfiles-%(version)s/bin/makeubuntupackages.py -b %(codename)s -d . > build-log.txt 2>&1 && rm /home/%(sshUser)s/%(buildDir)s/keystore.jks'
"""

class Builder(threading.Thread):
    def __init__(self, command):
        self._status = 0
        self._command = command
        threading.Thread.__init__(self)

    def run(self):
        _status = runCommand(self._command)

    def status():
        return _status

#
# Default values
#
codename = "trusty"
sourceDist = "zeroc-ice" + mmVersion + "_" + debVersion + ".orig.tar.gz"
distFiles = "distfiles-%s.tar.gz" % iceVersion
signKey = "A62B38F8" if iceVersion.endswith("b") else "53A473EE"
sshKey = None
sshUser = os.environ.get('USER')
i386BuildHost = None
amd64BuildHost = None
buildDir = "dist-HEAD"
skipBuild = False
verbose = False

#
# Program usage.
#
def usage():
    print("")
    print(r"Options:")
    print("")
    print(r"  --help                      Show this message.")
    print("")
    print(r"  --ssh-key                   SSH key used to sign into build host.")
    print("")
    print(r"  --ssh-user                  SSH user used to sign into build host.")
    print("")
    print(r"  --i386-build-host           ip address or hostname for the i386 package builder.")
    print("")
    print(r"  --amd64-build-host          ip address or hostname for the amd64 package builder.")
    print("")
    print(r"  --build-dir                 directory to build the packages, it is relative to the user home.")
    print("")
    print(r"  --codename                  Ubuntu release codename we are building (trusty, utopic, ...).")
    print("")
    print(r"  --skip-build                Don't build packages, just recreate the repository.")
    print("")
    print(r"  --jarsigner-keystore        Path to the keystore used to sign icegridgui.jar.")
    print("")
    print(r"  --verbose                   Verbose output.")
    print("")

try:
    opts, args = getopt.getopt(sys.argv[1:], "", ["help", "ssh-key=", "ssh-user=", "i386-build-host=",
                                                  "amd64-build-host=", "build-dir=", "codename=", "skip-build",
                                                  "jarsigner-keystore=", "verbose"])
except getopt.GetoptError as e:
    print("Error %s " % e)
    usage()
    sys.exit(1)

if args:
    usage()
    sys.exit(1)

for o, a in opts:
    if o == "--help":
        usage()
        sys.exit(0)
    elif o == "--ssh-key":
        sshKey = a
    elif o == "--ssh-user":
        sshUser = a
    elif o == "--i386-build-host":
        i386BuildHost = a
    elif o == "--amd64-build-host":
        amd64BuildHost = a
    elif o == "--build-dir":
        buildDir = a
    elif o == "--codename":
        codename = a
    elif o == "--skip-build":
        skipBuild = True
    elif o == "--jarsigner-keystore":
        keystore = a
    elif o == "--verbose":
        verbose = True

storepass = getpass.getpass("Keystore password:")

sshCommand = "ssh" if sshKey == None else "ssh -i %s" % sshKey
scpCommand = "scp" if sshKey == None else "scp -i %s" % sshKey

buildHosts = []

if i386BuildHost != None:
    buildHosts.append(i386BuildHost)

if amd64BuildHost != None:
    buildHosts.append(amd64BuildHost)

if not skipBuild:
    if not buildHosts:
        usage()
        print("You must set the builder host(s) using --i386-build-host and/or --amd64-build-host options")
        exit(1)

    buildThreads = []
    for host in buildHosts:
        thread = Builder(buildCommand % {"sshCommand": sshCommand,
                                         "scpCommand": scpCommand,
                                         "sshUser": sshUser,
                                         "sshBuildHost": host,
                                         "buildDir": buildDir,
                                         "signKey": signKey,
                                         "codename": codename,
                                         "distFiles": distFiles,
                                         "sourceDist": sourceDist,
                                         "keystore": keystore,
                                         "storepass": storepass,
                                         "version": iceVersion})
        thread.start()
        buildThreads.append(thread)


    for thread in buildThreads:
        thread.join()

runCommand("mkdir -p ubuntu", verbose)

for d in repoLayout:
    runCommand("cd ubuntu && mkdir -p %s" % (d % {"codename": codename}), verbose)

for host in [i386BuildHost, amd64BuildHost]:
    packages = i386Packages + noarchPackages if host == i386BuildHost else amd64Packages
    for package in packages:
        runCommand("cd ubuntu && %(scpCommand)s %(sshUser)s@%(sshBuildHost)s:/home/%(sshUser)s/%(buildDir)s/%(codename)s/%(package)s pool/%(codename)s/main/i/ice" + mmVersion + "/" %
                {"scpCommand": scpCommand,
                 "sshUser": sshUser,
                 "sshBuildHost": host,
                 "buildDir": buildDir,
                 "codename": codename,
                 "package": package}, verbose)

#
# Sign the source package
#
for command in ["cd ubuntu && echo \"y\" | debsign -k %(signKey)s pool/%(codename)s/main/i/ice%(mmver)s/zeroc-ice%(mmver)s_%(debver)s-1000.dsc"]:
    runCommand(command % {"signKey": signKey, "codename": codename, "mmver": mmVersion, "debver": debVersion}, verbose)

f = open('ubuntu/aptftp.conf', 'w')
f.write(aptftpConf % {"codename": codename})
f.close()

f = open('ubuntu/aptgenerate.conf', 'w')
f.write(aptgenerateConf % {"codename": codename})
f.close()


runCommand("cd ubuntu && apt-ftparchive generate aptgenerate.conf", verbose)
runCommand("cd ubuntu && apt-ftparchive release -c=aptftp.conf dists/%(codename)s > dists/%(codename)s/Release" % {"codename": codename}, verbose)

#
# Sign changes and release files
#
for command in ["cd ubuntu && echo \"y\" | debsign -k %(signKey)s pool/%(codename)s/main/i/ice%(mmver)s/zeroc-ice%(mmver)s_%(debver)s-1000_i386.changes",
                "cd ubuntu && echo \"y\" | debsign -k %(signKey)s pool/%(codename)s/main/i/ice%(mmver)s/zeroc-ice%(mmver)s_%(debver)s-1000_amd64.changes",
                "cd ubuntu && gpg --yes -u %(signKey)s -bao dists/%(codename)s/Release.gpg dists/%(codename)s/Release"]:
    runCommand(command % {"signKey": signKey, "codename": codename, "mmver": mmVersion, "debver": debVersion}, verbose)

runCommand("rm -f ubuntu/aptftp.conf ubuntu/aptgenerate.conf", verbose)
runCommand("tar cjf ubuntu.tar.bz2 ubuntu", verbose)
