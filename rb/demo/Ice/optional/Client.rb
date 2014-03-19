#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

require 'Ice'

Ice::loadSlice('Contact.ice')

class ContactClient < Ice::Application
    def run(args)
        if args.length > 0
            puts $0 + ": too many argumnets"
            return 1
        end

        contactdb = Demo::ContactDBPrx::checkedCast(Ice::Application::communicator().propertyToProxy('ContactDB.Proxy'))
        if not contactdb
            puts "invalid proxy"
            return 1
        end

        #
        # Add a contact for "john". All parameters are provided.
        #
        johnNumber = "123-456-7890"
        contactdb.addContact("john", Demo::NumberType::HOME, johnNumber, 0)

        print "Checking john... "
        STDOUT.flush()

        #
        # Find the phone number for "john"
        #
        number = contactdb.queryNumber("john")
        if number != johnNumber
            print "number is incorrect "
            STDOUT.flush()
        end

        dialgroup = contactdb.queryDialgroup("john")
        if dialgroup != 0
            print "dialgroup is incorrect "
            STDOUT.flush()
        end

        info = contactdb.query("john")
        #
        # All of the info parameters should be set.
        #
        if info.type != Demo::NumberType::HOME or info.number != johnNumber or info.dialGroup != 0
            print "info is incorrect "
            STDOUT.flush()
        end

        puts("ok")

        #
        # Add a contact for "steve". The behavior of the server is to
        # default construct the Contact, and then assign all set parameters.
        # Since the default value of NumberType in the slice definition
        # is HOME and in this case the NumberType is unset it will take
        # the default value.
        #
        steveNumber = "234-567-8901"
        contactdb.addContact("steve", Ice::Unset, steveNumber, 1)

        print "Checking steve... "
        STDOUT.flush()
        number = contactdb.queryNumber("steve")
        if number != steveNumber
            print "number is incorrect "
            STDOUT.flush()
        end

        info = contactdb.query("steve")
        #
        # Check the value for the NumberType.
        #
        if info.type != Demo::NumberType::HOME
            print "info is incorrect "
            STDOUT.flush()
        end

        if info.number != steveNumber or info.dialGroup != 1
            print "info is incorrect "
            STDOUT.flush()
        end

        dialgroup = contactdb.queryDialgroup("steve")
        if dialgroup != 1
            print "dialgroup is incorrect "
            STDOUT.flush()
        end

        puts("ok")

        #
        # Add a contact from "frank". Here the dialGroup field isn't set.
        #
        frankNumber = "345-678-9012"
        contactdb.addContact("frank", Demo::NumberType::CELL, frankNumber, Ice::Unset)

        print "Checking frank... "
        STDOUT.flush()

        number = contactdb.queryNumber("frank")
        if number != frankNumber
            print "number is incorrect "
            STDOUT.flush()
        end

        info = contactdb.query("frank")
        #
        # The dial group field should be unset.
        #
        if info.dialGroup != Ice::Unset
            print "info is incorrect "
            STDOUT.flush()
        end
        if info.type != Demo::NumberType::CELL or info.number != frankNumber
            print "info is incorrect "
            STDOUT.flush()
        end

        dialgroup = contactdb.queryDialgroup("frank")
        if dialgroup != Ice::Unset
            print "dialgroup is incorrect "
            STDOUT.flush()
        end

        puts("ok")

        #
        # Add a contact from "anne". The number field isn't set.
        #
        contactdb.addContact("anne", Demo::NumberType::OFFICE, Ice::Unset, 2)

        print "Checking anne... "
        STDOUT.flush()
        number = contactdb.queryNumber("anne")
        if number != Ice::Unset
            print "number is incorrect "
            STDOUT.flush()
        end

        info = contactdb.query("anne")
        #
        # The number field should be unset.
        #
        if info.number != Ice::Unset
            print "info is incorrect "
            STDOUT.flush()
        end
        if info.type != Demo::NumberType::OFFICE or info.dialGroup != 2
            print "info is incorrect "
            STDOUT.flush()
        end

        dialgroup = contactdb.queryDialgroup("anne")
        if dialgroup != 2
            print "dialgroup is incorrect "
            STDOUT.flush()
        end

        #
        # The optional fields can be used to determine what fields to
        # update on the contact. Here we update only the number for anne,
        # the remainder of the fields are unchanged.
        #
        anneNumber = "456-789-0123"
        contactdb.updateContact("anne", Ice::Unset, anneNumber, Ice::Unset)
        number = contactdb.queryNumber("anne")
        if number != anneNumber
            print "number is incorrect "
            STDOUT.flush()
        end
        info = contactdb.query("anne")
        if info.number != anneNumber or info.type != Demo::NumberType::OFFICE or info.dialGroup != 2
            print "info is incorrect "
            STDOUT.flush()
        end

        puts("ok")

        contactdb.shutdown()

        return 0
    end
end

app = ContactClient.new
exit(app.main(ARGV, "config.client"))
