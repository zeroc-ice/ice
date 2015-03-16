#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, fnmatch, re, getopt, atexit, shutil, subprocess, json

sys.path.append(os.path.join(os.path.dirname(__file__), "..", "lib"))
from DistUtils import *

#
# Program usage.
#
def usage():
    print
    print "Options:"
    print "-h               Show this message."
    print "-v               Be verbose."
    print "Example:"
    print ""
    print "makejspackages.py"
    print

def runCommand(cmd):
    print(cmd)
    if os.system(cmd) != 0:
        sys.exit(1)

iceVersion = "3.6.0"
mmVersion = "3.6"

os.chdir(os.path.join(os.path.dirname(__file__), "..", ".."))
if not os.path.isfile("Ice-%s.tar.gz" % iceVersion):
    print("Could not find Ice-%s.tar.gz" % iceVersion)
    sys.exit(1)

thirdPartyPackage = "ThirdParty-Sources-%s" % iceVersion
downloadUrl = "http://www.zeroc.com/download/Ice/%s/" % mmVersion

if not os.path.isfile(os.path.expanduser("~/Downloads/%s.tar.gz" % thirdPartyPackage)):
    runCommand(os.path.expanduser("cd ~/Downloads && wget http://www.zeroc.com/download/Ice/%(mmver)s/%(thirdParty)s.tar.gz" %
                                  {"thirdParty": thirdPartyPackage, "mmver": mmVersion}))

runCommand(os.path.expanduser(
           "rm -rf %(thirdParty)s && tar zxf ~/Downloads/%(thirdParty)s.tar.gz && cd %(thirdParty)s && tar zxf mcpp-2.7.2.tar.gz && "
           "cd mcpp-2.7.2 && patch -p0 < ../mcpp/patch.mcpp.2.7.2" % {"thirdParty": thirdPartyPackage}))

runCommand("tar zxf Ice-%s.tar.gz" % iceVersion)
for d in ["IceUtil", "Slice", "slice2js"]:
    runCommand("cd Ice-%(version)s/cpp/src/%(dir)s && make -j8" % { "dir": d, "version": iceVersion })
runCommand("cd Ice-%s/js && npm install && npm run gulp:dist" % iceVersion)


packages = ["zeroc-icejs", "zeroc-slice2js", "gulp-zeroc-slice2js"]
#
# Clone package git repositories
#
runCommand("rm -rf packages && mkdir packages")
for repo in packages:
    runCommand("cd packages && git clone ssh://dev.zeroc.com/home/git/%(repo)s.git" % {"repo": repo})
    runCommand("cd packages/%(repo)s && git remote add github git@github.com:ZeroC-Inc/%(repo)s.git" % {"repo": repo})
    runCommand("cd packages/%(repo)s && git config user.name 'ZeroC, Inc'" % {"repo": repo})
    runCommand("cd packages/%(repo)s && git config user.email 'github@zeroc.com'" % {"repo": repo})
    runCommand("cd packages/%(repo)s && rm -rf *" % {"repo": repo})

for package in packages:
    #
    # copy dist files to repositories
    #
    runCommand("cp -rf distfiles-%(version)s/src/js/%(package)s/* packages/%(package)s/" %
               { "package": package, "version": iceVersion })

    #
    # copy license files to each package
    #
    for f in ["LICENSE", "ICE_LICENSE"]:
        copy("Ice-%(version)s/%(file)s" % { "file": f, "version": iceVersion },
             "packages/%(package)s/%(file)s" % { "package": package, "file": f })


#
# zeroc-slice2js package
#
copy("%s/mcpp-2.7.2/src" % thirdPartyPackage, "packages/zeroc-slice2js/mcpp/src")

for d in ["IceUtil", "Slice", "slice2js"]:
    copyMatchingFiles(os.path.join("Ice-%s/cpp/src" % iceVersion, d),
                      os.path.join("packages/zeroc-slice2js/src", d), ["*.cpp", "*.h"])

for d in ["IceUtil", "Slice"]:
    copyMatchingFiles(os.path.join("Ice-%s/cpp/include" % iceVersion, d),
                      os.path.join("packages/zeroc-slice2js/include", d), ["*.h"])

for d in ["Glacier2", "Ice", "IceGrid", "IceSSL", "IceStorm"]:
    copyMatchingFiles(os.path.join("Ice-%s/slice" % iceVersion, d),
                      os.path.join("packages/zeroc-slice2js/slice", d), ["*.ice"])

copy("distfiles-%s/src/unix/MCPP_LICENSE" % iceVersion, "packages/zeroc-slice2js/MCPP_LICENSE")

#
# gulp-zeroc-slice2js package
#
copy("Ice-%s/js/gulp/gulp-slice2js/index.js" % iceVersion, "packages/gulp-zeroc-slice2js/index.js")

#
# zeroc-icejs package
#
copy("Ice-%s/js/src" % iceVersion, "packages/zeroc-icejs/src")
copyMatchingFiles("Ice-%s/js/gulp" % iceVersion, "packages/zeroc-icejs/gulp", ['bundle.js', 'libTasks.js'])
copyMatchingFiles("Ice-%s/js/lib" % iceVersion, "packages/zeroc-icejs/lib", ["*.js", "*.gz"])

for package in packages:
    runCommand("cd packages/%(package)s && git add . && git commit . -m '%(package)s version %(version)s'" %
               { "package": package, "version": iceVersion })

