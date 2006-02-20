#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, Ice, Test

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def allTests(communicator):
    print "testing stringToProxy... ",
    sys.stdout.flush()
    base = communicator.stringToProxy("test:default -p 12010 -t 10000")
    test(base)
    print "ok"

    print "testing checked cast... ",
    sys.stdout.flush()
    obj = Test.TestIntfPrx.checkedCast(base)
    test(obj)
    test(obj == base)
    print "ok"

    print "creating/activating/deactivating object adapter in one operation... ",
    sys.stdout.flush()
    obj.transient()
    print "ok"

    print "deactivating object adapter in the server... ",
    sys.stdout.flush()
    obj.deactivate()
    print "ok"

    print "testing whether server is gone... ",
    sys.stdout.flush()
    try:
	obj.ice_ping()
	test(False)
    except Ice.LocalException:
	print "ok"

    return obj
