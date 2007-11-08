#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, getopt

for toplevel in [".", "..", "../..", "../../..", "../../../..", "../../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil
import IceGridAdmin

ice_home = None
if not os.environ.has_key('ICE_HOME'):
    relPath = os.path.join(TestUtil.findTopLevel(), "cpp", "bin") 
    if os.path.exists(os.path.join(relPath, "icegridregistry")) or \
        os.path.exists(os.path.join(relPath, "icegridregistry.exe")):
        ice_home = os.path.dirname(relPath) 
    else:
        print "ICE_HOME is not defined."
        sys.exit(0)
else:
    ice_home = os.environ['ICE_HOME']

name = os.path.join("IceGrid", "simple")
testdir = os.path.dirname(os.path.abspath(__file__))

#
# Test client/server without on demand activation.
#
IceGridAdmin.iceGridClientServerTest(testdir, name, "", "--TestAdapter.Endpoints=default" + \
        " --TestAdapter.AdapterId=TestAdapter")

#
# Test client/server with on demand activation.
#
if TestUtil.mono:
    IceGridAdmin.iceGridTest(testdir, name, "simple_mono_server.xml", "--with-deploy")
else:
    IceGridAdmin.iceGridTest(testdir, name, "simple_server.xml", "--with-deploy")    
sys.exit(0)
