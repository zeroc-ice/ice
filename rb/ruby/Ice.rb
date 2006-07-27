# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

require 'IceRuby'
require 'thread'

module Ice
    #
    # Exceptions.
    #
    class Exception < ::StandardError
	def ice_name
	    to_s
	end
    end

    class UserException < Exception
    end

    class LocalException < Exception
    end

    #
    # Object.
    #
    T_Object = Ice.__declareClass('::Ice::Object')
    T_ObjectPrx = Ice.__declareProxy('::Ice::Object')

    module Object_mixin
	def ice_isA(id, current=nil)
	    return ice_ids().include?(id)
	end

	def ice_ping(current=nil)
	end
    end

    T_Object.defineClass(nil, true, nil, [], [])
    T_ObjectPrx.defineProxy(ObjectPrx, T_Object)

    #
    # LocalObject.
    #
    T_LocalObject = Ice.__declareClass('::Ice::LocalObject')
    T_LocalObject.defineClass(nil, true, nil, [], [])

    #
    # InitializationData.
    #
    class InitializationData
	def initialize(properties=nil, logger=nil, defaultContext=nil)
	    @properties = properties
	    @logger = logger
	    @defaultContext = defaultContext
	end

	attr_accessor :properties, :logger, :defaultContext
    end

    #
    # Slice checksum dictionary.
    #
    SliceChecksums = {}
end

#
# Include certain generated files.
#
require 'Ice/BuiltinSequences.rb'
require 'Ice/Current.rb'
require 'Ice/Endpoint.rb'
require 'Ice/LocalException.rb'
require 'Ice/Locator.rb'
require 'Ice/Logger.rb'
require 'Ice/ObjectFactory.rb'
require 'Ice/Process.rb'
require 'Ice/Router.rb'

module Ice
    #
    # Ice::Application.
    #
    class Application
	def initialize
	    if self.instance_of?(Application):
		raise RuntimeError, "Ice.Application is an abstract class"
	    end
	end

	def main(args, configFile=nil, initData=nil)
	    if @@_communicator
		print $0 + ": only one instance of the Application class can be used"
		return false
	    end

	    @@_interrupted = false
	    @@_appName = $0

	    #
	    # Install our handler for the signals we are interested in. We assume main()
	    # is called from the main thread.
	    #
	    if Signal.list.has_key?('HUP')
		Signal.trap('HUP') { Application::signalHandler('HUP') }
	    end
	    Signal.trap('INT') { Application::signalHandler('INT') }
	    Signal.trap('TERM') { Application::signalHandler('TERM') }

	    status = 0

	    begin
		if initData.nil?
		    initData = InitializationData.new
		end
		if configFile
		    initData.properties = Ice::createProperties
		    initData.properties.load(configFile)
		end
		@@_communicator = Ice::initialize(args, initData)

		#
		# Used by destroyOnInterruptCallback and shutdownOnInterruptCallback.
		#
		@@_nohup = @@_communicator.getProperties().getPropertyAsInt("Ice.Nohup") > 0

		#
		# The default is to destroy when a signal is received.
		#
		Application::destroyOnInterrupt

		status = run(args)
	    rescue => ex
		puts $!
		puts ex.backtrace.join("\n")
		status = 1
	    end

	    if @@_communicator
		#
		# We don't want to handle signals anymore.
		#
		Application::ignoreInterrupt

		begin
		    @@_communicator.destroy()
		rescue => ex
		    puts $!
		    puts ex.backtrace.join("\n")
		    status = 1
		end

		@@_communicator = nil
	    end

	    return status
	end

	def run(args)
	    raise RuntimeError, 'run() not implemented'
	end

	def Application.appName
	    @@_appName
	end

	def Application.communicator
	    @@_communicator
	end

	def Application.destroyOnInterrupt
	    @@_mutex.synchronize {
		if @@_ctrlCHandler == @@_holdInterruptCallbackProc
		    @@_released = true
		    @@_ctrlCHandler = @@_destroyOnInterruptCallbackProc
		    @@_condVar.signal
		else
		    @@_ctrlCHandler = @@_destroyOnInterruptCallbackProc
		end
	    }
	end

	def Application.shutdownOnInterrupt
	    @@_mutex.synchronize {
		if @@_ctrlCHandler == @@_holdInterruptCallbackProc
		    @@_released = true
		    @@_ctrlCHandler = @@_shutdownOnInterruptCallbackProc
		    @@_condVar.signal
		else
		    @@_ctrlCHandler = @@_shutdownOnInterruptCallbackProc
		end
	    }
	end

	def Application.ignoreInterrupt
	    @@_mutex.synchronize {
		if @@_ctrlCHandler == @@_holdInterruptCallbackProc
		    @@_released = true
		    @@_ctrlCHandler = nil
		    @@_condVar.signal
		else
		    @@_ctrlCHandler = nil
		end
	    }
	end

	def Application.holdInterrupt
	    @@_mutex.synchronize {
		if @@_ctrlCHandler != @@_holdInterruptCallbackProc
		    @@_previousCallback = @@_ctrlCHandler
		    @@_released = false
		    @@_ctrlCHandler = @@_holdInterruptCallbackProc
		end
		# else, we were already holding signals
	    }
	end

	def Application.releaseInterrupt
	    @@_mutex.synchronize {
		if @@_ctrlCHandler == @@_holdInterruptCallbackProc
		    #
		    # Note that it's very possible no signal is held;
		    # in this case the callback is just replaced and
		    # setting _released to true and signalling _condVar
		    # do no harm.
		    #
		    @@_released = true
		    @@_ctrlCHandler = @@_previousCallback
		    @@_condVar.signal
		end
		# Else nothing to release.
	    }
	end

	def Application.interrupted
	    @@_mutex.synchronize {
		return @@_interrupted
	    }
	end

	def Application.signalHandler(sig)
	    if @@_ctrlCHandler
		@@_ctrlCHandler.call(sig)
	    end
	end

	def Application.holdInterruptCallback(sig)
	    @@_mutex.synchronize {
		while not @@_released
		    @@_condVar.wait(@@_mutex)
		end
	    }

	    #
	    # Use the current callback to process this (old) signal.
	    #
	    if @@_ctrlCHandler
		@@_ctrlCHandler.call(sig)
	    end
	end

	def Application.destroyOnInterruptCallback(sig)
	    if @@_nohup and sig == 'HUP'
		return
	    end

	    @@_mutex.synchronize {
		@@_interrupted = true
	    }

	    begin
		@@_communicator.destroy()
	    rescue => ex
		puts $!
		puts @@_appName + " (while destroying in response to signal " + sig + "):"
		puts ex.backtrace.join("\n")
	    end
	end

	def Application.shutdownOnInterruptCallback(sig)
	    if @@_nohup and sig == 'HUP'
		return
	    end

	    @@_mutex.synchronize {
		@@_interrupted = true
	    }

	    begin
		@@_communicator.shutdown()
	    rescue => ex
		puts $!
		puts @@_appName + " (while shutting down in response to signal " + sig + "):"
		puts ex.backtrace.join("\n")
	    end
	end

	@@_appName = nil
	@@_communicator = nil
	@@_interrupted = false
	@@_released = false
	@@_mutex = Mutex.new
	@@_condVar = ConditionVariable.new
	@@_ctrlCHandler = nil
	@@_previousCallback = nil
	@@_holdInterruptCallbackProc = Proc.new { |sig| Application::holdInterruptCallback(sig) }
	@@_destroyOnInterruptCallbackProc = Proc.new { |sig| Application::destroyOnInterruptCallback(sig) }
	@@_shutdownOnInterruptCallbackProc = Proc.new { |sig| Application::shutdownOnInterruptCallback(sig) }
    end

    #
    # Proxy comparison functions.
    #
    def Ice.proxyIdentityEqual(lhs, rhs)
	if (lhs && !lhs.is_a?(ObjectPrx)) || (rhs && !rhs.is_a?(ObjectPrx))
	    raise TypeError, 'argument is not a proxy'
	end
	if lhs.nil? && rhs.nil?
	    return true
	elsif lhs.nil? && rhs
	    return false
	elsif lhs && rhs.nil?
	    return false
	else
	    return lhs.ice_getIdentity() == rhs.ice_getIdentity()
	end
    end

    def Ice.proxyIdentityAndFacetEqual(lhs, rhs)
	if (lhs && !lhs.is_a?(ObjectPrx)) || (rhs && !rhs.is_a?(ObjectPrx))
	    raise TypeError, 'argument is not a proxy'
	end
	if lhs.nil? && rhs.nil?
	    return true
	elsif lhs.nil? && rhs
	    return false
	elsif lhs && rhs.nil?
	    return false
	else
	    return lhs.ice_getIdentity() == rhs.ice_getIdentity() && lhs.ice_getFacet() == rhs.ice_getFacet()
	end
    end
end

Ice::Object_mixin::OP_ice_isA = ::Ice::__defineOperation('ice_isA', ::Ice::OperationMode.Nonmutating, false, [::Ice::T_string], [], ::Ice::T_bool, [])
Ice::Object_mixin::OP_ice_ping = ::Ice::__defineOperation('ice_ping', ::Ice::OperationMode.Nonmutating, false, [], [], nil, [])
Ice::Object_mixin::OP_ice_ids = ::Ice::__defineOperation('ice_ids', ::Ice::OperationMode.Nonmutating, false, [], [], ::Ice::T_StringSeq, [])
Ice::Object_mixin::OP_ice_id = ::Ice::__defineOperation('ice_id', ::Ice::OperationMode.Nonmutating, false, [], [], ::Ice::T_string, [])
