# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, IcePy
import Properties_ice

__name__ = 'Ice'
_M_Ice = Ice.openModule('Ice')

class PropertiesI(_M_Ice.Properties):
    def __init__(self, impl):
        self._impl = impl

    def getProperty(self, key):
        return self._impl.getProperty(key)

    def getPropertyWithDefault(self, key, value):
        return self._impl.getPropertyWithDefault(key, value)

    def getPropertyAsInt(self, key):
        return self._impl.getPropertyAsInt(key)

    def getPropertyAsIntWithDefault(self, key, value):
        return self._impl.getPropertyAsIntWithDefault(key, value)

    def getPropertiesForPrefix(self, prefix):
        return self._impl.getPropertiesForPrefix(prefix)

    def setProperty(self, key, value):
        self._impl.setProperty(key, value)

    def getCommandLineOptions(self):
        return self._impl.getCommandLineOptions()

    def parseCommandLineOptions(self, prefix, options):
        self._impl.parseCommandLineOptions(prefix, options)

    def parseIceCommandLineOptions(self, options):
        self._impl.parseIceCommandLineOptions(options)

    def load(self, file):
        self._impl.load(file)

    def clone(self):
        properties = self._impl.clone()
        return _M_Ice.PropertiesI(properties)

    def __iter__(self):
        dict = self._impl.getPropertiesForPrefix('')
        return iter(dict)

    def __str__(self):
        return str(self._impl)

_M_Ice.PropertiesI = PropertiesI
del PropertiesI

def createProperties(args=[]):
    properties = IcePy.createProperties(args)
    return _M_Ice.PropertiesI(properties)

_M_Ice.createProperties = createProperties
del createProperties

def getDefaultProperties(args=[]):
    properties = IcePy.getDefaultProperties(args)
    return _M_Ice.PropertiesI(properties)

_M_Ice.getDefaultProperties = getDefaultProperties
del getDefaultProperties
