#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, Ice

Ice.loadSlice('Value.ice')
import Demo, Printer

class Client(Ice.Application):
    def run(self, args):
        if len(args) > 1:
            print(self.appName() + ": too many arguments")
            return 1

        initial = Demo.InitialPrx.checkedCast(self.communicator().propertyToProxy('Initial.Proxy'))
        if not initial:
            print(args[0] + ": invalid proxy")
            return 1

        print('\n'\
              "Let's first transfer a simple object, for a class without\n"\
              "operations, and print its contents. No factory is required\n"\
              "for this.\n"\
              "[press enter]")
        sys.stdin.readline()

        simple = initial.getSimple()
        print("==> " + simple.message)

        print('\n'\
              "Yes, this worked. Now let's try to transfer an object for a class\n"\
              "with operations as type ::Demo::Printer, without installing a factory\n"\
              "first. This should give us a `no factory' exception.\n"\
              "[press enter]")
        sys.stdin.readline()

        try:
            printer, printerProxy = initial.getPrinter()
            print(args[0] + ": Did not get the expected NoObjectFactoryException!")
            sys.exit(false)
        except Ice.NoObjectFactoryException as ex:
            print("==>", ex)

        print('\n'\
              "Yep, that's what we expected. Now let's try again, but with\n"\
              "installing an appropriate factory first. If successful, we print\n"\
              "the object's content.\n"\
              "[press enter]")
        sys.stdin.readline()

        factory = Printer.ObjectFactory()
        self.communicator().addObjectFactory(factory, Demo.Printer.ice_staticId())

        printer, printerProxy = initial.getPrinter()
        print("==> " + printer.message)

        print('\n'\
              "Cool, it worked! Let's try calling the printBackwards() method\n"\
              "on the object we just received locally.\n"\
              "[press enter]")
        sys.stdin.readline()

        sys.stdout.write("==> ")
        sys.stdout.flush()
        printer.printBackwards()

        print('\n'\
              "Now we call the same method, but on the remote object. Watch the\n"\
              "server's output.\n"\
              "[press enter]")
        sys.stdin.readline()

        printerProxy.printBackwards()

        print('\n'\
              "Next, we transfer a derived object from the server as a base\n"\
              "object. Since we haven't yet installed a factory for the derived\n"\
              "class, the derived class (::Demo::DerivedPrinter) is sliced\n"\
              "to its base class (::Demo::Printer).\n"\
              "[press enter]")
        sys.stdin.readline()

        derivedAsBase = initial.getDerivedPrinter()
        print("==> The type ID of the received object is \"" + derivedAsBase.ice_id() + "\"")
        assert(derivedAsBase.ice_id() == Demo.Printer.ice_staticId())

        print('\n'\
              "Now we install a factory for the derived class, and try again.\n"\
              "[press enter]")
        sys.stdin.readline()

        self.communicator().addObjectFactory(factory, Demo.DerivedPrinter.ice_staticId())

        derived = initial.getDerivedPrinter()
        print("==> The type ID of the received object is \"" + derived.ice_id() + "\"")

        print('\n'\
              "Let's print the message contained in the derived object, and\n"\
              "call the operation printUppercase() on the derived object\n"\
              "locally.\n"\
              "[press enter]")
        sys.stdin.readline()

        print("==> " + derived.derivedMessage)
        sys.stdout.write("==> ")
        sys.stdout.flush()
        derived.printUppercase()


        print('\n'\
              "Now let's make sure that slice is preserved with [\"preserve-slice\"]\n"\
              "metadata. We create a derived type on the client and pass it to the\n"\
              "server, which does not have a factory for the derived type.\n"\
              "[press enter]\n")
        sys.stdin.readline()

        clientp = Printer.ClientPrinterI()
        clientp.message = "a message 4 u"
        self.communicator().addObjectFactory(factory, Demo.ClientPrinter.ice_staticId())

        derivedAsBase = initial.updatePrinterMessage(clientp)
        assert(derivedAsBase.ice_id() == Demo.ClientPrinter.ice_staticId())
        print("==> " + derivedAsBase.message)

        print('\n'\
              "Finally, we try the same again, but instead of returning the\n"\
              "derived object, we throw an exception containing the derived\n"\
              "object.\n"\
              "[press enter]")
        sys.stdin.readline()

        try:
            initial.throwDerivedPrinter()
            print(args[0] + "Did not get the expected DerivedPrinterException!")
            sys.exit(false)
        except Demo.DerivedPrinterException as ex:
            derived = ex.derived
            assert(derived)

        print("==> " + derived.derivedMessage)
        sys.stdout.write("==> ")
        sys.stdout.flush()
        derived.printUppercase()

        print('\n'\
              "That's it for this demo. Have fun with Ice!")

        initial.shutdown()

        return 0

app = Client()
sys.exit(app.main(sys.argv, "config.client"))
