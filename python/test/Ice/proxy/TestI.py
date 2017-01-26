# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test
import time

class MyDerivedClassI(Test._MyDerivedClassDisp):
    def __init__(self):
        self.ctx = None

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

    def getContext(self, current):
        return self.ctx

    def echo(self, obj, current):
        return obj

    def ice_isA(self, s, current):
        self.ctx = current.ctx
        return Test._MyDerivedClassDisp.ice_isA(self, s, current)
