# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from .Properties import Properties

class PropertiesI(Properties):
    def __init__(self, impl):
        self._impl = impl

    def getProperty(self, key):
        return self._impl.getProperty(key)

    def getIceProperty(self, key):
        return self._impl.getIceProperty(key)

    def getPropertyWithDefault(self, key, value):
        return self._impl.getPropertyWithDefault(key, value)

    def getPropertyAsInt(self, key):
        return self._impl.getPropertyAsInt(key)

    def getIcePropertyAsInt(self, key):
        return self._impl.getIcePropertyAsInt(key)

    def getPropertyAsIntWithDefault(self, key, value):
        return self._impl.getPropertyAsIntWithDefault(key, value)

    def getPropertyAsList(self, key):
        return self._impl.getPropertyAsList(key)

    def getIcePropertyAsList(self, key):
        return self._impl.getIcePropertyAsList(key)

    def getPropertyAsListWithDefault(self, key, value):
        return self._impl.getPropertyAsListWithDefault(key, value)

    def getPropertiesForPrefix(self, prefix):
        return self._impl.getPropertiesForPrefix(prefix)

    def setProperty(self, key, value):
        self._impl.setProperty(key, value)

    def getCommandLineOptions(self):
        return self._impl.getCommandLineOptions()

    def parseCommandLineOptions(self, prefix, options):
        return self._impl.parseCommandLineOptions(prefix, options)

    def parseIceCommandLineOptions(self, options):
        return self._impl.parseIceCommandLineOptions(options)

    def load(self, file):
        self._impl.load(file)

    def clone(self):
        properties = self._impl.clone()
        return PropertiesI(properties)

    def __iter__(self):
        dict = self._impl.getPropertiesForPrefix("")
        return iter(dict)

    def __str__(self):
        return str(self._impl)
