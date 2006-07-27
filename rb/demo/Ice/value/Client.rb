#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

require 'Ice'

Ice::loadSlice('Value.ice')

module PrinterI_mixin
    def printBackwards(current=nil)
	puts message.reverse
    end
end

class PrinterI < Demo::Printer
    include PrinterI_mixin
end

class DerivedPrinterI < Demo::DerivedPrinter
    include PrinterI_mixin

    def printUppercase(current=nil)
	puts derivedMessage.upcase
    end
end

class ObjectFactory
    def create(type)
	if type == "::Demo::Printer"
	    return PrinterI.new
	end

	if type == "::Demo::DerivedPrinter"
	    return DerivedPrinterI.new
	end

	fail "unknown type"
    end

    def destroy
	# Nothing to do
    end
end

class Client < Ice::Application
    def run(args)
	properties = Ice::Application::communicator().getProperties()
	refProperty = 'Value.Initial'
	proxy = properties.getProperty(refProperty)
	if proxy.length == 0
	    puts $0 + ": property `" + refProperty + "' not set"
	    return false
	end

	base = Ice::Application::communicator().stringToProxy(proxy)
	initial = Demo::InitialPrx::checkedCast(base)
	if not initial
	    puts $0 + ": invalid proxy"
	    return false
	end

	puts "\n"\
	     "Let's first transfer a simple object, for a class without\n"\
	     "operations, and print its contents. No factory is required\n"\
	     "for this.\n"\
	     "[press enter]"
	STDIN.readline

	simple = initial.getSimple()
	puts "==> " + simple.message

	puts "\n"\
	     "Yes, this worked. Now let's try to transfer an object for a class\n"\
	     "with operations as type ::Demo::Printer, without installing a factory\n"\
	     "first. This should give us a `no factory' exception.\n"\
	     "[press enter]"
	STDIN.readline

	begin
	    printer, printerProxy = initial.getPrinter()
	    puts $0 + ": Did not get the expected NoObjectFactoryException!"
	    exit(false)
	rescue Ice::NoObjectFactoryException => ex
	    puts "==> " + ex
	end

	puts "\n"\
	     "Yep, that's what we expected. Now let's try again, but with\n"\
	     "installing an appropriate factory first. If successful, we print\n"\
	     "the object's content.\n"\
	     "[press enter]"
	STDIN.readline

	factory = ObjectFactory.new
	Ice::Application::communicator().addObjectFactory(factory, "::Demo::Printer")

	printer, printerProxy = initial.getPrinter()
	puts "==> " + printer.message

	puts "\n"\
	     "Cool, it worked! Let's try calling the printBackwards() method\n"\
	     "on the object we just received locally.\n"\
	     "[press enter]"
	STDIN.readline

	print "==> "
	printer.printBackwards()

	puts "\n"\
	     "Now we call the same method, but on the remote object. Watch the\n"\
	     "server's output.\n"\
	     "[press enter]"
	STDIN.readline

	printerProxy.printBackwards()

	puts "\n"\
	     "Next, we transfer a derived object from the server as a base\n"\
	     "object. Since we haven't yet installed a factory for the derived\n"\
	     "class, the derived class (::Demo::DerivedPrinter) is sliced\n"\
	     "to its base class (::Demo::Printer).\n"\
	     "[press enter]"
	STDIN.readline

	derivedAsBase = initial.getDerivedPrinter()
	puts "==> The type ID of the received object is \"" + derivedAsBase.ice_id() + "\""
	fail unless derivedAsBase.ice_id() == "::Demo::Printer"

	puts "\n"\
	     "Now we install a factory for the derived class, and try again.\n"\
	     "Because we receive the derived object as a base object, we\n"\
	     "we need to do a dynamic_cast<> to get from the base to the derived object.\n"\
	     "[press enter]"
	STDIN.readline

	Ice::Application::communicator().addObjectFactory(factory, "::Demo::DerivedPrinter")

	derived = initial.getDerivedPrinter()
	puts "==> The type ID of the received object is \"" + derived.ice_id() + "\""

	puts "\n"\
	     "Let's print the message contained in the derived object, and\n"\
	     "call the operation printUppercase() on the derived object\n"\
	     "locally.\n"\
	     "[press enter]"
	STDIN.readline

	puts "==> " + derived.derivedMessage
	print "==> "
	derived.printUppercase()

	puts "\n"\
	     "Finally, we try the same again, but instead of returning the\n"\
	     "derived object, we throw an exception containing the derived\n"\
	     "object.\n"\
	     "[press enter]"
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

	return true
    end
end

app = Client.new
exit(app.main(ARGV, "config.client"))
