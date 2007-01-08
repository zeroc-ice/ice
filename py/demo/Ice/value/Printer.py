#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Demo, string

class PrinterI(Demo.Printer):
    def printBackwards(self, current=None):
        print self.message[::-1]

class DerivedPrinterI(Demo.DerivedPrinter, PrinterI):
    def printUppercase(self, current=None):
        print string.upper(self.derivedMessage)
