#!/usr/bin/env python

import os, sys, shutil, fnmatch, re, glob, getopt

#
# Program usage.
#
def usage():
    print "Usage: " + sys.argv[0] + " [-e] version"
    print
    print "Options:"
    print "-e                Fix version for Ice-E instead of Ice."
    print "-h, --help        Show this message."
    print


def intVersion(version):
    r = re.search("([0-9]*)\.([0-9]*)\.([0-9]*)", version)
    major = int(r.group(1))
    minor = int(r.group(2))
    patch = int(r.group(3))
    return ("%2d%02d%02d" % (major, minor, patch)).strip()

def soVersion(version):
    r = re.search("([0-9]*)\.([0-9]*)\.([0-9]*)", version)
    major = int(r.group(1))
    minor = int(r.group(2))
    return ("%d%d" % (major, minor)).strip()


#
# Find files matching a pattern.
#
def find(path, patt):
    result = [ ]
    files = os.listdir(path)
    for x in files:
        fullpath = os.path.join(path, x);
        if os.path.isdir(fullpath) and not os.path.islink(fullpath):
            result.extend(find(fullpath, patt))
        elif fnmatch.fnmatch(x, patt):
            result.append(fullpath)
    return result

def findSourceTree(tree, file):
    for path in [".", "..", "../..", "../../..", "../../../.."]:
        path = os.path.normpath(path)
        if os.path.exists(os.path.join(path, file)):
            break
        path = os.path.join(path, tree)
        if os.path.exists(os.path.join(path, file)):
            break
        path = None
    if not path:
        print "warning: can't find " + tree + " directory!"
    return path

#
# Replace a string matched by the first group of regular expression.
#
# For example: the regular expression "ICE_STRING_VERSION \"([0-9]*\.[0-9]*\.[0-9]*)\""
# will match the string version in "ICE_STRING_VERSION "2.1.0"" and will replace it with
# the given version.
#
def fileMatchAndReplace(filename, matchAndReplaceExps):

    oldConfigFile = open(filename, "r")
    newConfigFile = open(filename + ".new", "w")

    #
    # Compile the regular expressions
    #
    regexps = [ ]
    for (regexp, replace) in matchAndReplaceExps:
        regexps.append((re.compile(regexp), replace))

    #
    # Search for the line with the given regular expressions and
    # replace the matching string
    #
    updated = False
    for line in oldConfigFile.readlines():
        for (regexp, replace) in regexps:
            match = regexp.search(line)
            if match != None:
                oldLine = line
                line = oldLine.replace(match.group(1), replace)
#                print oldLine + line
                updated = True
                break
        newConfigFile.write(line)

    newConfigFile.close()
    oldConfigFile.close()

    if updated:
        print "updated " + filename
        os.rename(filename + ".new", filename)
    else:
        print "warning: " + filename + " didn't contain any version"
        os.unlink(filename + ".new")

#
# Replace all occurences of a regular expression in a file
#
def fileMatchAllAndReplace(filename, matchAndReplaceExps):

    oldFile = open(filename, "r")
    newFile = open(filename + ".new", "w")

    #
    # Compile the regular expressions
    #
    regexps = [ ]
    for (regexp, replace) in matchAndReplaceExps:
        regexps.append((re.compile(regexp), replace))

    #
    # Search for all lines with the given regular expressions and
    # replace the matching string
    #
    updated = False
    for line in oldFile.readlines():
	for (regexp, replace) in regexps:
	    match = regexp.search(line)
	    if match != None:
		oldLine = line
		line = oldLine.replace(match.group(1), replace)
		updated = True
	newFile.write(line)

    newFile.close()
    oldFile.close()

    if updated:
        print "updated " + filename
        os.rename(filename + ".new", filename)
    else:
        print "warning: " + filename + " didn't contain any version"
        os.unlink(filename + ".new")


if len(sys.argv) < 2:
    usage()
    sys.exit(0)

patchIceE = False
try:
    opts, args = getopt.getopt(sys.argv[1:], "he", ["help"])
except getopt.GetoptError:
    usage()
    sys.exit(1)
for o, a in opts:
    if o in ("-h", "--help"):
        usage()
	sys.exit(0)
    if o in ("-e"):
        patchIceE = True
if len(args) != 1:
    usage()
    sys.exit(1)

version = args[0]
if not re.match("[0-9]*\.[0-9]*\.[0-9]*$", version):
    print "invalid version number: " + version + " (it should have the form A.B.C)"
    sys.exit(0)

if not patchIceE:
    #
    # Fix version in Ice sources
    #
    ice_home = findSourceTree("ice", os.path.join("include", "IceUtil", "Config.h"))
    if ice_home:
	fileMatchAndReplace(os.path.join(ice_home, "include", "IceUtil", "Config.h"),
			    [("ICE_STRING_VERSION \"([0-9]*\.[0-9]*\.[0-9]*)\"", version), \
			     ("ICE_INT_VERSION ([0-9]*)", intVersion(version))])

	fileMatchAndReplace(os.path.join(ice_home, "config", "Make.rules"),
			    [("VERSION[\t\s]*= ([0-9]*\.[0-9]*\.[0-9]*)", version),
			     ("SOVERSION[\t\s]*= ([0-9]*)", soVersion(version))])
	
	fileMatchAllAndReplace(os.path.join(ice_home, "src", "Freeze", "freeze.dsp"),
			       [("freeze([0-9][0-9])d?\.((dll)|(pdb))", soVersion(version))])
	fileMatchAllAndReplace(os.path.join(ice_home, "src", "Glacier2", "glacier2.dsp"),
			       [("glacier2([0-9][0-9])d?\.((dll)|(pdb))", soVersion(version))])
	fileMatchAllAndReplace(os.path.join(ice_home, "src", "Ice", "ice.dsp"),
			       [("ice([0-9][0-9])d?\.((dll)|(pdb))", soVersion(version))])
	fileMatchAllAndReplace(os.path.join(ice_home, "src", "IceBox", "icebox.dsp"),
			       [("icebox([0-9][0-9])d?\.((dll)|(pdb))", soVersion(version))])
	fileMatchAllAndReplace(os.path.join(ice_home, "src", "IceGrid", "icegrid.dsp"),
			       [("icegrid([0-9][0-9])d?\.((dll)|(pdb))", soVersion(version))])
	fileMatchAllAndReplace(os.path.join(ice_home, "src", "IcePatch2", "icepatch2.dsp"),
			       [("icepatch2([0-9][0-9])d?\.((dll)|(pdb))", soVersion(version))])
	fileMatchAllAndReplace(os.path.join(ice_home, "src", "IceSSL", "icessl.dsp"),
			       [("icessl([0-9][0-9])d?\.((dll)|(pdb))", soVersion(version))])
	fileMatchAllAndReplace(os.path.join(ice_home, "src", "IceStorm", "icestorm.dsp"),
			       [("icestorm([0-9][0-9])d?\.((dll)|(pdb))", soVersion(version))])
	fileMatchAllAndReplace(os.path.join(ice_home, "src", "IceStorm", "icestormS.dsp"),
			       [("icestormservice([0-9][0-9])d?\.((dll)|(pdb))", soVersion(version))])
	fileMatchAllAndReplace(os.path.join(ice_home, "src", "IceUtil", "iceutil.dsp"),
			       [("iceutil([0-9][0-9])d?\.((dll)|(pdb))", soVersion(version))])
	fileMatchAllAndReplace(os.path.join(ice_home, "src", "IceXML", "icexml.dsp"),
			       [("icexml([0-9][0-9])d?\.((dll)|(pdb))", soVersion(version))])
	fileMatchAllAndReplace(os.path.join(ice_home, "src", "Slice", "slice.dsp"),
			       [("slice([0-9][0-9])d?\.((dll)|(pdb))", soVersion(version))])

    #
    # Fix version in IceJ sources
    #
    icej_home = findSourceTree("icej", os.path.join("src", "IceUtil", "Version.java"))
    if icej_home:
	fileMatchAndReplace(os.path.join(icej_home, "src", "IceUtil", "Version.java"),
			    [("ICE_STRING_VERSION = \"([0-9]*\.[0-9]*\.[0-9]*)\"", version), \
			     ("ICE_INT_VERSION = ([0-9]*)", intVersion(version))])

    #
    # Fix version in IceCS sources
    #
    icecs_home = findSourceTree("icecs", os.path.join("src", "Ice", "AssemblyInfo.cs"))
    if icecs_home:
	for f in find(icecs_home, "AssemblyInfo.cs"):
	    if f.find("generate") < 0 and f.find("ConsoleApplication") < 0:
		fileMatchAndReplace(f, [("AssemblyVersion\(\"([0-9]*\.[0-9]*\.[0-9]*)\"\)", version)])

	fileMatchAndReplace(os.path.join(icecs_home, "config", "Make.rules.cs"),
			    [("VERSION[\t\s]*= ([0-9]*\.[0-9]*\.[0-9]*)", version)])

	for f in find(icecs_home, "*.pc"):
	    fileMatchAndReplace(f, [("[\t\s]*version[\t\s]*=[\t\s]*([0-9]*\.[0-9]*\.[0-9]*)", version)])

    #
    # Fix version in IcePHP
    #
    icephp_home = findSourceTree("icephp", os.path.join("src", "ice", "php_ice.h"))
    if icephp_home:
	fileMatchAndReplace(os.path.join(icephp_home, "src", "ice", "php_ice.h"),
			    [("ICEPHP_STRING_VERSION \"([0-9]*\.[0-9]*\.[0-9]*)\"", version), \
			     ("ICEPHP_INT_VERSION ([0-9]*)", intVersion(version))])
	
    #
    # Fix version in IcePy
    #
    icephp_home = findSourceTree("icepy", os.path.join("modules", "IcePy", "Config.h"))
    if icephp_home:
	fileMatchAndReplace(os.path.join(icephp_home, "config", "Make.rules"),
			    [("^VERSION[ \t]+=[^\d]*([\d\.]+)", version),
			     ("^SOVERSION[\t\s]*= ([0-9]*)", soVersion(version))])

    sys.exit(0)

#
# Fix version in Ice-E sources
#
icee_home = findSourceTree("icee", os.path.join("include", "IceE", "Config.h"))
if icee_home:
    fileMatchAndReplace(os.path.join(icee_home, "include", "IceE", "Config.h"),
                        [("ICEE_STRING_VERSION \"([0-9]*\.[0-9]*\.[0-9]*)\"", version), \
                         ("ICEE_INT_VERSION ([0-9]*)", intVersion(version))])

    fileMatchAndReplace(os.path.join(icee_home, "config", "Make.rules"),
                        [("VERSION[\t\s]*= ([0-9]*\.[0-9]*\.[0-9]*)", version),
                         ("SOVERSION[\t\s]*= ([0-9]*)", soVersion(version))])
    
    fileMatchAllAndReplace(os.path.join(icee_home, "src", "IceE", "ice.dsp"),
                           [("icee([0-9][0-9])d?\.((dll)|(pdb))", soVersion(version))])
    fileMatchAllAndReplace(os.path.join(icee_home, "src", "IceEC", "icec.dsp"),
                           [("iceec([0-9][0-9])d?\.((dll)|(pdb))", soVersion(version))])
    fileMatchAllAndReplace(os.path.join(icee_home, "test", "Common", "testCommon.dsp"),
                           [("testCommon([0-9][0-9])d?\.((dll)|(pdb))", soVersion(version))])

#
# Fix version in IceJ sources
#
iceje_home = findSourceTree("iceje", os.path.join("src", "IceUtil", "Version.java"))
if iceje_home:
    fileMatchAndReplace(os.path.join(iceje_home, "src", "IceUtil", "Version.java"),
                        [("ICEE_STRING_VERSION = \"([0-9]*\.[0-9]*\.[0-9]*)\"", version), \
                         ("ICEE_INT_VERSION = ([0-9]*)", intVersion(version))])

sys.exit(0)
