#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, shutil, fnmatch, re, glob, tempfile

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


def createGitIgnore(filename, gitIgnoreFiles):
    file = open(filename, "r")
    lines = file.readlines()
    cwd = os.getcwd()

    newLines = [ ]
    ignore = ["*.o", "*.bak", "core"]

    for x in lines:
        x = x.strip()
        if not x.startswith("rm -f"):
            continue
        
        x = x.replace("rm -f", "", 1)
        if len(x) == 0:
            continue

        files = x.split()
        for f in files:
            if f in ignore:
                continue
            k = os.path.join(cwd, os.path.dirname(f), ".gitignore")
            k = os.path.normpath(k)
            if not gitIgnoreFiles.has_key(k):
                gitIgnoreFiles[k] = [ ]
            gitIgnoreFiles[k].append(os.path.basename(f) + "\n")

    file.close()

#
# Find where the root of the tree is.
#
for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "makegitignore.py")):
        break
else:
    print("cannot find top-level directory")
    sys.exit(1)

makefiles = find(toplevel, "Makefile")
cwd = os.getcwd()
gitIgnoreFiles = { }
for i in makefiles:
    os.chdir(os.path.dirname(i))
    if not os.system('grep -q TARGETS Makefile'):
        try:
            os.system("make -n clean > .tmp-gitignore")
            createGitIgnore(".tmp-gitignore", gitIgnoreFiles)
            os.remove(".tmp-gitignore")
        except:
            os.remove(".tmp-gitignore")
            raise
    os.chdir(cwd)
    
os.chdir(cwd)

for (path, files) in gitIgnoreFiles.iteritems():
    gitIgnore = open(path, "w")
    gitIgnore.writelines(files)
    gitIgnore.close()

