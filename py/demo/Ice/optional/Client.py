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

Ice.loadSlice("Contact.ice")
import Demo

class ContactClient(Ice.Application):
    def run(self, args):
        if len(args) > 1:
            print(self.appName() + ": too many arguments")
            return 1

        contactdb = Demo.ContactDBPrx.checkedCast(self.communicator().propertyToProxy("ContactDB.Proxy"))
        if contactdb == None:
            print("invalid proxy")
            return 1

        #
        # Add a contact for "john". All parameters are provided.
        #
        johnNumber = "123-456-7890"
        contactdb.addContact("john", Demo.NumberType.HOME, johnNumber, 0)

        sys.stdout.write("Checking john... ")
        sys.stdout.flush()

        #
        # Find the phone number for "john"
        #
        number = contactdb.queryNumber("john")
        if number != johnNumber:
            sys.stdout.write("number is incorrect ")
            sys.stdout.flush()

        dialgroup = contactdb.queryDialgroup("john")
        if dialgroup != 0:
            sys.stdout.write("dialgroup is incorrect ")
            sys.stdout.flush()

        info = contactdb.query("john")
        #
        # All of the info parameters should be set.
        #
        if info.type != Demo.NumberType.HOME or info.number != johnNumber or info.dialGroup != 0:
            sys.stdout.write("info is incorrect ")
            sys.stdout.flush()
        print("ok")

        #
        # Add a contact for "steve". The behavior of the server is to
        # default construct the Contact, and then assign all set parameters.
        # Since the default value of NumberType in the slice definition
        # is HOME and in this case the NumberType is unset it will take
        # the default value.
        #
        steveNumber = "234-567-8901"
        contactdb.addContact("steve", Ice.Unset, steveNumber, 1)

        sys.stdout.write("Checking steve... ")
        sys.stdout.flush()
        number = contactdb.queryNumber("steve")
        if number != steveNumber:
            sys.stdout.write("number is incorrect ")
            sys.stdout.flush()

        info = contactdb.query("steve")
        #
        # Check the value for the NumberType.
        #
        if info.type != Demo.NumberType.HOME:
            sys.stdout.write("info is incorrect ")
            sys.stdout.flush()

        if info.number != steveNumber or info.dialGroup != 1:
            sys.stdout.write("info is incorrect ")
            sys.stdout.flush()

        dialgroup = contactdb.queryDialgroup("steve")
        if dialgroup != 1:
            sys.stdout.write("dialgroup is incorrect ")
            sys.stdout.flush()

        print("ok")

        #
        # Add a contact from "frank". Here the dialGroup field isn't set.
        #
        frankNumber = "345-678-9012"
        contactdb.addContact("frank", Demo.NumberType.CELL, frankNumber, Ice.Unset)

        sys.stdout.write("Checking frank... ")
        sys.stdout.flush()

        number = contactdb.queryNumber("frank")
        if number != frankNumber:
            sys.stdout.write("number is incorrect ")
            sys.stdout.flush()

        info = contactdb.query("frank")
        #
        # The dial group field should be unset.
        #
        if info.dialGroup != Ice.Unset:
            sys.stdout.write("info is incorrect ")
            sys.stdout.flush()
        if info.type != Demo.NumberType.CELL or info.number != frankNumber:
            sys.stdout.write("info is incorrect ")
            sys.stdout.flush()

        dialgroup = contactdb.queryDialgroup("frank")
        if dialgroup != Ice.Unset:
            sys.stdout.write("dialgroup is incorrect ")
            sys.stdout.flush()
        print("ok")

        #
        # Add a contact from "anne". The number field isn't set.
        #
        contactdb.addContact("anne", Demo.NumberType.OFFICE, Ice.Unset, 2)

        sys.stdout.write("Checking anne... ")
        sys.stdout.flush()
        number = contactdb.queryNumber("anne")
        if number != Ice.Unset:
            sys.stdout.write("number is incorrect ")
            sys.stdout.flush()

        info = contactdb.query("anne")
        #
        # The number field should be unset.
        #
        if info.number != Ice.Unset:
            sys.stdout.write("info is incorrect ")
            sys.stdout.flush()
        if info.type != Demo.NumberType.OFFICE or info.dialGroup != 2:
            sys.stdout.write("info is incorrect ")
            sys.stdout.flush()

        dialgroup = contactdb.queryDialgroup("anne")
        if dialgroup != 2:
            sys.stdout.write("dialgroup is incorrect ")
            sys.stdout.flush()

        #
        # The optional fields can be used to determine what fields to
        # update on the contact. Here we update only the number for anne,
        # the remainder of the fields are unchanged.
        #
        anneNumber = "456-789-0123"
        contactdb.updateContact("anne", Ice.Unset, anneNumber, Ice.Unset)
        number = contactdb.queryNumber("anne")
        if number != anneNumber:
            sys.stdout.write("number is incorrect ")
            sys.stdout.flush()
        info = contactdb.query("anne")
        if info.number != anneNumber or info.type != Demo.NumberType.OFFICE or info.dialGroup != 2:
            sys.stdout.write("info is incorrect ")
            sys.stdout.flush()
        print("ok")

        contactdb.shutdown()

        return 0

app = ContactClient()
sys.exit(app.main(sys.argv, "config.client"))
