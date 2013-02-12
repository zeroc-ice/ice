#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

require 'Ice'

Ice::loadSlice('Value.ice')

#
# Reopen Demo::Printer (generated 'on the fly' by loadSlice above)
# and define printBackwards
#
class Demo::Printer
    def printBackwards()
        puts message.reverse
    end
end

#
# We can also use derivation (and register a factory) to implement 
# local operations
#
class DerivedPrinterI < Demo::DerivedPrinter
    def printUppercase(current=nil)
        puts derivedMessage.upcase
    end
end

class ClientPrinterI < Demo::ClientPrinter
end

class ObjectFactory
    def create(type)
        if type == Demo::DerivedPrinter::ice_staticId()
            return DerivedPrinterI.new
        end

        if type == Demo::ClientPrinter::ice_staticId()
            return ClientPrinterI.new
        end

        fail "unknown type"
    end
end

#
# Redefine a number of methods in Ice::Application
#
class Ice::Application
    def interruptCallback(sig)
        begin
            Ice::Application::communicator.destroy
        rescue => ex
            puts ex
        end
        exit(0)
    end

    def run(args)
        if args.length > 0
            puts $0 + ": too many argumnets"
            return 1
        end

        #
        # Since this is an interactive demo we want the custom interrupt
        # callback to be called when the process is interrupted.
        #
        Ice::Application::callbackOnInterrupt

        initial = Demo::InitialPrx::checkedCast(Ice::Application::communicator().propertyToProxy('Initial.Proxy'))
        if not initial
            puts $0 + ": invalid proxy"
            return 1
        end

        puts "\n"\
             "Let's first transfer a simple object, for a class without\n"\
             "operations, and print its contents. No factory is ever required\n"\
             "for this.\n"\
             "[press enter]"
	STDOUT.flush
        STDIN.readline

        simple = initial.getSimple()
        puts "==> " + simple.message

        puts "\n"\
             "Yes, this worked. Now let's try to transfer an object for a class\n"\
             "with operations as type ::Demo::Printer.\n"\
             "[press enter]"
	STDOUT.flush
        STDIN.readline
      
        printer, printerProxy = initial.getPrinter()
        puts "==> " + printer.message

        puts "\n"\
             "Cool, it worked! Let's try calling the printBackwards() method\n"\
             "on the object we just received locally.\n"\
             "[press enter]"
	STDOUT.flush
        STDIN.readline

        print "==> "
        printer.printBackwards()

        puts "\n"\
             "Now we call the same method, but on the remote object. Watch the\n"\
             "server's output.\n"\
             "[press enter]"
	STDOUT.flush
        STDIN.readline

        printerProxy.printBackwards()

        puts "\n"\
             "Next, we transfer a derived object from the server.\n"\
             "With Ice for Ruby, installing an object factory is optional,\n"\
             "and since we compiled the Slice definition for DerivedPrinter, the object\n"\
             "is not sliced.\n"\
             "[press enter]"
	STDOUT.flush
        STDIN.readline

        derived = initial.getDerivedPrinter()
        puts "==> The type ID of the received object is \"" + derived.ice_id() + "\""
        fail unless derived.ice_id() == "::Demo::DerivedPrinter"

        puts "\n"\
             "However since we did not implement Demo::DerivedPrinter::printUppercase.\n"\
             "calling printUppercase on this object does not work.\n"\
             "[press enter]"
	STDOUT.flush
        STDIN.readline

        begin
            derived.printUpperCase
            puts $0 + ": Did not get the expected NoMethodError!"
            exit(false)
        rescue NoMethodError => ex
            print "==> ", ex, "\n"
        end

        puts "\n"\
             "Now we install a factory for the derived class, and try again.\n"\
             "[press enter]"
	STDOUT.flush
        STDIN.readline

        Ice::Application::communicator().addObjectFactory(ObjectFactory.new, Demo::DerivedPrinter::ice_staticId())

        derived = initial.getDerivedPrinter()
        puts "==> The type ID of the received object is \"" + derived.ice_id() + "\""

        puts "\n"\
             "Let's print the message contained in the derived object, and\n"\
             "call the operation printUppercase() on the derived object\n"\
             "locally.\n"\
             "[press enter]"
	STDOUT.flush
        STDIN.readline

        puts "==> " + derived.derivedMessage
        print "==> "
        derived.printUppercase()

	puts "\n"\
	     "Now let's make sure that slice is preserved with [\"preserve-slice\"]\n"\
	     "metadata. We create a derived type on the client and pass it to the\n"\
	     "server, which does not have a factory for the derived type.\n"\
	     "[press enter]\n"
	STDOUT.flush
        STDIN.readline

	clientp = ClientPrinterI.new
	clientp.message = "a message 4 u"
        Ice::Application::communicator().addObjectFactory(ObjectFactory.new, Demo::ClientPrinter::ice_staticId())

	derivedAsBase = initial.updatePrinterMessage(clientp)
        fail unless derivedAsBase.ice_id() == "::Demo::ClientPrinter"
	puts "==> " + derivedAsBase.message

        puts "\n"\
             "Finally, we try the same again, but instead of returning the\n"\
             "derived object, we throw an exception containing the derived\n"\
             "object.\n"\
             "[press enter]"
	STDOUT.flush
        STDIN.readline

        begin
            initial.throwDerivedPrinter()
            puts $0 + "Did not get the expected DerivedPrinterException!"
            exit(false)
        rescue Demo::DerivedPrinterException => ex
            derived = ex.derived
            fail unless derived
        end

        puts "==> " + derived.derivedMessage
        print "==> "
        derived.printUppercase()

        puts "\n"\
             "That's it for this demo. Have fun with Ice!"

        initial.shutdown()

        return 0
    end
end

app = Ice::Application.new
exit(app.main(ARGV, "config.client"))
