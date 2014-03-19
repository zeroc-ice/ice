#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, getopt
sys.path.append(os.path.join(os.path.dirname(__file__), "..", "lib"))
import FixUtil

def usage():
    print "Usage: " + sys.argv[0] + " version"
    print
    print "Options:"
    print "-h, --help        Show this message."
    print

if len(sys.argv) < 2:
    usage()
    sys.exit(0)

try:
    opts, args = getopt.getopt(sys.argv[1:], "h", ["help"])
except getopt.GetoptError:
    usage()
    sys.exit(1)
for o, a in opts:
    if o in ("-h", "--help"):
        usage()
        sys.exit(0)
if len(args) != 1:
    usage()
    sys.exit(1)

version = args[0]
ice_dir = os.path.normpath(os.path.join(os.path.dirname(__file__), "..", ".."))

FixUtil.checkVersion(version)

# Note newVersion and version might be different for beta versions: the patch version will be equal to 51.
newVersion =  FixUtil.majorVersion(version) + "." + FixUtil.minorVersion(version) + "." + FixUtil.patchVersion(version)

#
# Common build files
#
FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "config", "Make.common.rules"),
                            [("VERSION_MAJOR[\t\s]*= ([0-9]*)", FixUtil.majorVersion(version)),
                             ("VERSION_MINOR[\t\s]*= ([0-9]*b?)", FixUtil.minorVersion(version) +
                              FixUtil.betaVersion(version)),
                             ("SHORT_VERSION[\t\s]*= ([0-9]*\.[0-9]*)", FixUtil.shortVersion(version)),
                             ("VERSION_PATCH[\t\s]*= ([0-9]*)", FixUtil.patchVersion(version)),
                             ("VERSION[\t\s]*= " + FixUtil.vpatMatch, version),
                             ("SOVERSION[\t\s]*= ([0-9]+b?)", FixUtil.soVersion(version))])

FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "config", "Make.common.rules.mak"),
                            [("^VERSION[\t\s]*= " + FixUtil.vpatMatch, version),
                             ("INTVERSION[\t\s]*= " + FixUtil.vpatMatch, newVersion),
                             ("SHORT_VERSION[\t\s]*= ([0-9]*\.[0-9]*)", FixUtil.shortVersion(version)),
                             ("PATCH_VERSION[\t\s]*= ([0-9]*)", FixUtil.patchVersion(version)),
                             ("SOVERSION[\t\s]*= ([0-9]+b?)", FixUtil.soVersion(version))])

#
# Distribution files
#
FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "rpm", "ice.spec"),
                            [("Version: " + FixUtil.vpatMatch, version),
                             ("%define soversion ([0-9]+b?)", FixUtil.soVersion(version)),
                             ("%define dotnetversion ([0-9]*\.[0-9]*\.[0-9]*)", newVersion),
                             ("%define dotnetpolicyversion ([0-9]*\.[0-9]*)",
                              FixUtil.majorVersion(version) + "." + FixUtil.minorVersion(version))])

FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "common", "build.properties"),
                            [("ice\.version[\t\s]*= " + FixUtil.vpatMatch, version)])

FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "common", "README.DEMOS.txt"),
                            [("Ice-" + FixUtil.vpatMatch, version)])

FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "windows", "docs", "thirdparty", "README.txt"),
                            [("Ice " + FixUtil.vpatMatch, version)])

for f in FixUtil.find("*.py"):
    FixUtil.fileMatchAndReplace(f, [("iceVersion[\t\s]*= '" + FixUtil.vpatMatch, version)], False)

#
# Demo config files and scripts
#
FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "demoscript", "IceStorm", "clock.py"),
                            [("IceStormService,([0-9]+b?)", FixUtil.soVersion(version))])

FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "demoscript", "Util.py"),
                            [("/opt/Ice-([0-9]+\.[0-9]+)", FixUtil.shortVersion(version))])

for f in FixUtil.find("config.icebox"):
    FixUtil.fileMatchAndReplace(f, [("IceStormService,([0-9]+b?)", FixUtil.soVersion(version))])

for f in FixUtil.find("expect.py"):
    FixUtil.fileMatchAndReplace(f, [("IceStormService,([0-9]+b?)", FixUtil.soVersion(version))])

for f in FixUtil.find("config*"):
    FixUtil.fileMatchAndReplace(f, [("Version=*([0-9]*\.[0-9]*\.[0-9]*).0", newVersion)], False)

#
# C++ specfic files
#
ice_home = os.path.join(ice_dir, "cpp")
FixUtil.fileMatchAndReplace(os.path.join(ice_home, "include", "IceUtil", "Config.h"),
                    [("ICE_STRING_VERSION \"" + FixUtil.vpatMatch + "\"", version), \
                     ("ICE_INT_VERSION ([0-9]*)", FixUtil.intVersion(version))])

FixUtil.fileMatchAndReplace(os.path.join(ice_home, "src", "ca", "iceca"),
                    [("Ice-" + FixUtil.vpatMatch, version)])

FixUtil.fileMatchAndReplace(os.path.join(ice_home, "doc", "swish", "swish.conf"),
                    [("doc/Ice-" + FixUtil.vpatMatch, version)])

FixUtil.fileMatchAndReplace(os.path.join(ice_home, "doc", "swish", "swishcgi.conf"),
                    [("ice_version[\t\s]*= '" + FixUtil.vpatMatch, version)])

FixUtil.fileMatchAndReplace(os.path.join(ice_home, "demo", "IceStorm", "replicated2", "config.ib1"),
                    [("IceStormService,([0-9]+b?)", FixUtil.soVersion(version))])
FixUtil.fileMatchAndReplace(os.path.join(ice_home, "demo", "IceStorm", "replicated2", "config.ib2"),
                    [("IceStormService,([0-9]+b?)", FixUtil.soVersion(version))])
FixUtil.fileMatchAndReplace(os.path.join(ice_home, "demo", "IceStorm", "replicated2", "config.ib3"),
                    [("IceStormService,([0-9]+b?)", FixUtil.soVersion(version))])
    
FixUtil.fileMatchAndReplace(os.path.join(ice_home, "demo", "IceStorm", "replicated", "application.xml"),
                    [("IceStormService,([0-9]+b?)", FixUtil.soVersion(version))])

FixUtil.fileMatchAndReplace(os.path.join(ice_home, "test", "IceStorm", "repgrid", "application.xml"),
                    [("IceStormService,([0-9]+b?)", FixUtil.soVersion(version))])

FixUtil.fileMatchAndReplace(os.path.join(ice_home, "config", "templates.xml"),
                    [("IceStormService,([0-9]+b?)", FixUtil.soVersion(version))])

FixUtil.fileMatchAndReplace(os.path.join(ice_home, "demo", "Ice", "plugin", "Ice.plugin.hello.vcxproj"),
                    [("helloplugin([0-9]+b?)", FixUtil.soVersion(version))])

FixUtil.fileMatchAndReplace(os.path.join(ice_home, "demo", "Ice", "plugin", "Ice.plugin.logger.vcxproj"),
                    [("loggerplugin([0-9]+b?)", FixUtil.soVersion(version))])

for f in FixUtil.find("*.vcxproj"):
    FixUtil.fileMatchAndReplace(f, [("IceWinRT\\\\([0-9]+\.[0-9]+)@", FixUtil.shortVersion(version)),
                                    ("IceWinRT, Version=([0-9]+\.[0-9]+)", FixUtil.shortVersion(version))])

for f in FixUtil.find("*.rc"):
    FixUtil.fileMatchAndReplace(f, 
                                [("\"FileVersion\", \"" + FixUtil.vpatMatch, version),
                                 ("\"ProductVersion\", \"" + FixUtil.vpatMatch, version),
                                 ("INTERNALNAME \"[^0-9]*2?([0-9][0-9]b?)d?", FixUtil.soVersion(version)),
                                 ("ORIGINALFILENAME \"[^0-9]*2?([0-9][0-9]b?)d?\.dll", FixUtil.soVersion(version)),
                                 ("FILEVERSION ([0-9]+,[0-9]+,[0-9]+)", FixUtil.commaVersion(version)),
                                 ("PRODUCTVERSION ([0-9]+,[0-9]+,[0-9]+)", FixUtil.commaVersion(version))])
    
FixUtil.fileMatchAndReplace(os.path.join(ice_home, "test", "Ice", "background", ".gitignore"),
                    [("libTestTransport.so." + FixUtil.vpatMatch, version)])
FixUtil.fileMatchAndReplace(os.path.join(ice_home, "test", "Ice", "background", ".gitignore"),
                    [("libTestTransport.so.([0-9][0-9]b?)", FixUtil.soVersion(version))])

#
# Java specific files
#
icej_home = os.path.join(ice_dir, "java")
FixUtil.fileMatchAndReplace(os.path.join(icej_home, "config", "build.properties"),
                            [("ice\.version[\t\s]*= " + FixUtil.vpatMatch, version)])

FixUtil.fileMatchAndReplace(os.path.join(icej_home, "src", "IceUtil", "Version.java"),
                            [("ICE_STRING_VERSION = \"" + FixUtil.vpatMatch +"\"", version),
                             ("ICE_INT_VERSION = ([0-9]*)", FixUtil.intVersion(version))])
                            
FixUtil.fileMatchAndReplace(os.path.join(icej_home, "src", "Ice", "Util.java"),
                            [("return \"" + FixUtil.vpatMatch +"\".*A=major", version),
                             ("return ([0-9]*).*AA=major", FixUtil.intVersion(version))])
FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "java", "bin", "icegridgui.deb"),
                            [("IceGridGUI-" + FixUtil.vpatMatch, version)])

#
# C# specific files
#
icecs_home = os.path.join(ice_dir, "cs")

FixUtil.fileMatchAndReplace(os.path.join(icecs_home, "src", "IceBox", "Makefile.mak"),
                      [("codeBase version=\"" + FixUtil.vpatMatch + "\.0\"", newVersion)])


for f in FixUtil.find("AssemblyInfo*.cs"):
    if f.find("generate") < 0 and f.find("ConsoleApplication") < 0:
        FixUtil.fileMatchAndReplace(f, [("AssemblyVersion\(\"" + FixUtil.vpatMatch + "\"", newVersion),
                                        ("AssemblyFileVersion\(\"" + FixUtil.vpatMatch + "\"", newVersion)])

for f in FixUtil.find("*.pc"):
    FixUtil.fileMatchAndReplace(f, [("[\t\s]*version[\t\s]*=[\t\s]* " + FixUtil.vpatMatch, newVersion)], False)

FixUtil.fileMatchAndReplace(os.path.join(icecs_home, "src", "Ice", "Util.cs"),
                            [("return \"" + FixUtil.vpatMatch +"\".*A=major", version),
                             ("return ([0-9]*).*AA=major", FixUtil.intVersion(version))])
                            
#
# Fix C# and VB projects
#
for f in FixUtil.find(["*.csproj", "*.vbproj"]):
    for c in ["Ice", "IceBox", "IceGrid", "IcePatch2", "IceSSL", "IceStorm", "Glacier2"]:
        FixUtil.fileMatchAndReplace(f, [( c + ", Version=" + FixUtil.vpatMatch, newVersion)], False)

# Release notes
FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "RELEASE_NOTES"), [("Ice\+" + FixUtil.vpatMatch, version)])

# Eclipse plug-in
FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "eclipse", "java", "Slice2javaPlugin", "src",
                                         "com", "zeroc", "slice2javaplugin", "preferences", "messages.properties"),
                            [("IceStringVersion=" + FixUtil.vpatMatch, version)])

# VS addin Ice.props
FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "vsaddin", "config", "Ice.props"),
                            [("Ice\s" + FixUtil.vpatMatch, version)])
