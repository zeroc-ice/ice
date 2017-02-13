#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, getopt, re

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil

# Filter-out the deprecated property warnings
TestUtil.clientTraceFilters = [ lambda x: re.sub("-! .* warning: deprecated property: IceSSL.FindCert.*\n", "", x) ]
TestUtil.serverTraceFilters = [ lambda x: re.sub("-! .* warning: deprecated property: IceSSL.FindCert.*\n", "", x) ]
#
# The drive letter needs to be removed on Windows or loading the SSL
# plug-in will not work.
#
TestUtil.clientServerTest(additionalClientOptions = '"%s"' % os.path.splitdrive(os.getcwd())[1])
