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

Ice::loadSlice('Hello.ice')

def menu
    print <<MENU
usage:
t: send greeting as twoway
o: send greeting as oneway
O: send greeting as batch oneway
d: send greeting as datagram
D: send greeting as batch datagram
f: flush all batch requests
T: set a timeout
S: switch secure mode on/off
s: shutdown server
x: exit
?: help
MENU
end

class Client < Ice::Application
    def run(args)
	properties = Ice::Application::communicator().getProperties()
	refProperty = 'Hello.Proxy'
	proxy = properties.getProperty(refProperty)
	if proxy.length == 0
	    puts $0 + ": property `" + refProperty + "' not set"
	    return false
	end

	twoway = Demo::HelloPrx::checkedCast(
	    Ice::Application::communicator().stringToProxy(proxy).ice_twoway().ice_timeout(-1).ice_secure(false))
	if not twoway
	    puts $0 + ": invalid proxy"
	    return false
	end

	oneway = Demo::HelloPrx::uncheckedCast(twoway.ice_oneway())
	batchOneway = Demo::HelloPrx::uncheckedCast(twoway.ice_batchOneway())
	datagram = Demo::HelloPrx::uncheckedCast(twoway.ice_datagram())
	batchDatagram = Demo::HelloPrx::uncheckedCast(twoway.ice_batchDatagram())

	secure = false
	timeout = -1

	menu()

	c = nil
	while c != 'x'
	    begin
		print "==> "
		STDOUT.flush
		line = STDIN.readline
		c = line[0..0]
		if c == 't'
		    twoway.sayHello()
		elsif c == 'o'
		    oneway.sayHello()
		elsif c == 'O'
		    batchOneway.sayHello()
		elsif c == 'd'
		    if secure
			puts "secure datagrams are not supported"
		    else
			datagram.sayHello()
		    end
		elsif c == 'D'
		    if secure
			puts "secure datagrams are not supported"
		    else
			batchDatagram.sayHello()
		    end
		elsif c == 'f'
		    Ice::Application::communicator().flushBatchRequests()
		elsif c == 'T'
		    if timeout == -1
			timeout = 2000
		    else
			timeout = -1
		    end

		    twoway = Demo::HelloPrx::uncheckedCast(twoway.ice_timeout(timeout))
		    oneway = Demo::HelloPrx::uncheckedCast(oneway.ice_timeout(timeout))
		    batchOneway = Demo::HelloPrx::uncheckedCast(batchOneway.ice_timeout(timeout))

		    if timeout == -1
			puts "timeout is now switched off"
		    else
			puts "timeout is now set to 2000ms"
		    end
		elsif c == 'S'
		    secure = !secure

		    twoway = Demo::HelloPrx::uncheckedCast(twoway.ice_secure(secure))
		    oneway = Demo::HelloPrx::uncheckedCast(oneway.ice_secure(secure))
		    batchOneway = Demo::HelloPrx::uncheckedCast(batchOneway.ice_secure(secure))
		    datagram = Demo::HelloPrx::uncheckedCast(datagram.ice_secure(secure))
		    batchDatagram = Demo::HelloPrx::uncheckedCast(batchDatagram.ice_secure(secure))

		    if secure
			puts "secure mode is now on"
		    else
			puts "secure mode is now off"
		    end
		elsif c == 's'
		    twoway.shutdown()
		elsif c == 'x'
		    # Nothing to do
		elsif c == '?'
		    menu()
		else
		    puts "unknown command `" + c + "'"
		    menu()
		end
	    rescue EOFError
		break
	    end
	end

	return true
    end
end

app = Client.new()
exit(app.main(ARGV, "config.client"))
