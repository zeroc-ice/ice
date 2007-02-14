#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil
import IceGridAdmin

name = os.path.join("IceGrid", "simple")

#
# Test client/server without on demand activation.
#
IceGridAdmin.iceGridClientServerTest(name, "", "--TestAdapter.Endpoints=default" + \
                                               " --TestAdapter.AdapterId=TestAdapter")

#
# Test client/server with on demand activation.
#
IceGridAdmin.iceGridTest(name, "simple_server.xml", "--with-deploy")
sys.exit(0)
