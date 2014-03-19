#!/usr/bin/env python
# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, fnmatch, re, getopt, atexit, shutil, subprocess, zipfile, time, threading, tempfile

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "lib")))

import BuildUtils

version = "3.5.1"

def runCommand(cmd, verbose):
    if len(cmd) > 0:
        if verbose:
            print(cmd)
        if os.system(cmd) != 0:
            sys.exit(1)

class Platform:
    
    def __init__(self, server, remoteDist, outputDir, verbose):
        self._skipDownload = server == None
        self._server = server 
        self._remoteDist = remoteDist
        self._outputDir = outputDir
        self._verbose = verbose
        
    def extractDistfilesArchive(self):
        
        self._distfiles = os.path.join(self._localDist, "distfiles-%s" % version)
        
        if not os.path.exists(self._distfiles):
            zipfile.ZipFile("%s.zip" % self._distfiles).extractall()
        else:
            print("%s already exists using it" % self._localDist)
            
    def getSourceArchive(self):
        return "%s/Ice-%s.tar.gz" % (self._localDist, version)
        
    def getDemoArchive(self):
        return "%s/Ice-%s-demos.tar.gz" % (self._localDist, version)
        
    def makeBinDist(self, verbose):
        runCommand("python %s/bin/makebindist.py" % self._distfiles, verbose)
        
    def install(self, verbose):
        #
        # Override in Platform sub classes that need to install the binary
        # distribution before testing
        #
        pass
        
    def uninstall(self, verbose):
        #
        # Override in Platform sub classes that need to uninstall the binary
        # distribution after testing
        #
        pass
        
    def run(self):
        #
        # Local dist points to the distribution containing this script
        #
        self._localDist = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

        useCacheDist = False

        if self._skipDownload:
            useCacheDist = True
        else:
            if os.path.exists(self._localDist):
                
                localCommit = None
                localDate = None
                
                remoteCommit = None
                remoteDate = None
                
                out = open("%s/README" % (self._localDist), "r")
                for line in out:
                    if line.find("Creation time:") != -1:
                        localDate = re.sub("Creation time:", "", line).strip()
                    elif line.find("Git commit:") != -1:
                        localCommit = re.sub("Git commit:", "", line).strip()
                out.close()
                
                try:        
                    out = subprocess.check_output("ssh %s \"cat %s/README\"" % (self._server, self._remoteDist), shell = True)
                    if type(out) != str:
                        out = out.decode()

                    out = out.split("\n")
                    for line in out:
                        if line.find("Creation time:") != -1:
                            remoteDate = re.sub("Creation time:", "", line).strip()
                        elif line.find("Git commit:") != -1:
                            remoteCommit = re.sub("Git commit:", "", line).strip()
                            
                except subprocess.CalledProcessError as e:
                    print(e)
                    sys.exit(1)

                if localCommit == remoteCommit and localDate == remoteDate:
                    useCacheDist = True

        if not os.path.exists(self._outputDir):
            os.makedirs(self._outputDir)
            
        if not useCacheDist:
            runCommand("scp -r %s:%s %s" % (self._server, self._remoteDist, self._outputDir), self._verbose)
        else:
            print("Using cached distribution from %s" % self._localDist)
            
            
        os.chdir(self._localDist)

        #
        # Get the Version from distFiles README
        #
        out = open("README", "r")
        for line in out:
            if line.find("Version:") != -1:
                version = re.sub("Version:", "", line).strip()
        out.close()
        
        self.extractDistfilesArchive()
        
        self.makeBinDist(verbose)
        
        self.install(verbose)
        
        command = "python %s/bin/testicedist.py" % self._distfiles
        if sys.platform != "win32" and sys.platform != "darwin":
            command += " --ice-home=%s" % self.getBinDir()
            
        runCommand(command, verbose)

        self.uninstall(verbose)
        
class Linux(Platform):
    
    def __init__(self, server, remoteDist, outputDir, verbose):
        Platform.__init__(self, server, remoteDist, outputDir, verbose)
        
        p = subprocess.Popen("uname -m", shell = True, stdout = subprocess.PIPE, stderr = subprocess.STDOUT)
        if(p.wait() != 0):
            os.exists(1)
        self._machine = p.stdout.readline().decode('UTF-8').strip()
        
    def getBinDir(self):
        return "%s/build-linux-%s/Ice-%s" % (self._localDist, version, version)
        
        
class Solaris(Platform):
    
    def __init__(self, server, remoteDist, outputDir, verbose):
        Platform.__init__(self, server, remoteDist, outputDir, verbose)
        
    def getBinDir(self):
        return "%s/build-solaris-%s/Ice-%s" % (self._localDist, version, version)
        
class Darwin(Platform):
    
    def __init__(self, server, remoteDist, outputDir, verbose):
        Platform.__init__(self, server, remoteDist, outputDir, verbose)
        
    def getBinDir(self):
        return "/Library/Developer/Ice-%s" % version
        
    def installer(self):
        return os.path.abspath(os.path.join(self._distfiles, "..", "Ice-%s.dmg" % version))
    
    def install(self, verbose):
        runCommand("hdiutil mount %s" % self.installer(), verbose)
        runCommand("installer -pkg /Volumes/Ice-%s/Ice-%s.pkg -target /" % (version, version), verbose)
        runCommand("hdiutil unmount /Volumes/Ice-%s" % version, verbose)
        
    def uninstall(self, verbose):
        runCommand("hdiutil mount %s" % self.installer(), verbose)
        runCommand("/Volumes/Ice-%s/uninstall.sh" % version, verbose)
        runCommand("hdiutil unmount /Volumes/Ice-%s" % version, verbose)
        
    def makeBinDist(self, verbose):
        if os.path.exists(self.installer()):
            print("Use existing binary distribution")
        else:
            Platform.makeBinDist(self, verbose)
        
class Windows(Platform):
    
    def __init__(self, server, remoteDist, outputDir, verbose):
        Platform.__init__(self, server, remoteDist, outputDir, verbose)

    def getSourceArchive(self):
        return "%s/Ice-%s.zip" % (self._localDist, version)
        
    def getDemoArchive(self):
        return "%s/Ice-%s-demos.zip" % (self._localDist, version)

    def installer(self):
        return os.path.abspath(os.path.join(self._distfiles, "..", "Ice-%s.msi" % version))

    def getBinDir(self):
        return BuildUtils.getIceHome(version)

    def getBinDir(self):
        return BuildUtils.getIceHome(version)

    def makeBinDist(self, verbose):
        if os.path.exists(self.installer()):
            print("Use existing binary distribution")
        else:
            print("%s don't exists create new binary distribution" % self.installer())
            runCommand("python %s/bin/makemsi.py --verbose" % self._distfiles, verbose)
        
    def install(self, verbose):
        if BuildUtils.getIceHome(version):
            self.uninstall(verbose)
        runCommand("msiexec /qr /i %s /Lv installer.log" % self.installer(), verbose)
        
    def uninstall(self, verbose):
        runCommand("msiexec /qr /x %s" % self.installer(), verbose)
#
# Program usage.
#
def usage():
    print("")
    print("Options:")
    print("  --skip-download                    Don't try to download a new distribution an use the existing distribution.")
    print("")
    print("  --server                           Server to copy distribution")
    print("")
    print("  --remote-dist                      Could be a remote or local distribution, to copy a remote distribution")
    print("                                     scp is used for example use --dist=dev.zeroc.com:/share/srcdists/3.5")
    print("")
    print("  --verbose                          Be verbose.")


skipDownload = False
server = None
remoteDist = None
outputDir = "."

verbose = False

args = None
opts = None

try:
    opts, args = getopt.getopt(sys.argv[1:], "", ["help", "verbose", "skip-download", "server=", "remote-dist=", \
                                                  "remote-dist=", "output-dir="])
except getopt.GetoptError as e:
    print("Error %s " % e)
    usage()
    sys.exit(1)

if args:
    usage()
    sys.exit(1)

for o, a in opts:
    if o == "--skip-download":
        skipDownload = True
    elif o == "--server":
        server = a
    elif o == "--remote-dist":
        remoteDist = a
    elif o == "--verbose":
        verbose = True

platform = None

if sys.platform == "win32":
    platform = Windows(server, remoteDist, outputDir, verbose)
elif sys.platform == "sunos5":
   platform = Solaris(server, remoteDist, outputDir, verbose)
elif sys.platform == "darwin":
    platform = Darwin(server, remoteDist, outputDir, verbose)
elif sys.platform.startswith("linux"):
    platform = Linux(server, remoteDist, outputDir, verbose)
else:
    print("Unknown platform: %s" % sys.platform)
    sys.exit(1)
    
platform.run()


