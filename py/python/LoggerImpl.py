# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, IcePy
import Logger_ice

__name__ = 'Ice'
_M_Ice = Ice.openModule('Ice')

class LoggerI(_M_Ice.Logger):

    def __init__(self, impl):
        self._impl = impl

    def trace(self, category, message):
        self._impl.trace(category, message)

    def warning(self, message):
        self._impl.warning(message)

    def error(self, message):
        self._impl.error(message)

_M_Ice.LoggerI = LoggerI
del LoggerI
