#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Demo, string, Ice

class PrinterI(Demo.Printer):
    def printBackwards(self, current=None):
        print(self.message[::-1])

class DerivedPrinterI(Demo.DerivedPrinter, PrinterI):
    def printUppercase(self, current=None):
        print(self.derivedMessage.upper())

class ClientPrinterI(Demo.ClientPrinter, PrinterI):
    pass

class ObjectFactory(Ice.ObjectFactory):
    def create(self, type):
        if type == Demo.Printer.ice_staticId():
            return PrinterI()

        if type == Demo.DerivedPrinter.ice_staticId():
            return DerivedPrinterI()

        if type == Demo.ClientPrinter.ice_staticId():
            return ClientPrinterI()

        assert(False)

    def destroy(self):
        # Nothing to do
        pass
