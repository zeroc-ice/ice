#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

    def getPrinter(self, current=None):
        return (self._printer, self._printerProxy)

    def getDerivedPrinter(self, current=None):
        return self._derivedPrinter

    def updatePrinterMessage(self, printer, current=None):
        printer.message = "a modified message 4 u"
        return printer

    def throwDerivedPrinter(self, current=None):
        ex = Demo.DerivedPrinterException()
        ex.derived = self._derivedPrinter
        raise ex

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

class Server(Ice.Application):
    def run(self, args):
        if len(args) > 1:
            print(self.appName() + ": too many arguments")
            return 1

        factory = Printer.ObjectFactory()
        self.communicator().addObjectFactory(factory, Demo.Printer.ice_staticId())

        adapter = self.communicator().createObjectAdapter("Value")
        adapter.add(InitialI(adapter), self.communicator().stringToIdentity("initial"))
        adapter.activate()
        self.communicator().waitForShutdown()
        return 0

app = Server()
sys.exit(app.main(sys.argv, "config.server"))
