#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2001
# MutableRealms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

import os

for toplevel in ["", "..", os.path.join("..", ".."), os.path.join("..", "..", "..")]:
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

for name in ["parser", "operations", "exceptions", "inheritance", "faultTolerance"]:

    dir = os.path.join(toplevel, "test", name)
    
    print
    print "*** running tests in " + dir + ":"
    print

    os.system("python " + os.path.join(dir, "run.py"))
    
