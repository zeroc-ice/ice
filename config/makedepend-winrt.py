#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os, fileinput, re, string, getopt

previous = ""

commentre = re.compile("^#")

for top_srcdir in [".", "..", "../..", "../../..", "../../../.."]:
    top_srcdir = os.path.normpath(top_srcdir)
    if os.path.exists(os.path.join(top_srcdir, "..", "config", "makedepend.py")):
        break
else:
    raise RuntimeError("can't find top level source directory!")
    
def contains(s, words):
    i = -1
    l = 0
    for word in words:
        i = s.find(word) 
        if i >= 0:
            l = len(word)
            break
    return i, l

dependmak = open(".depend.mak", "a")

HDIR = "$(HDIR)"
CPPDIR =  ".."

if len(sys.argv) >= 2:
    HDIR = sys.argv[1]

if len(sys.argv) >= 3:
    CPPDIR = sys.argv[2]

includes = []
for line in fileinput.input("-"):
    line = line.strip()
    if not line:
        continue
    if(previous):
        line = previous + " " + line

    if(line[-1] == "\\"):
        previous = line[:-2]
        continue
    else:
        previous = ""
    line = line.replace("/", "\\")
    if line.startswith("Note: including file:"):
        line = line[len("Note: including file:"):].strip()
        (i, l) = contains(line, ["IceUtil\\", "Ice\\", "Glacier2\\", "Glacier2Lib\\", "IceStorm\\", "IceGrid\\", "IceGridLib\\", "IceStormLib\\"])
        if i >= 0:
            j = line.find("winrt\\")
            if j < 0:
                j = line.find("cpp\\src\\")
                if j >= 0 and i > j:
                    j = i + l
                    line = line[j:]
                    if not line in includes:
                        print('"..\\' + line + '"', end = " ", file = dependmak)
                        includes.append(line)
                    continue
            if j < 0:
                j = line.find("cpp\\include\\")
            if j >= 0:
                line = line[j:]
            line = line.replace("winrt\\", "")
            line = line.replace("cpp\\src\\", "..\\")
            line = line.replace("..\\..\\..\\include\\", "$(includedir)\\")
            line = line.replace("cpp\\include\\", "$(includedir)\\")
            line = line.strip()
            if not line in includes:
                print('"' + line + '"', end = " ", file = dependmak)
                includes.append(line)
    elif line.endswith(".cpp") and not line.endswith(".cpp:"):
        line = "$(ARCH)\\$(CONFIG)\\" + line.replace(".cpp", "$(OBJEXT):") + " " + sys.argv[1]
        print(line, end = " ", file = dependmak)
    elif line.find("slice\\") >= 0:
        for s in line.split():
            i = s.find("slice\\")
            if i >= 0:
                s = "$(slicedir)\\" + s[i + len("slice\\"):]
                print('"' + s + '"', end = " ", file = dependmak)
            elif s.endswith(".cpp:"):
                print(CPPDIR + "\\" + s, end = " ", file = dependmak)
            elif s.endswith(".h"):
                print(HDIR + "\\" + s, end = " ", file = dependmak)
        print("\"$(SLICE2CPP)\"", file = dependmak)

print("", file=dependmak)
dependmak.close()
