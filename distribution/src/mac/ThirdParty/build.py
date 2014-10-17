#!/bin/python

import os, sys

rootDir = "/opt/Ice-@ver@-ThirdParty"
volname = "Ice-@ver@-ThirdParty"

def runCommand(cmd):
    print(cmd)
    if os.system(cmd) != 0:
        sys.exit(1)

for f in ["packages", "scratch.dmg.sparseimage", ("%s.dmg" % volname), ("installer/%s.pkg" % volname)]:
    runCommand("rm -rf %s" % f)

runCommand("mkdir -p packages")

#
# Copy README,THIRD_PARTY_LICENSE and SOURCES to the root dir
#
for f in ["README.txt", "SOURCES.txt", "THIRD_PARTY_LICENSE.txt"]:
    runCommand("cp resources/%s %s" % (f, rootDir))

runCommand("pkgbuild --root " + rootDir + " " +
           "--identifier=com.zeroc.ice@ver@-thirdparty " +
           "--install-location=/Library/Developer/Ice-@ver@-ThirdParty " +
           "packages/com.zeroc.ice-@ver@-thirdParty.pkg")

runCommand("productbuild --distribution=Distribution.xml " +
           "--package-path=packages " +
           "--resources=resources  installer/%s.pkg" % volname)
runCommand("rm -rf packages")

runCommand("hdiutil create scratch.dmg -volname \"%s\" -type SPARSE -fs HFS+" % volname)
runCommand("hdid scratch.dmg.sparseimage")
runCommand("ditto -rsrc installer \"/Volumes/%s\"" % volname)
runCommand("hdiutil detach \"/Volumes/%s\"" % volname)

runCommand("hdiutil convert  scratch.dmg.sparseimage -format UDZO -o %s.dmg -imagekey zlib-devel=9" % volname)
runCommand("rm scratch.dmg.sparseimage")

runCommand("rm -rf installer/%s.pkg" % volname)

print("Package %s.dmg created ok" % volname)
