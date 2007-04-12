#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
    Object_mixin::ICE_TYPE = T_Object

    T_ObjectPrx.defineProxy(ObjectPrx, T_Object)
    ObjectPrx::ICE_TYPE = T_ObjectPrx

    #
    # LocalObject.
    #
    T_LocalObject = Ice.__declareClass('::Ice::LocalObject')
    T_LocalObject.defineClass(nil, true, nil, [], [])

    #
    # InitializationData.
    #
    class InitializationData
        def initialize(properties=nil, logger=nil)
            @properties = properties
            @logger = logger
        end

        attr_accessor :properties, :logger
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
    # Note the interface is the same as the C++ CtrlCHandler
    # implementation, however, the implementation is different.
    #
    class CtrlCHandler
        def initialize
            if @@_self != nil
                raise RuntimeError, "Only a single instance of a CtrlCHandler can be instantiated."
            end
            @@_self = self

            # State variables. These are not class static variables.
            @condVar = ConditionVariable.new
            @mutex = Mutex.new
            @queue = Array.new
            @done = false
            @callback = nil

            #
            # Setup and install signal handlers
            #
            if Signal.list.has_key?('HUP')
                Signal.trap('HUP') { signalHandler('HUP') }
            end
            Signal.trap('INT') { signalHandler('INT') }
            Signal.trap('TERM') { signalHandler('TERM') }

            @thr = Thread.new { main }
        end

        # Dequeue and dispatch signals.
        def main
            while true
                sig, callback = @mutex.synchronize {
                    while @queue.empty? and not @done
                        @condVar.wait(@mutex)
                    end
                    if @done
                        return
                    end
                    @queue.shift
                }
                if callback
                    callback.call(sig)
                end
            end
        end

        # Destroy the object. Wait for the thread to terminate and cleanup
        # the internal state.
        def destroy
            @mutex.synchronize {
                @done = true
                @condVar.signal
            }

            # Wait for the thread to terminate
            @thr.join

            #
            # Cleanup any state set by the CtrlCHandler.
            #
            if Signal.list.has_key?('HUP')
                Signal.trap('HUP', 'SIG_DFL')
            end
            Signal.trap('INT', 'SIG_DFL')
            Signal.trap('TERM', 'SIG_DFL')
            @@_self = nil
        end

        def setCallback(callback)
            @mutex.synchronize {
                @callback = callback
            }
        end

        def getCallback
            @mutex.synchronize {
                return @callback
            }
        end

        # Private. Only called by the signal handling mechanism.
        def signalHandler(sig)
            @mutex.synchronize {
                #
                # The signal AND the current callback are queued together.
                #
                @queue = @queue.push([sig, @callback])
                @condVar.signal
            }
        end
        @@_self = nil
    end

    #
    # Ice::Application.
    #
    class Application
        
        def main(args, configFile=nil, initData=nil)
            if @@_communicator
                print $0 + ": only one instance of the Application class can be used"
                return false
            end
            @@_ctrlCHandler = CtrlCHandler.new

            @@_interrupted = false
            @@_appName = $0

            status = 0

            begin
                if initData.nil?
                    initData = InitializationData.new
                end
                if configFile
                    initData.properties = Ice::createProperties
                    initData.properties.load(configFile)
                end
                @@_application = self
                @@_communicator = Ice::initialize(args, initData)
                @@_destroyed = false

                #
                # Used by destroyOnInterruptCallback.
                #
                @@_nohup = @@_communicator.getProperties().getPropertyAsInt("Ice.Nohup") > 0

                #
                # The default is to destroy when a signal is received.
                #
                Application::destroyOnInterrupt

                status = run(args)
            rescue => ex
                puts $!.inspect
                puts ex.backtrace.join("\n")
                status = 1
            end

            #
            # Don't want any new interrupt and at this point (post-run),
            # it would not make sense to release a held signal to run
            # shutdown or destroy.
            #
            Application::ignoreInterrupt

            @@_mutex.synchronize {
                while @@_callbackInProgress
                    @@_condVar.wait(@@_mutex)
                end
                if @@_destroyed
                    @@_communicator = nil
                else
                    @@_destroyed = true
                end
                #
                # And _communicator != 0, meaning will be destroyed
                # next, _destroyed = true also ensures that any
                # remaining callback won't do anything
                #
                @@_application = nil
            }

            if @@_communicator
                begin
                    @@_communicator.destroy()
                rescue => ex
                    puts $!
                    puts ex.backtrace.join("\n")
                    status = 1
                end

                @@_communicator = nil
            end

            @@_ctrlCHandler.destroy()
            @@_ctrlCHandler = nil

            return status
        end

        def interruptCallback(sig)
        end

        def Application.appName
            @@_appName
        end

        def Application.communicator
            @@_communicator
        end

        def Application.destroyOnInterrupt
            @@_mutex.synchronize {
                if @@_ctrlCHandler.getCallback == @@_holdInterruptCallbackProc
                    @@_released = true
                    @@_condVar.signal
                end
                @@_ctrlCHandler.setCallback(@@_destroyOnInterruptCallbackProc)
            }
        end

        # No support for this since no server side in Ice for ruby.
        #def Application.shutdownOnInterrupt
        #end

        def Application.ignoreInterrupt
            @@_mutex.synchronize {
                if @@_ctrlCHandler.getCallback == @@_holdInterruptCallbackProc
                    @@_released = true
                    @@_condVar.signal
                end
                @@_ctrlCHandler.setCallback(nil)
            }
        end

        def Application.callbackOnInterrupt()
            @@_mutex.synchronize {
                if @@_ctrlCHandler.getCallback == @@_holdInterruptCallbackProc
                    @@_released = true
                    @@_condVar.signal
                end
                @@_ctrlCHandler.setCallback(@@_callbackOnInterruptCallbackProc)
            }
        end

        def Application.holdInterrupt
            @@_mutex.synchronize {
                if @@_ctrlCHandler.getCallback != @@_holdInterruptCallbackProc
                    @@_previousCallback = @@_ctrlCHandler.getCallback
                    @@_released = false
                    @@_ctrlCHandler.setCallback(@@_holdInterruptCallbackProc)
                end
                # else, we were already holding signals
            }
        end

        def Application.releaseInterrupt
            @@_mutex.synchronize {
                if @@_ctrlCHandler.getCallback == @@_holdInterruptCallbackProc
                    #
                    # Note that it's very possible no signal is held;
                    # in this case the callback is just replaced and
                    # setting _released to true and signalling _condVar
                    # do no harm.
                    #
                    @@_released = true
                    @@_ctrlCHandler.setCallback(@@_previousCallback)
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

        def Application.holdInterruptCallback(sig)
            callback = @@_mutex.synchronize {
                while not @@_released
                    @@_condVar.wait(@@_mutex)
                end
                if @@_destroyed
                    return
                end
                @@_ctrlCHandler.getCallback
            }

            #
            # Use the current callback to process this (old) signal.
            #
            if callback
                callback.call(sig)
            end
        end

        def Application.destroyOnInterruptCallback(sig)
            @@_mutex.synchronize {
                if @@_destroyed or @@_nohup and sig == 'HUP'
                    return
                end
                @@_callbackInProcess = true
                @@_interrupted = true
                @@_destroyed = true
            }

            begin
                @@_communicator.destroy()
            rescue => ex
                puts $!
                puts @@_appName + " (while destroying in response to signal " + sig + "):"
                puts ex.backtrace.join("\n")
            end
            @@_mutex.synchronize {
                @@_callbackInProcess = false
                @@_condVar.signal
            }
        end

        def Application.callbackOnInterruptCallback(sig)
            # For SIGHUP the user callback is always called. It can
            # decide what to do.
            @@_mutex.synchronize {
                if @@_destroyed
                    #
                    # Being destroyed by main thread.
                    #
                    return
                end
                @@_interrupted = true
                @@_callbackInProcess = true
            }

            begin
                @@_application.interruptCallback(sig)
            rescue => ex
                puts $!
                puts @@_appName + " (while interrupting in response to signal " + sig + "):"
                puts ex.backtrace.join("\n")
            end
            @@_mutex.synchronize {
                @@_callbackInProcess = false
                @@_condVar.signal
            }
        end

        @@_appName = nil
        @@_communicator = nil
        @@_application = nil
        @@_ctrlCHandler = nil
        @@_previousCallback = nil
        @@_interrupted = false
        @@_released = false
        @@_destroyed = false
        @@_callbackInProgress = false
        @@_condVar = ConditionVariable.new
        @@_mutex = Mutex.new
        @@_holdInterruptCallbackProc = Proc.new { |sig| Application::holdInterruptCallback(sig) }
        @@_destroyOnInterruptCallbackProc = Proc.new { |sig| Application::destroyOnInterruptCallback(sig) }
        @@_callbackOnInterruptCallbackProc = Proc.new { |sig| Application::callbackOnInterruptCallback(sig) }
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

Ice::Object_mixin::OP_ice_isA = ::Ice::__defineOperation('ice_isA', ::Ice::OperationMode::Idempotent, ::Ice::OperationMode::Nonmutating, false, [::Ice::T_string], [], ::Ice::T_bool, [])
Ice::Object_mixin::OP_ice_ping = ::Ice::__defineOperation('ice_ping', ::Ice::OperationMode::Idempotent, ::Ice::OperationMode::Nonmutating, false, [], [], nil, [])
Ice::Object_mixin::OP_ice_ids = ::Ice::__defineOperation('ice_ids', ::Ice::OperationMode::Idempotent, ::Ice::OperationMode::Nonmutating, false, [], [], ::Ice::T_StringSeq, [])
Ice::Object_mixin::OP_ice_id = ::Ice::__defineOperation('ice_id', ::Ice::OperationMode::Idempotent, ::Ice::OperationMode::Nonmutating, false, [], [], ::Ice::T_string, [])
