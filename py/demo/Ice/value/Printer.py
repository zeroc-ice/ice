# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Value, _Top, string

class PrinterI(_Top.Printer):
    def printBackwards(self, current=None):
        print self.message[::-1]

class DerivedPrinterI(_Top.DerivedPrinter, PrinterI):
    def printUppercase(self, current=None):
        print string.upper(self.derivedMessage)
