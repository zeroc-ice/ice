#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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

if os.path.abspath(ice_dir) != os.getcwd():
    print "!!! WARNING - SCRIPT MUST BE RAN FROM THE TOP LEVEL ICE DIRECTORY !!!"
    sys.exit(1)

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

for f in FixUtil.find("*README*"):
    FixUtil.fileMatchAndReplace(f, [("Ice[ \-\+]" + FixUtil.vpatMatch, version)], False)

FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "common", "gradle.properties"),
                            [("iceVersion[\t\s]*= " + FixUtil.vpatMatch, version)])

FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "common", "gradle.properties.android"),
                            [("iceVersion[\t\s]*= " + FixUtil.vpatMatch, version)])

FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "rpm", "icegridregistry.conf"),
                            [("Ice-" + FixUtil.vpatMatch, version)])

for f in FixUtil.find("*.py"):
    FixUtil.fileMatchAndReplace(f, [("iceVersion[\t\s]*= ['\"]" + FixUtil.vpatMatch, version)], False)

FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "rpm", "RPM_README"),
                            [("ice-" + FixUtil.vpatMatch, version)], False)

FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "rpm", "RPM_README"),
                            [("Ice-rpmbuild-" + FixUtil.vpatMatch, version)], False)

for pkg in ["ice", "gradle-plugin", "glacier2", "freeze", "icebox", "icediscovery", "icegrid", "icestorm", "icepatch2"]:
    FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "deb", "debian", "libzeroc-ice-java.install"),
                            [(pkg + "[-/]" + FixUtil.vpatMatch, version)], False)
    FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "deb", "debian", "libzeroc-ice-java.links"),
                            [(pkg + "-" + FixUtil.vpatMatch, version)], False)

for f in ["zeroc-icegrid.install", "zeroc-ice-slice.install", "zeroc-ice-utils-java.install",
          "zeroc-icegrid.icegridregistry.conf"]:
    FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "deb", "debian", f),
                            [("Ice-" + FixUtil.vpatMatch, version)])

FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "deb", "debian", "rules"),
                            [("VERSION[\t\s]*= " + FixUtil.vpatMatch, version)])

jsonMatch = "([0-9]+\.[0-9]+\.[0-9]+(-beta.0)?)"
for t in ["package.json", "bower.json"]:
    for f in FixUtil.find(t):
        FixUtil.fileMatchAndReplace(f, [("\"version\":[\t\s]\"" + jsonMatch, FixUtil.jsonVersion(version))], False)
        FixUtil.fileMatchAndReplace(f, [("\"zeroc-slice2js\":[\t\s]\"\^" + jsonMatch, FixUtil.jsonVersion(version))], False)
        FixUtil.fileMatchAndReplace(f, [("\"gulp-zeroc-slice2js\":[\t\s]\"\^" + jsonMatch, FixUtil.jsonVersion(version))], False)
        FixUtil.fileMatchAndReplace(f, [("\"zeroc-icejs\":[\t\s]\"~" + jsonMatch, FixUtil.jsonVersion(version))], False)
        FixUtil.fileMatchAndReplace(f, [("\"zeroc-icejs\":[\t\s]\"\^" + jsonMatch, FixUtil.jsonVersion(version))], False)

mmver = FixUtil.majorVersion(version) + "." + FixUtil.minorVersion(version) + FixUtil.betaVersion(version)
mmverMatch = "([0-9]+\.[0-9b]+)"

FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "deb", "DEB_README"),
                                         [("ice" + mmverMatch, mmver)])

FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "deb", "DEB_README"),
                                         [("Ice/" + mmverMatch + "/", mmver)])

FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "deb", "DEB_README"),
                                         [("ice[0-9]+\.[0-9b]+-" + FixUtil.vpatMatch, version)])

FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "deb", "DEB_README"),
                            [("VERSION[\t\s]*= " + FixUtil.vpatMatch, version)])

FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "deb", "debian", "libzeroc-freeze3.6.links"),
                            [("freeze" + mmverMatch, mmver)])

FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "deb", "debian", "libzeroc-ice3.6.links"),
                            [("ice" + mmverMatch, mmver)])

FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "deb", "debian", "libzeroc-icestorm3.6.links"),
                            [("icestorm" + mmverMatch, mmver)])

nbmmver = FixUtil.majorVersion(version) + "." + FixUtil.minorVersion(version)
nbmmverMatch = "([0-9]+\.[0-9]+)"

for f in ["control", "rules"]:
    for pkg in ["ice", "freeze", "icestorm"]:
        FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "deb", "debian", f),
                                    [("zeroc-" + pkg + nbmmverMatch, nbmmver)])

FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "unix", "README.Darwin"),
                            [("Ice\+" + nbmmverMatch + "\+Manual", nbmmver)])

FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "unix", "README.Linux"),
                            [("Ice\+" + nbmmverMatch + "\+Manual", nbmmver)])

FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "src", "windows", "docs", "main", "README.txt"),
                            [("Ice\+" + nbmmverMatch + "\+Manual", nbmmver)])

debVersion = FixUtil.majorVersion(version) + "." + FixUtil.minorVersion(version) + "."
if FixUtil.patchVersion(version) != "51":
    debVersion += FixUtil.patchVersion(version)
else:
    debVersion += "0"

for f in ["makeubuntupackages.py", "makeubunturepo.py", "makejspackages.py"]:
    FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "bin", f),
                                [("mmVersion[\t\s]*= \"" + nbmmverMatch, nbmmver)])
    FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "distribution", "bin", f),
                                [("debVersion[\t\s]*= \"" + FixUtil.vpatMatch, debVersion)])


#
# Miscelaneous source distribution files
#
FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "CHANGES"), [("Ice\+" + FixUtil.vpatMatch, version)])

FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "README"), [("Ice[ \+]" + FixUtil.vpatMatch, version)])

for f in FixUtil.find("*.AddIn"):
    FixUtil.fileMatchAndReplace(f, [("Ice " + FixUtil.vpatMatch, version)])

#
# Demo config files and scripts
#
FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "demoscript", "IceStorm", "clock.py"),
                            [("IceStormService,([0-9]+b?)", FixUtil.soVersion(version))])

FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "demoscript", "Util.py"),
                            [("/opt/Ice-([0-9]+\.[0-9]+)", FixUtil.shortVersion(version))])

for f in FixUtil.find("config.icebox"):
    FixUtil.fileMatchAndReplace(f, [("IceStormService,([0-9]+b?)", FixUtil.soVersion(version))], False)

for f in FixUtil.find("expect.py"):
    FixUtil.fileMatchAndReplace(f, [("IceStormService,([0-9]+b?)", FixUtil.soVersion(version))], False)

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

FixUtil.fileMatchAndReplace(os.path.join(ice_home, "demo", "IceStorm", "clock", "config.icebox"),
                    [("IceStormService,([0-9]+b?)", FixUtil.soVersion(version))])

FixUtil.fileMatchAndReplace(os.path.join(ice_home, "demo", "IceStorm", "counter", "config.icebox"),
                    [("IceStormService,([0-9]+b?)", FixUtil.soVersion(version))])
FixUtil.fileMatchAndReplace(os.path.join(ice_home, "demo", "IceStorm", "counter", "expect.py"),
                    [("IceStormService,([0-9]+b?)", FixUtil.soVersion(version))])

FixUtil.fileMatchAndReplace(os.path.join(ice_home, "demo", "IceStorm", "replicated2", "config.ib1"),
                    [("IceStormService,([0-9]+b?)", FixUtil.soVersion(version))])
FixUtil.fileMatchAndReplace(os.path.join(ice_home, "demo", "IceStorm", "replicated2", "config.ib2"),
                    [("IceStormService,([0-9]+b?)", FixUtil.soVersion(version))])
FixUtil.fileMatchAndReplace(os.path.join(ice_home, "demo", "IceStorm", "replicated2", "config.ib3"),
                    [("IceStormService,([0-9]+b?)", FixUtil.soVersion(version))])
FixUtil.fileMatchAndReplace(os.path.join(ice_home, "demo", "IceStorm", "replicated2", "expect.py"),
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

FixUtil.fileMatchAndReplace(os.path.join(ice_home, "demo", "IceGrid", "customLoadBalancing", "IceGrid.customLoadBalancing.plugin.vcxproj"),
                    [("registryplugin([0-9]+b?)", FixUtil.soVersion(version))])

for f in FixUtil.find("*.vcxproj"):
    FixUtil.fileMatchAndReplace(f, [("Ice\\\\([0-9]+\.[0-9])", FixUtil.shortVersion(version)),
                                    ("Ice, Version=([0-9]+\.[0-9]+)", FixUtil.shortVersion(version))], False)

for f in FixUtil.find("*.rc"):
    FixUtil.fileMatchAndReplace(f,
                                [("\"FileVersion\", \"" + FixUtil.vpatMatch, version),
                                 ("\"ProductVersion\", \"" + FixUtil.vpatMatch, version),
                                 ("INTERNALNAME \"[^0-9]*2?([0-9][0-9]b?)d?", FixUtil.soVersion(version)),
                                 ("ORIGINALFILENAME \"[^0-9]*2?([0-9][0-9]b?)d?\.dll", FixUtil.soVersion(version)),
                                 ("FILEVERSION ([0-9]+,[0-9]+,[0-9]+)", FixUtil.commaVersion(version)),
                                 ("PRODUCTVERSION ([0-9]+,[0-9]+,[0-9]+)", FixUtil.commaVersion(version))], False)

#
# Java specific files
#
icej_home = os.path.join(ice_dir, "java")
FixUtil.fileMatchAndReplace(os.path.join(icej_home, "gradle.properties"),
                            [("iceVersion[\t\s]*= " + FixUtil.vpatMatch, version)])

FixUtil.fileMatchAndReplace(os.path.join(icej_home, "buildSrc", "build.gradle"),
                            [("version[\t\s]* '" + FixUtil.vpatMatch + "'", version)])

FixUtil.fileMatchAndReplace(os.path.join(icej_home, "src", "Ice", "src", "main", "java", "Ice", "Util.java"),
                            [("return \"" + FixUtil.vpatMatch +"\".*A=major", version),
                             ("return ([0-9]*).*AA=major", FixUtil.intVersion(version))])

FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "java", "bin", "icegridgui.deb"),
                            [("IceGridGUI-" + FixUtil.vpatMatch, version)])

FixUtil.fileMatchAndReplace(os.path.join(icej_home, "demo", "Database", "library", "README"),
                            [("ice-" + FixUtil.vpatMatch, version)])

#
# Android Files
#
FixUtil.fileMatchAndReplace(os.path.join(os.path.join(ice_dir, "android"), "gradle.properties"),
                            [("iceVersion[\t\s]*= " + FixUtil.vpatMatch, version)])

for f in FixUtil.find("*.iml"):
    FixUtil.fileMatchAndReplace(f,
                                [("version=\"" + FixUtil.vpatMatch + "\"", version),
                                 ("name=\"ice-" + FixUtil.vpatMatch + "\"", version),
                                 ("name=\"glacier2-" + FixUtil.vpatMatch + "\"", version)], False)
#
# JavaScript specific files
#
icejs_home = os.path.join(ice_dir, "js")

FixUtil.fileMatchAndReplace(os.path.join(icejs_home, "bin", "HttpServer.js"),
                            [("Ice-" + FixUtil.vpatMatch, version)])

FixUtil.fileMatchAndReplace(os.path.join(icejs_home, "package.json"),
                            [("\"version\": \"([^\"]+)\"", FixUtil.jsonVersion(version))])

FixUtil.fileMatchAndReplace(os.path.join(icejs_home, "bower.json"),
                            [("\"version\": \"([^\"]+)\"", FixUtil.jsonVersion(version))])

#
# C# specific files
#
icecs_home = os.path.join(ice_dir, "cs")

FixUtil.fileMatchAndReplace(os.path.join(icecs_home, "src", "IceBox", "Makefile.mak"),
                      [("codeBase version=\"" + FixUtil.vpatMatch + "\.0\"", newVersion)])


for f in FixUtil.find("AssemblyInfo*.cs"):
    if f.find("generate") < 0 and f.find("ConsoleApplication") < 0:
        FixUtil.fileMatchAndReplace(f, [("AssemblyVersion\(\"" + FixUtil.vpatMatch + "\"", newVersion),
                                        ("AssemblyFileVersion\(\"" + FixUtil.vpatMatch + "\"", newVersion)], False)

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

# Eclipse plug-in
FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "eclipse", "java", "Slice2javaPlugin", "src",
                                         "com", "zeroc", "slice2javaplugin", "preferences", "messages.properties"),
                            [("IceStringVersion=" + FixUtil.vpatMatch, version)])

# VS addin Ice.props
FixUtil.fileMatchAndReplace(os.path.join(ice_dir, "vsaddin", "config", "Ice.props"),
                            [("Ice\s" + FixUtil.vpatMatch, version)])


# man pages
for f in FixUtil.find(["*.1"]):
    FixUtil.fileMatchAndReplace(f, [("/Ice([0-9]+[0-9]+)/", FixUtil.majorVersion(version) + FixUtil.minorVersion(version))], False)

