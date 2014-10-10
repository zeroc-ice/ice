#!/bin/python

import os, sys

verbose = True
volname = "ThirdParty-@ver@"

def runCommand(cmd, verbose):
    if len(cmd) > 0:
        if verbose:
            print(cmd)
        if os.system(cmd) != 0:
            sys.exit(1)

runCommand("mkdir -p packages", verbose)

runCommand("pkgbuild --root /opt/Ice-@ver@-ThirdParty " +
           "--identifier=com.zeroc.ice@ver@-thirdparty " +
           "--install-location=/Library/Developer/Ice-@ver@-ThirdParty " +
           "packages/com.zeroc.ice-@ver@-thirdParty.pkg", verbose)

runCommand("productbuild --distribution=Distribution.xml " +
           "--package-path=packages " +
           "--resources=resources  installer/ThirdParty-@ver@.pkg", verbose)

runCommand("rm -rf packages", verbose)

if os.path.exists("scratch.dmg.sparseimage"):
    os.remove("scratch.dmg.sparseimage")

runCommand("hdiutil create scratch.dmg -volname \"%s\" -type SPARSE -fs HFS+" % volname, verbose)
runCommand("hdid scratch.dmg.sparseimage", verbose)
runCommand("ditto -rsrc installer \"/Volumes/%s\"" % volname, verbose)
runCommand("hdiutil detach \"/Volumes/%s\"" % volname, verbose)

if os.path.exists(volname + ".dmg"):
    os.remove(volname + ".dmg")

runCommand("hdiutil convert  scratch.dmg.sparseimage -format UDZO -o %s.dmg -imagekey zlib-devel=9" % volname, verbose)
runCommand("rm scratch.dmg.sparseimage", verbose)

os.remove("installer/ThirdParty-@ver@.pkg")

print("ok")
