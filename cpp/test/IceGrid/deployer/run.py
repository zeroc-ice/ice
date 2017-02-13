#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil, IceGridAdmin

os.environ["MY_FOO"] = "12"
IceGridAdmin.iceGridTest("application.xml", '--TestDir="%s"' % os.getcwd(), "icebox.exe='%s'" % TestUtil.getIceBox())

# Tests with targets
IceGridAdmin.iceGridTest("application.xml", '-t --TestDir="%s"' % os.getcwd(),
                         "icebox.exe='%s' moreservers moreservices moreproperties" % TestUtil.getIceBox())
