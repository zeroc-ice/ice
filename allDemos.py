#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, imp

sys.path.append(os.path.join(os.path.dirname(__file__), "demoscript"))
from demoscript import *

demoGroups = []

if os.path.isdir("cpp"):
    subdirs = [ "cpp", "java", "cs", "py", "rb"]
else:
    subdirs = [ "demo", "demoj", "democs", "demopy", "demorb"]

for d in subdirs:
    
    filename = os.path.abspath(os.path.join(os.path.dirname(__file__), d, "allDemos.py"))
    f = file(filename, "r")
    current_mod = imp.load_module("allDemos", f, filename, (".py", "r", imp.PY_SOURCE)) 
    f.close()

    demos = [ os.path.join(d, "demo", x) for x in current_mod.demos ]
    demoGroups.extend(demos)

Util.run(demoGroups, root=True)
