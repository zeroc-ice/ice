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

class BI(Test.B):
    def __init__(self):
        self.preMarshalInvoked = False
        self._postUnmarshalInvoked = False

    def postUnmarshalInvoked(self, current=None):
        return self._postUnmarshalInvoked

    def ice_preMarshal(self):
        self.preMarshalInvoked = True

    def ice_postUnmarshal(self):
        self._postUnmarshalInvoked = True

class CI(Test.C):
    def __init__(self):
        self.preMarshalInvoked = False
        self._postUnmarshalInvoked = False

    def postUnmarshalInvoked(self, current=None):
        return self._postUnmarshalInvoked

    def ice_preMarshal(self):
        self.preMarshalInvoked = True

    def ice_postUnmarshal(self):
        self._postUnmarshalInvoked = True

class DI(Test.D):
    def __init__(self):
        self.preMarshalInvoked = False
        self._postUnmarshalInvoked = False

    def postUnmarshalInvoked(self, current=None):
        return self._postUnmarshalInvoked

    def ice_preMarshal(self):
        self.preMarshalInvoked = True

    def ice_postUnmarshal(self):
        self._postUnmarshalInvoked = True
