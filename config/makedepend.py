#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
    raise "can't find top level source directory!"

subincludedir = top_srcdir + "/include"
subcppincludedir = top_srcdir + "/../cpp/include"

prefix = None
if len(sys.argv) == 2:
    prefix = sys.argv[1]
    del sys.argv[1]

lang = None

for line in fileinput.input():
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

    for s in line.split():
        if(s[0] == "/"):
            continue

        if s.endswith(".cs:"):
            lang = "cs"
            s = "generated/" + s
            print s,
            continue

        if s.endswith(".rb:") and not prefix == None:
            s = prefix + "/" + s
            print s,
            continue

        if s.endswith(".php:"):
            lang = "php"
            if not prefix == None:
                s = prefix + "/" + s
                print s,
                continue

        if s.startswith(subincludedir):
            s = "$(includedir)" + s[len(subincludedir):]
            print s,
            continue

        if s.startswith(subcppincludedir):
            s = "$(ice_cpp_dir)/include" + s[len(subcppincludedir):]
            print s,
            continue

        idx = s.find("./slice")
        if idx >= 0:
            s = "$(slicedir)" + s[idx + 7:]
            print s,
            continue

        print s,

    if lang == "cs":
        print "$(SLICE2CS) $(SLICEPARSERLIB)"
    elif lang == "php":
        print "$(SLICE2PHP) $(SLICEPARSERLIB)"
    else:
        print
            
