#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback

import Ice, AllTests

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def usage(n):
    sys.stderr.write("Usage: " + n + " port...\n")

def run(args, communicator):
    ports = []
    for arg in args[1:]:
        if arg[0] == '-':
            sys.stderr.write(args[0] + ": unknown option `" + arg + "'\n")
            usage(args[0])
            return False

        ports.append(12010 + int(arg))

    if len(ports) == 0:
        sys.stderr.write(args[0] + ": no ports specified\n")
        usage(args[0])
        return False

    try:
        AllTests.allTests(communicator, ports)
    except:
        traceback.print_exc()
        test(False)

    return True

try:
    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(sys.argv)

    #
    # This test aborts servers, so we don't want warnings.
    #
    initData.properties.setProperty('Ice.Warn.Connections', '0')

    communicator = Ice.initialize(sys.argv, initData)
    status = run(sys.argv, communicator)
except:
    traceback.print_exc()
    status = False

if communicator:
    try:
        communicator.destroy()
    except:
        traceback.print_exc()
        status = False

sys.exit(not status)
