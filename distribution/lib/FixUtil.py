#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, shutil, fnmatch, re, glob, getopt
from stat import *

def getTrackedFiles():
    files = []
    pipe = os.popen('git ls-files')
    for line in pipe.readlines():
        p = os.path.join('.', line.strip())
        files.append(p)
    return files

def fnmatchlist(x, l):
    for p in l:
        if fnmatch.fnmatch(x, p):
            return True
    return False
#
# Version patterns
#
vpatCheck = "[0-9]+\.[0-9]+(\.[0-9]+|b[0-9]*)$"
vpatParse = "([0-9]+)\.([0-9]+)(\.[0-9]+|b[0-9]*)"
vpatMatch = "([0-9]+\.[0-9]+(\.[0-9]+|b[0-9]*))"

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
def find(patt):
    if type(patt) != type([]):
        patt = [patt]
    result = [ ]
    for fullpath in getTrackedFiles():
        x = os.path.basename(fullpath)
        if os.path.isdir(fullpath):
            continue;
        for p in patt:
            if fnmatch.fnmatch(x, p):
                result.append(fullpath)
                break
    return result

#
# Replace a string matched by the first group of regular expression.
#
# For example: the regular expression "ICE_STRING_VERSION \"([0-9]*\.[0-9]*\.[0-9]*)\""
# will match the string version in "ICE_STRING_VERSION "2.1.0"" and will replace it with
# the given version.
#
def fileMatchAndReplace(filename, matchAndReplaceExps, verbose=True):

    if os.path.isdir(filename):
        return

    mode = os.stat(filename).st_mode
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
                if len(match.groups()) == 0:
                    line = oldLine.replace(match.group(), replace)
                else:
                    line = oldLine.replace(match.group(1), replace)
                #print oldLine + line
                updated |= oldLine != line
                break
        newConfigFile.write(line)

    newConfigFile.close()
    oldConfigFile.close()

    if updated:
        if verbose:
            print("updated " + filename)
        os.remove(filename)
        os.rename(filename + ".new", filename)
        os.chmod(filename, S_IMODE(mode))
    else:
        if verbose:
            print("warning: " + filename + " didn't contain any match")
        os.unlink(filename + ".new")

#
# Replace all occurences of a regular expression in a file
#
def fileMatchAllAndReplace(filename, matchAndReplaceExps, verbose=True):

    mode = os.stat(filename).st_mode
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
                if len(match.groups()) == 0:
                    line = oldLine.replace(match.group(), replace)
                else:
                    line = oldLine.replace(match.group(1), replace)
                updated |= oldLine != line
        newFile.write(line)

    newFile.close()
    oldFile.close()

    if updated:
        if verbose:
            print("updated " + filename)
        os.remove(filename)
        os.rename(filename + ".new", filename)
        os.chmod(filename, S_IMODE(mode))
    else:
        if verbose:
            print("warning: " + filename + " didn't contain any match")
        os.unlink(filename + ".new")

def checkVersion(version):
    if not re.match(vpatCheck, version):
        print("invalid version number: " + version + " (it should have the form 3.2.1 or 3.2b or 3.2b2)")
        sys.exit(0)

def fixLineEnd():
    files = getTrackedFiles()

    #
    # Filename suffixes that don't need to be checked.
    #
    ignoreSubfix = ["\.aip", "\.bmp", "\.exe", "\.gif", "\.jpg", "\.png", "\.rtf", "\.zip", \
                    "\.snk", "\.pfx", "\.class", "\.ico", "\.jks", "\.gz", "\.DS_Store", "\.chm", "\.utf16le", \
                    "\.utf16be", "\.utf32le", "\.utf32be", "\.dat", "\.bin", "\.patch", "patch\..*", "passwords",
                    "plugin.properties", ".jar", "\.cfg", "\.depend.mak"]

    #
    # File extensions that must use DOS-style line endings.
    #
    dosExtensions = [".mak", ".mak.cs", ".mak.php", ".mak.vb", ".txt", ".exe.config", ".csproj", ".vbproj", \
                     ".vcproj", ".vsproj", ".sln", ".vsdir", ".bat", ".rc", ".rc2", ".settings", ".AddIn", \
                     ".resx", ".bcc", ".msvc", ".dsp", ".dsw"]

    for filename in files:
        path = filename.lower()
        ignore = False
        for e in ignoreSubfix:
            regexp = re.compile(e.lower() + "$")
            if regexp.search(path):
                ignore = True
                #print "Ignoring file " + filename + " with extension " + e
                break

        if ignore:
            continue

        dos = False
        for e in dosExtensions:
            if path.endswith(e.lower()):
                dos = True
                break

        file = open(filename, "r")

        convert = False
        for line in file:
            if dos:
                if line.endswith("\n") and not line.endswith("\r\n"):
                    convert = True
                    break
            else:
                if line.endswith("\r\n"):
                    convert = True
                    break

        file.close()
        file = open(filename, "r")
        text = file.read()
        file.close()

        eol = None
        if len(text) > 0: # Ignore empty files
            if convert:
                if not text.endswith("\n"):
                    eol = "\n"
            else:
                if dos and not text.endswith("\r\n"):
                    eol = "\r\n"
                elif not dos and not text.endswith("\n"):
                    eol = "\n"

        if eol:
            file = open(filename, "w")
            file.write(text + eol)
            file.close()
            print("Added EOL to file " + filename)

        if convert:
            print("Converting " + filename)
            os.popen("dos2unix -U -q " + filename)
            if dos:
                os.popen("recode -f latin1..dos " + filename)
