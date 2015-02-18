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


os.chdir(os.path.join(os.path.dirname(__file__), "..", ".."))
if not os.path.isfile("Ice-@ver@.tar.gz"):
    print("Could not find Ice-@ver@.tar.gz")
    sys.exit(1)

thirdPartyPackage = "ThirdParty-Sources-@ver@"
downloadUrl = "http://www.zeroc.com/download/Ice/@debmmver@/"

if not os.path.isfile(os.path.expanduser("~/Downloads/%s.tar.gz" % thirdPartyPackage)):
    runCommand(os.path.expanduser("cd ~/Downloads && wget http://www.zeroc.com/download/Ice/@debmmver@/%s.tar.gz" % thirdPartyPackage))

runCommand(os.path.expanduser(
           "rm -rf %(thirdParty)s && tar zxf ~/Downloads/%(thirdParty)s.tar.gz && cd %(thirdParty)s && tar zxf mcpp-2.7.2.tar.gz && "
           "cd mcpp-2.7.2 && patch -p0 < ../mcpp/patch.mcpp.2.7.2" % {"thirdParty": thirdPartyPackage}))

runCommand("tar zxf Ice-@ver@.tar.gz")
for d in ["IceUtil", "Slice", "slice2js"]:
    runCommand("cd Ice-@ver@/cpp/src/%s && make -j8" % d)
runCommand("cd Ice-@ver@/js && npm install && npm run gulp:dist")


packages = ["zeroc-icejs", "icejs-demos", "zeroc-slice2js", "gulp-zeroc-slice2js"]
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
    runCommand("cp -rf distfiles-@ver@/src/js/%(package)s/* packages/%(package)s/" % {"package": package})

    #
    # copy license files to each package
    #
    for f in ["LICENSE", "ICE_LICENSE"]:
        copy("Ice-@ver@/%(file)s" % {"file": f},
             "packages/%(package)s/%(file)s" % {"package": package, "file": f})


#
# zeroc-slice2js package
#
copy("%s/mcpp-2.7.2/src" % thirdPartyPackage, "packages/zeroc-slice2js/mcpp/src")

for d in ["IceUtil", "Slice", "slice2js"]:
    copyMatchingFiles(os.path.join("Ice-@ver@/cpp/src", d), os.path.join("packages/zeroc-slice2js/src", d), ["*.cpp", "*.h"])

for d in ["IceUtil", "Slice"]:
    copyMatchingFiles(os.path.join("Ice-@ver@/cpp/include", d), os.path.join("packages/zeroc-slice2js/include", d), ["*.h"])

for d in ["Glacier2", "Ice", "IceGrid", "IceSSL", "IceStorm"]:
    copyMatchingFiles(os.path.join("Ice-@ver@/slice", d), os.path.join("packages/zeroc-slice2js/slice", d), ["*.ice"])

copy("distfiles-@ver@/src/unix/MCPP_LICENSE", "packages/zeroc-slice2js/MCPP_LICENSE")

#
# gulp-zeroc-slice2js package
#
copy("Ice-@ver@/js/gulp/gulp-slice2js/index.js", "packages/gulp-zeroc-slice2js/index.js")

#
# zeroc-icejs package
#
copy("Ice-@ver@/js/src", "packages/zeroc-icejs/src")
copyMatchingFiles("Ice-@ver@/js/gulp", "packages/zeroc-icejs/gulp", ['bundle.js', 'libTasks.js'])
copyMatchingFiles("Ice-@ver@/js/lib", "packages/zeroc-icejs/lib", ["*.js", "*.gz"])

#
# zeroc-icejs-demo package
#
for f in os.listdir("Ice-@ver@/js/demo"):
    if f == "README":
        continue
    copy(os.path.join("Ice-@ver@/js/demo", f), os.path.join("packages/icejs-demos", f))
copy("Ice-@ver@/js/bin", "packages/icejs-demos/bin")
copy("Ice-@ver@/certs", "packages/icejs-demos/certs")
copy("Ice-@ver@/js/assets", "packages/icejs-demos/assets")
copy("Ice-@ver@/js/.jshintrc", "packages/icejs-demos/.jshintrc")

jshint = json.load(open("Ice-@ver@/js/.jshintrc_browser", "r"))
jshintDemo = json.load(open("Ice-@ver@/js/demo/.jshintrc_browser", "r"))

for key, value in jshintDemo["globals"].iteritems():
    jshint["globals"][key] = value
json.dump(jshint, open("packages/icejs-demos/.jshintrc_browser", "w"), indent = 4, separators=(',', ': '))

for package in packages:
    runCommand("cd packages/%(package)s && git add . && git commit . -m '%(package)s version @ver@'" %
               {"package": package})

