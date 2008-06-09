#!/usr/bin/env python

import os, sys, FixUtil

def usage():
    print "Usage: " + sys.argv[0] + " [options]"
    print
    print "Options:"
    print "-h    Show this message."
    print

for x in sys.argv[1:]:
    if x == "-h":
        usage()
        sys.exit(0)
    elif x.startswith("-"):
        print sys.argv[0] + ": unknown option `" + x + "'"
        print
        usage()
        sys.exit(1)

ice_dir = os.path.normpath(os.path.join(os.path.dirname(__file__)))

FixUtil.replaceAllCopyrights(ice_dir, False, False)
for dir in ["slice", "cpp", "java", "cs", "vb", "php", "py", "rb", "demoscript", "distribution", "config", "certs"]:
    home = os.path.join(ice_dir, dir)
    if home:
       FixUtil.replaceAllCopyrights(home, False, True)

#
# Fix various other files that have copyright info in them that
# are not taken care of above.
#
cpatMatch = "20[0-9][0-9]-(20[0-9][0-9]) ZeroC"
copyright = "2008"

files = FixUtil.find(ice_dir, "*.rc")
files += FixUtil.find(ice_dir, "*LICENSE")
files += FixUtil.find(os.path.join(ice_dir, "cpp", "src"), "Gen.cpp")
files += FixUtil.find(os.path.join(ice_dir, "cpp", "src"), "Parser.cpp")
files += FixUtil.find(os.path.join(ice_dir, "cpp", "src", "Slice"), "*Util.cpp")
files += [os.path.join(ice_dir, "cpp", "src", "ca", "iceca")]
files += [os.path.join(ice_dir, "cpp", "doc", "symboltree.js")]
files += [os.path.join(ice_dir, "cpp", "demo", "Freeze", "backup", "backup")]
files += FixUtil.find(os.path.join(ice_dir, "cpp"), "*.bat")
files += [os.path.join(ice_dir, "cpp", "test", "IceSSL", "certs", "makecerts")]
files += [os.path.join(ice_dir, "java", "bin", "icegridgui.rpm")]
files += [os.path.join(ice_dir, "java", "src", "IceGridGUI", "Coordinator.java")]
files += FixUtil.find(os.path.join(ice_dir, "java", "resources", "IceGridAdmin"), "icegridadmin_content_*.html")
files += [os.path.join(ice_dir, "config", "makeprops.py")]
files += FixUtil.find(os.path.join(ice_dir), "AssemblyInfo.cs")
files += FixUtil.find(os.path.join(ice_dir, "distribution", "src", "rpm"), "*")
files += FixUtil.find(os.path.join(ice_dir, "php"), "*.php")
files += [os.path.join(ice_dir, "cpp", "test", "Slice", "errorDetection", "IllegalIdentifier.ice")]

for f in files:
    FixUtil.fileMatchAndReplace(f, [(cpatMatch, copyright)])
