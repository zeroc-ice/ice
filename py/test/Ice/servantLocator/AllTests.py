#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, Ice, Test

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def testExceptions(obj, collocated):

    try:
        obj.requestFailedException()
        test(false)
    except Ice.ObjectNotExistException, ex:
        if not collocated:
            test(ex.id == obj.ice_getIdentity())
            test(ex.facet == obj.ice_getFacet())
            test(ex.operation == "requestFailedException")

    try:
        obj.unknownUserException()
        test(false)
    except Ice.UnknownUserException, ex:
        test(ex.unknown == "reason")
        pass

    try:
        obj.unknownLocalException()
        test(false)
    except Ice.UnknownLocalException, ex:
        test(ex.unknown == "reason")
        pass

    try:
        obj.unknownException()
        test(false)
    except Ice.UnknownException, ex:
        test(ex.unknown == "reason")
        pass

    try:
        obj.userException()
        test(false)
    except Ice.UnknownUserException, ex:
        #print ex.unknown
        test(not collocated)
        test(ex.unknown.find("Test.TestIntfUserException") >= 0)
    except Test.TestIntfUserException:
        test(collocated)

    try:
        obj.localException()
        test(false)
    except Ice.UnknownLocalException, ex:
        #print ex.unknown
        test(not collocated)
        test(ex.unknown.find("Ice.SocketException") >= 0)
    except SocketException:
        test(collocated)

    try:
        obj.pythonException()
        test(false)
    except Ice.UnknownException, ex:
        #print ex.unknown
        test(not collocated)
        test(ex.unknown.find("RuntimeError: message") >= 0)
    except RuntimeError:
        test(collocated)

def allTests(communicator, collocated):
    print "testing stringToProxy... ",
    sys.stdout.flush()
    base = communicator.stringToProxy("asm:default -p 12010 -t 10000")
    test(base)
    print "ok"

    print "testing checked cast... ",
    sys.stdout.flush()
    obj = Test.TestIntfPrx.checkedCast(base)
    test(obj)
    test(obj == base)
    print "ok"

    print "testing servant locator...",
    sys.stdout.flush()
    base = communicator.stringToProxy("category/locate:default -p 12010 -t 10000")
    obj = Test.TestIntfPrx.checkedCast(base)
    try:
        Test.TestIntfPrx.checkedCast(communicator.stringToProxy("category/unknown:default -p 12010 -t 10000"))
    except Ice.ObjectNotExistException:
        pass
    print "ok"

    print "testing default servant locator...",
    sys.stdout.flush()
    base = communicator.stringToProxy("anothercat/locate:default -p 12010 -t 10000")
    obj = Test.TestIntfPrx.checkedCast(base)
    base = communicator.stringToProxy("locate:default -p 12010 -t 10000")
    obj = Test.TestIntfPrx.checkedCast(base)
    try:
        Test.TestIntfPrx.checkedCast(communicator.stringToProxy("anothercat/unknown:default -p 12010 -t 10000"))
    except Ice.ObjectNotExistException:
        pass
    try:
        Test.TestIntfPrx.checkedCast(communicator.stringToProxy("unknown:default -p 12010 -t 10000"))
    except Ice.ObjectNotExistException:
        pass
    print "ok"

    print "testing locate exceptions... ",
    sys.stdout.flush()
    base = communicator.stringToProxy("category/locate:default -p 12010 -t 10000")
    obj = Test.TestIntfPrx.checkedCast(base)
    testExceptions(obj, collocated)
    print "ok"

    print "testing finished exceptions... ",
    sys.stdout.flush()
    base = communicator.stringToProxy("category/finished:default -p 12010 -t 10000")
    obj = Test.TestIntfPrx.checkedCast(base)
    testExceptions(obj, collocated)
    print "ok"

    return obj
