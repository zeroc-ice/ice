#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, re

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel os.getcwd()!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil

slice2cpp = '"%s"' % TestUtil.getSliceTranslator()

regex1 = re.compile("\.ice$", re.IGNORECASE)
files = []
for file in os.listdir(os.getcwd()):
    if(regex1.search(file)):
        files.append(file)

files.sort()

for file in files:

    sys.stdout.write(file + "... ")
    sys.stdout.flush()

    if file.find("Underscore") != -1:
        command = slice2cpp + ' --underscore -I. "%s"' % os.path.join(os.getcwd(), file)
    else:
        command = slice2cpp + ' -I. "%s"' % os.path.join(os.getcwd(), file)
    p = TestUtil.runCommand(command)
    (stdin, stdout, stderr) = (p.stdin, p.stdout, p.stderr)
    
    lines1 = stderr.readlines()
    lines2 = open(os.path.join(os.getcwd(), regex1.sub(".err", file)), "r").readlines()
    if len(lines1) != len(lines2):
        print("failed!")
        sys.exit(1)
    
    regex2 = re.compile("^.*(?=" + file + ")")
    i = 0
    while i < len(lines1):
        if sys.version_info[0] == 2:
            line1 = regex2.sub("", lines1[i]).strip()
            line2 = regex2.sub("", lines2[i]).strip()
        else:
            line1 = regex2.sub("", lines1[i].decode("utf-8")).strip()
            line2 = regex2.sub("", lines2[i]).strip()
        if line1 != line2:
            print("failed!")
            sys.exit(1)
        i = i + 1
    else:
        print("ok")

sys.exit(0)
