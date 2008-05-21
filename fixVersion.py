#!/usr/bin/env python

import os, sys, shutil, fnmatch, re, glob, getopt

#
# version pattern
#
vpatCheck = "[0-9]+\.[0-9]+(\.[0-9]+|b[0-9]*)$"
vpatParse = "([0-9]+)\.([0-9]+)(\.[0-9]+|b[0-9]*)"
vpatMatch = "([0-9]+\.[0-9]+(\.[0-9]+|b[0-9]*))"

#
# Program usage.
#
def usage():
    print "Usage: " + sys.argv[0] + " [-e | -s] version"
    print
    print "Options:"
    print "-e                Fix version for Ice-E instead of Ice."
    print "-s                Fix version for IceSL instead of Ice."
    print "-h, --help        Show this message."
    print


def commaVersion(version):
    major = majorVersion(version)
    minor = minorVersion(version)
    patch = patchVersion(version)
    return ("%s,%s,%s" % (major, minor, patch))

def intVersion(version):
    r = re.search(vpatParse, version)
    major = int(r.group(1))
    minor = int(r.group(2))
    gr3 = r.group(3)
    patch = -1
    if gr3.startswith("."):
        patch = int(gr3[1:])
    else:
        if len(gr3) > 1:
            patch = 50 + int(gr3[1:])
        else:
            patch = 51
    return ("%2d%02d%02d" % (major, minor, patch)).strip()        

def betaVersion(version):
    r = re.search(vpatParse, version)
    if r.group(3).startswith("b"):
        return "b"
    else:
        return ""

def soVersion(version):
    r = re.search(vpatParse, version)
    major = int(r.group(1))
    minor = int(r.group(2))
    v = ("%d%d" % (major, minor)).strip()
    if r.group(3).startswith("b"):
        return v + "b"
    else:
        return v

def majorVersion(version):
    r = re.search(vpatParse, version)
    major = int(r.group(1))
    return ("%d" % (major)).strip()

def minorVersion(version):
    r = re.search(vpatParse, version)
    minor = int(r.group(2))
    return ("%d" % (minor)).strip()

def shortVersion(version):
    r = re.search(vpatParse, version)
    major = int(r.group(1))
    minor = int(r.group(2))
    return ("%d.%d" % (major, minor)).strip()

def patchVersion(version):
    r = re.search(vpatParse, version)
    
    gr3 = r.group(3)
    patch = -1
    if gr3.startswith("."):
        patch = int(gr3[1:])
    else:
        if len(gr3) > 1:
            patch = 50 + int(gr3[1:])
        else:
            patch = 51

    return ("%d" % (patch)).strip()

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


#
# Replace a string matched by the first group of regular expression.
#
# For example: the regular expression "ICE_STRING_VERSION \"([0-9]*\.[0-9]*\.[0-9]*)\""
# will match the string version in "ICE_STRING_VERSION "2.1.0"" and will replace it with
# the given version.
#
def fileMatchAndReplace(filename, matchAndReplaceExps, warn=True):

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
    elif warn:
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

patchIce = True
patchIceE = False
patchIceSL = False
try:
    opts, args = getopt.getopt(sys.argv[1:], "hes", ["help"])
except getopt.GetoptError:
    usage()
    sys.exit(1)
for o, a in opts:
    if o in ("-h", "--help"):
        usage()
        sys.exit(0)
    if o in ("-e"):
        patchIceE = True
        patchIce = False
    if o in ("-s"):
        patchIceSL = True
        patchIce = False
if len(args) != 1:
    usage()
    sys.exit(1)

version = args[0]
ice_dir = os.path.normpath(os.path.join(os.path.dirname(__file__)))

if not re.match(vpatCheck, version):
    print "invalid version number: " + version + " (it should have the form 3.2.1 or 3.2b or 3.2b2)"
    sys.exit(0)

if patchIce:
    fileMatchAndReplace(os.path.join(ice_dir, "config", "Make.common.rules"),
                        [("VERSION_MAJOR[\t\s]*= ([0-9]*)", majorVersion(version)),
                         ("VERSION_MINOR[\t\s]*= ([0-9]*b?)", minorVersion(version) + betaVersion(version)),
                         ("SHORT_VERSION[\t\s]*= ([0-9]*\.[0-9]*)", shortVersion(version)),
                         ("VERSION[\t\s]*= " + vpatMatch, version),
                         ("SOVERSION[\t\s]*= ([0-9]+b?)", soVersion(version))])
    
    fileMatchAndReplace(os.path.join(ice_dir, "config", "Make.common.rules.mak"),
                        [("^VERSION[\t\s]*= " + vpatMatch, version),
                         ("INTVERSION[\t\s]*= " + vpatMatch, majorVersion(version) + "." + minorVersion(version) + \
                                                             "." + patchVersion(version)),
                         ("SHORT_VERSION[\t\s]*= ([0-9]*\.[0-9]*)", shortVersion(version)),
                         ("SOVERSION[\t\s]*= ([0-9]+b?)", soVersion(version))])

    fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "rpm", "ice.spec"),
                        [("Version: " + vpatMatch, version)])
    fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "rpm", "ice.spec"),
                        [("%define soversion ([0-9]+b?)", soVersion(version))])
    fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "rpm", "ice.spec"),
                        [("%define dotnetversion ([0-9]*\.[0-9]*\.[0-9]*)",
                          majorVersion(version) + "." + minorVersion(version) + "." + patchVersion(version))])


    #
    # Fix version in C++ sources
    #
    ice_home = os.path.join(ice_dir, "cpp")
    if ice_home:
        fileMatchAndReplace(os.path.join(ice_home, "include", "IceUtil", "Config.h"),
                            [("ICE_STRING_VERSION \"" + vpatMatch + "\"", version), \
                             ("ICE_INT_VERSION ([0-9]*)", intVersion(version))])

        fileMatchAndReplace(os.path.join(ice_home, "src", "ca", "iceca"),
                            [("Ice-" + vpatMatch, version)])
        os.system("chmod 755 " + os.path.join(ice_home, "src", "ca", "iceca"))        

        fileMatchAndReplace(os.path.join(ice_home, "doc", "swish", "swish.conf"),
                            [("doc/Ice-" + vpatMatch, version)])

        fileMatchAndReplace(os.path.join(ice_home, "demo", "IceStorm", "clock", "config.icebox"),
                            [("IceStormService,([0-9]+b?)", soVersion(version))])

        fileMatchAndReplace(os.path.join(ice_home, "demo", "IceStorm", "counter", "config.icebox"),
                            [("IceStormService,([0-9]+b?)", soVersion(version))])

        fileMatchAndReplace(os.path.join(ice_home, "demo", "IceStorm", "replicated2", "config.ib1"),
                            [("IceStormService,([0-9]+b?)", soVersion(version))])
        fileMatchAndReplace(os.path.join(ice_home, "demo", "IceStorm", "replicated2", "config.ib2"),
                            [("IceStormService,([0-9]+b?)", soVersion(version))])
        fileMatchAndReplace(os.path.join(ice_home, "demo", "IceStorm", "replicated2", "config.ib3"),
                            [("IceStormService,([0-9]+b?)", soVersion(version))])
        
        fileMatchAndReplace(os.path.join(ice_home, "demo", "IceStorm", "replicated", "application.xml"),
                            [("IceStormService,([0-9]+b?)", soVersion(version))])

        fileMatchAndReplace(os.path.join(ice_home, "config", "templates.xml"),
                            [("IceStormService,([0-9]+b?)", soVersion(version))])

        fileMatchAndReplace(os.path.join(ice_home, "test", "IceStorm", "repgrid", "application.xml"),
                            [("IceStormService,([0-9]+b?)", soVersion(version))])

        for f in find(os.path.join(ice_home, "src"), "*.rc"):
            fileMatchAndReplace(f, [("\"FileVersion\", \"" + vpatMatch, version), \
                                    ("\"ProductVersion\", \"" + vpatMatch, version), \
                                    ("INTERNALNAME \"[^0-9]*2?([0-9][0-9]b?)d?", soVersion(version)), \
                                    ("ORIGINALFILENAME \"[^0-9]*2?([0-9][0-9]b?)d?\.dll", soVersion(version)), \
                                    ("FILEVERSION ([0-9]+,[0-9]+,[0-9]+)", commaVersion(version)), \
                                    ("PRODUCTVERSION ([0-9]+,[0-9]+,[0-9]+)", commaVersion(version))])
        
    #
    # Fix version in Java sources
    #
    icej_home = os.path.join(ice_dir, "java")
    if icej_home:
        fileMatchAndReplace(os.path.join(icej_home, "config", "build.properties"),
                            [("ice\.version[\t\s]*= " + vpatMatch, version)])
         
        fileMatchAndReplace(os.path.join(icej_home, "src", "IceUtil", "Version.java"),
                            [("ICE_STRING_VERSION = \"" + vpatMatch +"\"", version), \
                             ("ICE_INT_VERSION = ([0-9]*)", intVersion(version))])

        fileMatchAndReplace(os.path.join(icej_home, "src", "Ice", "Util.java"),
                            [("return \"" + vpatMatch +"\".*A=major", version), \
                             ("return ([0-9]*).*AA=major", intVersion(version))])

        fileMatchAndReplace(os.path.join(icej_home, "demo", "IceStorm", "clock", "config.icebox"),
                            [("IceStormService,([0-9]+b?)", soVersion(version))])

    #
    # Fix version in C# sources
    #
    icecs_home = os.path.join(ice_dir, "cs")
    if icecs_home:
        for f in find(icecs_home, "AssemblyInfo.cs"):
            if f.find("generate") < 0 and f.find("ConsoleApplication") < 0:
                fileMatchAndReplace(f, [("AssemblyVersion\(\"" + vpatMatch + "\"",
                                         majorVersion(version) + "." + minorVersion(version) + "." + \
                                         patchVersion(version))])

        fileMatchAndReplace(os.path.join(icecs_home, "demo", "IceStorm", "clock", "config.icebox"),
                            [("IceStormService,([0-9]+b?)", soVersion(version))])

        for f in find(icecs_home, "*.pc"):
            print "matching " + f
            fileMatchAndReplace(f, [("[\t\s]*version[\t\s]*=[\t\s]* " + vpatMatch, majorVersion(version) + "." + \
                                minorVersion(version) + "." + patchVersion(version))])

        for f in find(icecs_home, "config*"):
            print "matching " + f
            fileMatchAndReplace(f, 
                                [("Version=*([0-9]*\.[0-9]*\.[0-9]*).0",
                                 majorVersion(version) + "." + minorVersion(version) + "." + patchVersion(version))],
                                False) # Disable warnings as many files might not have SSL configuration

        fileMatchAndReplace(os.path.join(icecs_home, "src", "Ice", "Util.cs"),
                            [("return \"" + vpatMatch +"\".*A=major", version), \
                             ("return ([0-9]*).*AA=major", intVersion(version))])


    #
    # Fix version in VB sources
    #
    icevb_home = os.path.join(ice_dir, "vb")
    if icevb_home:
        fileMatchAndReplace(os.path.join(icevb_home, "demo", "IceStorm", "clock", "config.icebox"),
                            [("IceStormService,([0-9]+b?)", soVersion(version))])

        for f in find(icevb_home, "config*"):
            print "matching " + f
            fileMatchAndReplace(f, 
                                [("Version=*([0-9]*\.[0-9]*\.[0-9]*).0",
                                 majorVersion(version) + "." + minorVersion(version) + "." + patchVersion(version))],
                                False) # Disable warnings as many files might not have SSL configuration

    #
    # Fix version in PHP sources
    #
    #icephp_home = os.path.join(ice_dir, "php")
    #if icephp_home:

    #
    # Fix version in IcePy
    #
    icepy_home = os.path.join(ice_dir, "py")
    if icepy_home:
        fileMatchAndReplace(os.path.join(icepy_home, "demo", "IceStorm", "clock", "config.icebox"),
                            [("IceStormService,([0-9]+b?)", soVersion(version))])

        fileMatchAndReplace(os.path.join(icepy_home, "demo", "Ice", "bidir", "Server.py"),
                            [("Ice-" + vpatMatch, version)])
        fileMatchAndReplace(os.path.join(icepy_home, "demo", "Ice", "bidir", "Client.py"),
                            [("Ice-" + vpatMatch, version)])

    #
    # Fix version in IceRuby
    #
    #icerb_home = os.path.join(ice_dir, "rb")
    #if icerb_home:

    sys.exit(0)

#
# Fix version in Ice-E sources
#
if patchIceE:
    icee_home = os.path.join(ice_dir, "cppe")
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
    iceje_home = os.path.join(ice_dir, "javae")
    if iceje_home:
        fileMatchAndReplace(os.path.join(iceje_home, "src", "IceUtil", "Version.java"),
                            [("ICEE_STRING_VERSION = \"([0-9]*\.[0-9]*\.[0-9]*)\"", version), \
                             ("ICEE_INT_VERSION = ([0-9]*)", intVersion(version))])

    sys.exit(0)

if patchIceSL:
    icecpp_home = os.path.join(ice_dir, "cpp")
    if icecpp_home:
        fileMatchAndReplace(os.path.join(icecpp_home, "src", "slice2sl", "Gen.h"),
                            [("ICESL_STRING_VERSION \"([0-9]*\.[0-9]*\.[0-9]*)\"", version)])

    icesl_home = os.path.join(ice_dir, "sl")
    if icesl_home:
        fileMatchAndReplace(os.path.join(icesl_home, "src", "Ice", "AssemblyInfo.cs"),
                            [("AssemblyVersion\(\"" + vpatMatch + "\"",
                             majorVersion(version) + "." + minorVersion(version) + "." + patchVersion(version))])

    sys.exit(0)
