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

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    if os.path.exists(os.path.normpath(toplevel + "/config/TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

for i in \
    [ \
    "Slice/errorDetection", \
    "Ice/operations", \
    "Ice/exceptions", \
    "Ice/inheritance", \
    "Ice/faultTolerance", \
    "Ice/locationForward", \
    "IcePack/simple", \
    ]:

    dir = os.path.normpath(toplevel + "/test/" + i)
    
    print
    print "*** running tests in " + dir + ":"
    print

    os.system("python " + os.path.normpath(dir + "/run.py"))
