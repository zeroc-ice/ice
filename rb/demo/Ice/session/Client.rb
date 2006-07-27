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
require 'thread'
require 'timeout'

Ice::loadSlice('Session.ice')

class SessionRefreshThread
    def initialize(logger, timeout, session)
	@_logger = logger
	@_session = session
	@_timeout = timeout
	@_terminated = false
	@_cond = ConditionVariable.new
	@_mutex = Mutex.new
    end

    def run
	@_mutex.synchronize {
	    while not @_terminated
		begin
		    Timeout::timeout(@_timeout) do
			@_cond.wait(@_mutex)
		    end
		rescue Timeout::Error
		end
		if not @_terminated
		    begin
			@_session.refresh()
		    rescue Ice::LocalException => ex
			@_logger.warning("SessionRefreshThread: " + ex)
			@_terminated = true
		    end
		end
	    end
	}
    end

    def terminate
	@_mutex.synchronize {
	    @_terminated = true
	    @_cond.signal
	}
    end
end

class Client < Ice::Application
    def run(args)
	while true
	    print "Please enter your name ==> "
	    STDOUT.flush
	    name = STDIN.readline.chomp
	    if name.length > 0
		break
	    end
	end

	properties = Ice::Application::communicator().getProperties()
	proxyProperty = 'SessionFactory.Proxy'
	proxy = properties.getProperty(proxyProperty)
	if proxy.length == 0
	    puts $0 + ": property `" + proxyProperty + "' not set"
	    return false
	end

	base = Ice::Application::communicator().stringToProxy(proxy)
	factory = Demo::SessionFactoryPrx::checkedCast(base)
	if not factory
	    puts $0 + ": invalid proxy"
	    return false
	end

	session = factory.create(name)
	begin
	    refresh = SessionRefreshThread.new(Ice::Application::communicator().getLogger(), 5, session)
	    refreshThread = Thread.new { refresh.run }

	    hellos = []

	    menu()

	    destroy = true
	    shutdown = false
	    while true
		begin
		    print "==> "
		    STDOUT.flush
		    c = STDIN.readline.chomp
		    if c =~ /^[0-9]+$/
			index = c.to_i
			if index < hellos.length
			    hello = hellos[index]
			    hello.sayHello()
			else
			    puts "Index is too high. " + hellos.length.to_s + " hello objects exist so far.\n" +\
				 "Use `c' to create a new hello object."
			end
		    elsif c == 'c'
			hellos.push(session.createHello())
			puts "Created hello object " + (hellos.length - 1).to_s
		    elsif c == 's'
			destroy = false
			shutdown = true
			break
		    elsif c == 'x'
			break
		    elsif c == 't'
			destroy = false
			break
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
	    #
	    # The refresher thread must be terminated before destroy is
	    # called, otherwise it might get ObjectNotExistException. refresh
	    # is set to 0 so that if session->destroy() raises an exception
	    # the thread will not be re-terminated and re-joined.
	    #
	    refresh.terminate
	    refreshThread.join
	    refresh = nil

	    if destroy
		session.destroy()
	    end
	    if shutdown
		factory.shutdown()
	    end
	ensure
	    #
	    # The refresher thread must be terminated in the event of a
	    # failure.
	    #
	    if refresh
		refresh.terminate
		refreshThread.join
	    end
	end

	return true
    end

    def menu
	print <<MENU
usage:
c:     create a new per-client hello object
0-9:   send a greeting to a hello object
s:     shutdown the server and exit
x:     exit
t:     exit without destroying the session
?:     help
MENU
    end
end

app = Client.new
exit(app.main(ARGV, "config.client"))
