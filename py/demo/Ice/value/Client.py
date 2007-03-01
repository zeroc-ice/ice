#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, Ice

Ice.loadSlice('Value.ice')
import Demo, Printer

class ObjectFactory(Ice.ObjectFactory):
    def create(self, type):
        if type == "::Demo::Printer":
            return Printer.PrinterI()

        if type == "::Demo::DerivedPrinter":
            return Printer.DerivedPrinterI()

        assert(False)

    def destroy(self):
        # Nothing to do
        pass

class Client(Ice.Application):
    def run(self, args):
        base = self.communicator().propertyToProxy('Value.Initial')
        initial = Demo.InitialPrx.checkedCast(base)
        if not initial:
            print args[0] + ": invalid proxy"
            return 1

        print '\n'\
              "Let's first transfer a simple object, for a class without\n"\
              "operations, and print its contents. No factory is required\n"\
              "for this.\n"\
              "[press enter]"
        raw_input()

        simple = initial.getSimple()
        print "==> " + simple.message

        print '\n'\
              "Yes, this worked. Now let's try to transfer an object for a class\n"\
              "with operations as type ::Demo::Printer, without installing a factory\n"\
              "first. This should give us a `no factory' exception.\n"\
              "[press enter]"
        raw_input()

        try:
            printer, printerProxy = initial.getPrinter()
            print args[0] + ": Did not get the expected NoObjectFactoryException!"
            sys.exit(false)
        except Ice.NoObjectFactoryException, ex:
            print "==>", ex

        print '\n'\
              "Yep, that's what we expected. Now let's try again, but with\n"\
              "installing an appropriate factory first. If successful, we print\n"\
              "the object's content.\n"\
              "[press enter]"
        raw_input()

        factory = ObjectFactory()
        self.communicator().addObjectFactory(factory, "::Demo::Printer")

        printer, printerProxy = initial.getPrinter()
        print "==> " + printer.message

        print '\n'\
              "Cool, it worked! Let's try calling the printBackwards() method\n"\
              "on the object we just received locally.\n"\
              "[press enter]"
        raw_input()

        print "==>",
        printer.printBackwards()

        print '\n'\
              "Now we call the same method, but on the remote object. Watch the\n"\
              "server's output.\n"\
              "[press enter]"
        raw_input()

        printerProxy.printBackwards()

        print '\n'\
              "Next, we transfer a derived object from the server as a base\n"\
              "object. Since we haven't yet installed a factory for the derived\n"\
              "class, the derived class (::Demo::DerivedPrinter) is sliced\n"\
              "to its base class (::Demo::Printer).\n"\
              "[press enter]"
        raw_input()

        derivedAsBase = initial.getDerivedPrinter()
        print "==> The type ID of the received object is \"" + derivedAsBase.ice_id() + "\""
        assert(derivedAsBase.ice_id() == "::Demo::Printer")

        print '\n'\
              "Now we install a factory for the derived class, and try again.\n"\
              "Because we receive the derived object as a base object, we\n"\
              "we need to do a dynamic_cast<> to get from the base to the derived object.\n"\
              "[press enter]"
        raw_input()

        self.communicator().addObjectFactory(factory, "::Demo::DerivedPrinter")

        derived = initial.getDerivedPrinter()
        print "==> The type ID of the received object is \"" + derived.ice_id() + "\""

        print '\n'\
              "Let's print the message contained in the derived object, and\n"\
              "call the operation printUppercase() on the derived object\n"\
              "locally.\n"\
              "[press enter]"
        raw_input()

        print "==> " + derived.derivedMessage
        print "==>",
        derived.printUppercase()

        print '\n'\
              "Finally, we try the same again, but instead of returning the\n"\
              "derived object, we throw an exception containing the derived\n"\
              "object.\n"\
              "[press enter]"
        raw_input()

        try:
            initial.throwDerivedPrinter()
            print args[0] + "Did not get the expected DerivedPrinterException!"
            sys.exit(false)
        except Demo.DerivedPrinterException, ex:
            derived = ex.derived
            assert(derived)

        print "==> " + derived.derivedMessage
        print "==>",
        derived.printUppercase()

        print '\n'\
              "That's it for this demo. Have fun with Ice!"

        initial.shutdown()

        return 0

app = Client()
sys.exit(app.main(sys.argv, "config.client"))
