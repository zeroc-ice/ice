#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, Ice

Ice.loadSlice('Value.ice')
import Demo, Printer

class InitialI(Demo.Initial):
    def __init__(self, adapter):
        self._simple = Demo.Simple()
        self._simple.message = "a message 4 u"

        self._printer = Printer.PrinterI()
        self._printer.message = "Ice rulez!"
        self._printerProxy = Demo.PrinterPrx.uncheckedCast(adapter.addWithUUID(self._printer))

        self._derivedPrinter = Printer.DerivedPrinterI()
        self._derivedPrinter.message = self._printer.message
        self._derivedPrinter.derivedMessage = "a derived message 4 u"
        adapter.addWithUUID(self._derivedPrinter)

    def getSimple(self, current=None):
        return self._simple

    def getPrinterAsObject(self, current=None):
        return self._printer

    def getPrinter(self, current=None):
        return (self._printer, self._printerProxy)

    def getDerivedPrinter(self, current=None):
        return self._derivedPrinter

    def throwDerivedPrinter(self, current=None):
        ex = Demo.DerivedPrinterException()
        ex.derived = self._derivedPrinter
        raise ex

def run(argv, communicator):
    adapter = communicator.createObjectAdapter("Value")
    object = InitialI(adapter)
    adapter.add(object, Ice.stringToIdentity("initial"))
    adapter.activate()
    communicator.waitForShutdown()
    return True

try:
    properties = Ice.createProperties()
    properties.load("config")
    communicator = Ice.initializeWithProperties(sys.argv, properties)
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

if status:
    sys.exit(0)
else:
    sys.exit(1)
