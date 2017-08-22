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

# Test IceGrid discovery with multiple replicas
IceGridAdmin.nreplicas=2

clientOpts=""
if not TestUtil.isLinux():
    clientOpts='--IceLocatorDiscovery.Interface="{}"'.format("::1" if TestUtil.ipv6 else "127.0.0.1")

#
# Test client/server without on demand activation.
#
IceGridAdmin.iceGridClientServerTest(clientOpts, "--TestAdapter.Endpoints=default --TestAdapter.AdapterId=TestAdapter")

#
# Test client/server with on demand activation.
#
IceGridAdmin.iceGridTest("simple_server.xml", "--with-deploy")
