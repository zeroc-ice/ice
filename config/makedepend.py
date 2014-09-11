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

subincludedir = top_srcdir + "/include"
subcppincludedir = top_srcdir + "/../cpp/include"

try:
    opts, args = getopt.getopt(sys.argv[1:], "n", ["nmake"])
except getopt.GetoptError:
    raise RuntimeError("invalid arguments")

prefix = None
if len(args) > 0:
    prefix = args[0]

nmake = False
for o, a in opts:
    if o in ("-n", "--nmake"):
        nmake = True

depend = None
if not nmake:
    depend = open(".depend", "a")
dependmak = open(".depend.mak", "a")

lang = None
for line in fileinput.input("-"):
    line = line.strip()

    if commentre.search(line, 0):
        continue;

    if len(line) == 0:
        continue

    line = string.replace(line, ".o:", "$(OBJEXT):")

    if(previous):
        line = previous + " " + line

    if(line[-1] == "\\"):
        previous = line[:-2]
        continue
    else:
        previous = ""

    i = 0
    for s in line.split():
        if(s[0] == "/"):
            continue

        if i == 0 and s.endswith(".h") and prefix != None:
            if depend:
                print >>depend, prefix + "/" + s,
            print >>dependmak, prefix + "\\" + s,
            i += 1
            continue

        if s.endswith(".cs:"):
            lang = "cs"
            s = "generated/" + s
            if depend:
                print >>depend, s,
            print >>dependmak, s,
            continue

        if s.endswith(".cpp:"):
            lang = "cpp"

        if s.endswith(".rb:") and prefix != None:
            s = prefix + "/" + s
            if depend:
                print >>depend, s,
            print >>dependmak, s,
            continue

        if s.endswith(".php:"):
            lang = "php"
            if prefix != None:
                s = prefix + "/" + s
                if depend:
                    print >>depend, s,
                print >>dependmak, s,
                continue

        if s.startswith(subincludedir):
            s = "$(includedir)" + s[len(subincludedir):]
            if depend:
                print >>depend, s,
            print >>dependmak, '"' + s + '"',
            continue

        if s.startswith(subcppincludedir):
            s = "$(ice_cpp_dir)/include" + s[len(subcppincludedir):]
            if depend:
                print >>depend, s,
            print >>dependmak, '"' + s + '"',
            continue

        idx = s.find("./slice")
        if idx >= 0:
            s = "$(slicedir)" + s[idx + 7:]
            if depend:
                print >>depend, s,
            print >>dependmak, '"' + s + '"',
            continue

        if depend:
            print >>depend, s,
        print >>dependmak, s,

    if lang == "cpp":
        if depend:
            print >>depend, "$(SLICE2CPP) $(SLICEPARSERLIB)"
        print >>dependmak, "\"$(SLICE2CPP)\" \"$(SLICEPARSERLIB)\""
    elif lang == "cs":
        if depend:
            print >>depend, "$(SLICE2CS) $(SLICEPARSERLIB)"
        print >>dependmak, "\"$(SLICE2CS)\" \"$(SLICEPARSERLIB)\""
    elif lang == "php":
        if depend:
            print >>depend, "$(SLICE2PHP) $(SLICEPARSERLIB)"
        print >>dependmak, "\"$(SLICE2PHP)\" \"$(SLICEPARSERLIB)\""
    else:
        if depend:
            print >>depend
        print >>dependmak

if depend:
    depend.close()
dependmak.close()
