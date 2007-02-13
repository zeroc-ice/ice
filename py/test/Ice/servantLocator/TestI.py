#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback, time
import Ice, Test

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class TestI(Test.TestIntf):

    def requestFailedException(self, current=None):
        pass

    def unknownUserException(self, current=None):
        pass

    def unknownLocalException(self, current=None):
        pass

    def unknownException(self, current=None):
        pass

    def localException(self, current=None):
        pass

    def userException(self, current=None):
        pass

    def pythonException(self, current=None):
        pass

    def shutdown(self, current=None):
        current.adapter.deactivate()

class CookieI(Test.Cookie):
    def message(self):
        return 'blahblah'

class ServantLocatorI(Ice.ServantLocator):
    def __init__(self, category):
        self._deactivated = False
        self._category = category

    def __del__(self):
        test(self._deactivated)

    def locate(self, current):
        test(not self._deactivated)

        test(current.id.category == self._category or self._category == "")
        
        if current.id.name == "unknown":
            return None

        test(current.id.name == "locate" or current.id.name == "finished")
        if current.id.name == "locate":
            self.exception(current)

        return (TestI(), CookieI())

    def finished(self, current, servant, cookie):
        test(not self._deactivated)

        test(current.id.category == self._category  or self._category == "")
        test(current.id.name == "locate" or current.id.name == "finished")
        
        if current.id.name == "finished":
            self.exception(current)

        test(isinstance(cookie, Test.Cookie))
        test(cookie.message() == 'blahblah')

    def deactivate(self, category):
        test(not self._deactivated)

        self._deactivated = True

    def exception(self, current):
        if current.operation == "requestFailedException":
            raise Ice.ObjectNotExistException()
        elif current.operation == "unknownUserException":
            ex = Ice.UnknownUserException()
            ex.unknown = "reason"
            raise ex
        elif current.operation == "unknownLocalException":
            ex = Ice.UnknownLocalException()
            ex.unknown = "reason"
            raise ex
        elif current.operation == "unknownException":
            ex = Ice.UnknownException()
            ex.unknown = "reason"
            raise ex
        elif current.operation == "userException":
            raise Test.TestIntfUserException()
        elif current.operation == "localException":
            ex = Ice.SocketException()
            ex.error = 0
            raise ex
        elif current.operation == "pythonException":
            raise RuntimeError("message")
