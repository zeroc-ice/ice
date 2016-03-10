#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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

#
# Don't use a slave registry for this test. This can lead to race
# conditions when resolving endpoints of round-robin replica groups if
# the slave didn't fully load the servers at the time of the client
# invocation
#
IceGridAdmin.nreplicas=0

if TestUtil.isDarwin():
    #
    # On OS X, make sure to also run the IceBox services in 32bits mode if
    # x64 isn't specified and the service is built for 32bits.
    #
    iceBox = os.path.join(os.getcwd(), "iceboxwrapper")
    iceBoxWrapper = open(iceBox, "w")
    if TestUtil.x64:
        iceBoxWrapper.write("#!/bin/sh\narch -x86_64 " + TestUtil.getIceBox() + " \"$@\"\n")
    else:
        iceBoxWrapper.write("#!/bin/sh\narch -i386 -x86_64 " + TestUtil.getIceBox() + " \"$@\"\n")
    iceBoxWrapper.close()
    os.chmod(iceBox, 0o700)
else:
    iceBox = TestUtil.getIceBox()

IceGridAdmin.registryOptions += " --IceGrid.Registry.DynamicRegistration"

IceGridAdmin.iceGridTest("application.xml", "--Ice.RetryIntervals=\"0 50 100 250\"", "icebox.exe='%s'" % iceBox)

if TestUtil.isDarwin():
    os.unlink("iceboxwrapper")
