#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, fnmatch, re, getopt, atexit, shutil, subprocess, threading


i386Packages = [
    "libjs-zeroc-ice_@debver@-1_i386.deb",
    "libzeroc-freeze@debmmver@_@debver@-1_i386.deb",
    "libzeroc-ice@debmmver@_@debver@-1_i386.deb",
    "libzeroc-ice-dev_@debver@-1_i386.deb",
    "libzeroc-ice-java_@debver@-1_i386.deb",
    "libzeroc-icestorm@debmmver@_@debver@-1_i386.deb",
    "php5-zeroc-ice_@debver@-1_i386.deb",
    "php5-zeroc-ice-dev_@debver@-1_i386.deb",
    "python-zeroc-ice_@debver@-1_i386.deb",
    "python-zeroc-ice-dev_@debver@-1_i386.deb",
    "ruby-zeroc-ice_@debver@-1_i386.deb",
    "ruby-zeroc-ice-dev_@debver@-1_i386.deb",
    "zeroc-glacier2_@debver@-1_i386.deb",
    "zeroc-ice@debmmver@_@debver@-1_i386.changes",
    "zeroc-icebox_@debver@-1_i386.deb",
    "zeroc-icegrid_@debver@-1_i386.deb",
    "zeroc-icepatch2_@debver@-1_i386.deb",
    "zeroc-ice-utils_@debver@-1_i386.deb"]

amd64Packages = [
    "libjs-zeroc-ice_@debver@-1_amd64.deb",
    "libzeroc-freeze@debmmver@_@debver@-1_amd64.deb",
    "libzeroc-ice@debmmver@_@debver@-1_amd64.deb",
    "libzeroc-ice-dev_@debver@-1_amd64.deb",
    "libzeroc-ice-java_@debver@-1_amd64.deb",
    "libzeroc-icestorm@debmmver@_@debver@-1_amd64.deb",
    "php5-zeroc-ice_@debver@-1_amd64.deb",
    "php5-zeroc-ice-dev_@debver@-1_amd64.deb",
    "python-zeroc-ice_@debver@-1_amd64.deb",
    "python-zeroc-ice-dev_@debver@-1_amd64.deb",
    "ruby-zeroc-ice_@debver@-1_amd64.deb",
    "ruby-zeroc-ice-dev_@debver@-1_amd64.deb",
    "zeroc-glacier2_@debver@-1_amd64.deb",
    "zeroc-ice@debmmver@_@debver@-1_amd64.changes",
    "zeroc-icebox_@debver@-1_amd64.deb",
    "zeroc-icegrid_@debver@-1_amd64.deb",
    "zeroc-icepatch2_@debver@-1_amd64.deb",
    "zeroc-ice-utils_@debver@-1_amd64.deb"]

noarchPackages = [
    "node-zeroc-ice_@debver@-1_all.deb",
    "node-zeroc-ice-dev_@debver@-1_all.deb",
    "zeroc-ice@debmmver@_@debver@-1.debian.tar.gz",
    "zeroc-ice@debmmver@_@debver@-1.dsc",
    "zeroc-ice@debmmver@_@debver@.orig.tar.gz",
    "zeroc-ice-utils-java_@debver@-1_all.deb",
    "zeroc-ice-slice_@debver@-1_all.deb",
    "zeroc-ice-all-runtime_@debver@-1_all.deb",
    "zeroc-ice-all-dev_@debver@-1_all.deb"]


def runCommand(cmd):
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
TreeDefault::Directory "pool/main/";
TreeDefault::SrcDirectory "pool/main/";
Default::Packages::Extensions ".deb";
Default::Packages::Compress ". gzip bzip2";
Default::Sources::Compress ". gzip bzip2";
Default::Contents::Compress ". gzip bzip2";
BinDirectory "dists/%(codename)s/main" {
  Packages "dists/%(codename)s/main/binary-i386/Packages";
  Packages "dists/%(codename)s/main/binary-amd64/Packages";
  Contents "dists/%(codename)s/Contents-i386";
  Contents "dists/%(codename)s/Contents-amd64";
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
    "pool/main/i/ice@debmmver@"]


buildCommand = """
%(sshCommand)s %(sshUser)s@%(sshBuildHost)s 'rm -rf %(buildDir)s && mkdir %(buildDir)s' && \\
%(scpCommand)s %(distFiles)s %(sshUser)s@%(sshBuildHost)s:/home/%(sshUser)s/%(buildDir)s && \\
%(scpCommand)s %(sourceDist)s %(sshUser)s@%(sshBuildHost)s:/home/%(sshUser)s/%(buildDir)s && \\
%(sshCommand)s %(sshUser)s@%(sshBuildHost)s \\
    'cd %(buildDir)s && tar zxf %(distFiles)s && python distfiles-@ver@/bin/makeubuntupackages.py -b %(codename)s -d . > build-log.txt 2>&1'
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
sourceDist = "zeroc-ice@debmmver@_@debver@.orig.tar.gz"
distFiles = "distfiles-@ver@.tar.gz"
signKey = "A62B38F8" if "@ver@".endswith("b") else "53A473EE"
sshKey = None
sshUser = os.environ.get('USER')
i386BuildHost = None
amd64BuildHost = None
buildDir = "dist-HEAD"
skipBuild = False

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

try:
    opts, args = getopt.getopt(sys.argv[1:], "", ["help", "ssh-key=", "ssh-user=", "i386-build-host=",
                                                  "amd64-build-host=", "build-dir=", "codename=", "skip-build"])
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
                                        "sourceDist": sourceDist})
        thread.start()
        buildThreads.append(thread)


    for thread in buildThreads:
        thread.join()
    
runCommand("mkdir -p ubuntu")
for d in repoLayout:
    runCommand("cd ubuntu && mkdir -p %s" % (d % {"codename": codename}))

for host in [i386BuildHost, amd64BuildHost]:
    packages = i386Packages + noarchPackages if host == i386BuildHost else amd64Packages
    for package in packages:
        runCommand("cd ubuntu && %(scpCommand)s %(sshUser)s@%(sshBuildHost)s:/home/%(sshUser)s/%(buildDir)s/%(codename)s/%(package)s pool/main/i/ice@debmmver@/" % 
                {"scpCommand": scpCommand,
                 "sshUser": sshUser,
                 "sshBuildHost": host,
                 "buildDir": buildDir,
                 "codename": codename,
                 "package": package})

f = open('ubuntu/aptftp.conf', 'w')
f.write(aptftpConf % {"codename": codename})
f.close()

f = open('ubuntu/aptgenerate.conf', 'w')
f.write(aptgenerateConf % {"codename": codename})
f.close()


runCommand("cd ubuntu && apt-ftparchive generate aptgenerate.conf")
runCommand("cd ubuntu && apt-ftparchive release -c=aptftp.conf dists/%(codename)s > dists/%(codename)s/Release" % {"codename": codename})

#
# Sign packages
#
for command in ["cd ubuntu && debsign -k %(signKey)s pool/main/i/ice@debmmver@/zeroc-ice@debmmver@_@debver@-1.dsc",
                "cd ubuntu && debsign -k %(signKey)s pool/main/i/ice@debmmver@/zeroc-ice@debmmver@_@debver@-1_i386.changes",
                "cd ubuntu && debsign -k %(signKey)s pool/main/i/ice@debmmver@/zeroc-ice@debmmver@_@debver@-1_amd64.changes",
                "cd ubuntu && gpg -u %(signKey)s -bao dists/raring/Release.gpg dists/raring/Release"]:
    runCommand(command % {"signKey": signKey})

runCommand("rm -f ubuntu/aptftp.conf ubuntu/aptgenerate.conf")
runCommand("tar cjvf ubuntu ubuntu.tar.bz2")
