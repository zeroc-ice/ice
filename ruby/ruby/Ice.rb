# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

require 'IceRuby'
require 'thread'

module Ice

    #
    # Convenience function for locating the directory containing the
    # Slice files.
    #
    def Ice.getSliceDir
        #
        # Get the parent of the directory containing this file
        # (Ice.rb).
        #
        rbHome = File::join(File::dirname(__FILE__), "..")

        #
        # For an installation from a source distribution, a binary
        # tarball, or a Windows installer, the "slice" directory is a
        # sibling of the "rb" directory.
        #
        dir = File::join(rbHome, "slice")
        if File::exists?(dir)
            return File::expand_path(dir)
        end

        #
        # In a source distribution, the "slice" directory is one level
        # higher.
        #
        dir = File::join(rbHome, "..", "slice")
        if File::exists?(dir)
            return File::expand_path(dir)
        end

        if RUBY_PLATFORM =~ /linux/i
            #
            # Check the default Linux location.
            #
            dir = File::join("/", "usr", "share", "ice", "slice")
            if File::exists?(dir)
                return dir
            end
        end

        return nil
    end

    #
    # Exceptions.
    #
    class Exception < ::StandardError
        def ice_name
            to_s[2..-1]
        end

        def ice_id
            to_s
        end

        def inspect
            return ::Ice::__stringifyException(self)
        end
    end

    class UserException < Exception
    end

    class LocalException < Exception
    end

    #
    # Ice::Object
    #
    T_Value = Ice.__declareClass('::Ice::Object')
    T_ObjectPrx = Ice.__declareProxy('::Ice::Object')

    #
    # Provide some common functionality for structs
    #
    module Inspect_mixin
        def inspect
            ::Ice::__stringify(self, self.class::ICE_TYPE)
        end
    end

    #
    # Provide some common functionality for proxy classes
    #
    module Proxy_mixin
        module ClassMethods
            def inspect
                ::Ice::__stringify(self, self.class::ICE_TYPE)
            end

            def ice_staticId()
                self::ICE_ID
            end

            def checkedCast(proxy, facetOrContext=nil, context=nil)
                ice_checkedCast(proxy, self::ICE_ID, facetOrContext, context)
            end

            def uncheckedCast(proxy, facet=nil)
                ice_uncheckedCast(proxy, facet)
            end
        end

        def self.included(base)
            base.extend(ClassMethods)
        end
    end

    #
    # Base class for Value types
    #
    class Value
        def inspect
            ::Ice::__stringify(self, self.class::ICE_TYPE)
        end

        def ice_id()
            self.class::ICE_ID
        end

        def Value.ice_staticId()
            self::ICE_ID
        end

        def ice_getSlicedData()
            return _ice_slicedData
        end

        attr_accessor :_ice_slicedData  # Only used for instances of preserved classes.
    end

    T_Value.defineClass(Value, -1, false, false, nil, [])

    T_ObjectPrx.defineProxy(ObjectPrx, nil, [])

    class InterfaceByValue < Value
        def initialize(id)
            @id = id
        end

        def ice_id
            @id
        end
    end

    #
    # UnknownSlicedValue.
    #
    class UnknownSlicedValue < Value
        def ice_id
            return @unknownTypeId
        end
    end
    T_UnknownSlicedValue = Ice.__declareClass('::Ice::UnknownSlicedValue')
    T_UnknownSlicedValue.defineClass(UnknownSlicedValue, -1, true, false, T_Value, [])

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
    # SlicedData
    #
    class SlicedData
        attr_accessor :slices   # array of SliceInfo
    end

    #
    # SliceInfo
    #
    class SliceInfo
        attr_accessor :typeId, :compactId, :bytes, :instances, :hasOptionalMembers, :isLastSlice
    end

    class FormatType
        include Comparable

        def initialize(val)
            fail("invalid value #{val} for FormatType") unless(val >= 0 and val < 3)
            @val = val
        end

        def FormatType.from_int(val)
            raise IndexError, "#{val} is out of range 0..2" if(val < 0 || val > 2)
            @@_values[val]
        end

        def to_s
            @@_names[@val]
        end

        def to_i
            @val
        end

        def <=>(other)
            other.is_a?(FormatType) or raise ArgumentError, "value must be a FormatType"
            @val <=> other.to_i
        end

        def hash
            @val.hash
        end

        def inspect
            @@_names[@val] + "(#{@val})"
        end

        def FormatType.each(&block)
            @@_values.each(&block)
        end

        @@_names = ['DefaultFormat', 'CompactFormat', 'SlicedFormat']
        @@_values = [FormatType.new(0), FormatType.new(1), FormatType.new(2)]

        DefaultFormat = @@_values[0]
        CompactFormat = @@_values[1]
        SlicedFormat = @@_values[2]

        private_class_method :new
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
require 'Ice/Communicator.rb'
require 'Ice/EndpointTypes.rb'
require 'Ice/LocalException.rb'
require 'Ice/Locator.rb'
require 'Ice/Logger.rb'
require 'Ice/ObjectFactory.rb'
require 'Ice/ValueFactory.rb'
require 'Ice/Process.rb'
require 'Ice/Router.rb'
require 'Ice/Connection.rb'
require 'Ice/Version.rb'
require 'Ice/Instrumentation.rb'
require 'Ice/Metrics.rb'
require 'Ice/RemoteLogger.rb'

module Ice
    #
    # Reopen Identity to add the <=> method.
    #
    class Identity
        def <=>(other)
            n = self.name <=> other.name
            if n == 0
                return self.category <=> other.category
            else
                return n
            end
        end
    end

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
            @callback = nil

            @read, @write = IO.pipe

            #
            # Setup and install signal handlers
            #
            if Signal.list.has_key?('HUP')
                Signal.trap('HUP') { @write.puts 'HUP' }
            end
            Signal.trap('INT') { @write.puts 'INT' }
            Signal.trap('TERM') { @write.puts 'TERM' }

            @thr = Thread.new { main }
        end

        # Read and dispatch signals.
        def main
            while rs = IO.select([@read])
                signal = rs.first[0].gets.strip
                if signal == 'DONE'
                    @read.close()
                    break
                end
                callback = @callback
                if callback
                    callback.call(signal)
                end
            end
        end

        # Destroy the object. Wait for the thread to terminate and cleanup
        # the internal state.
        def destroy
            @write.puts 'DONE'
            @write.close()

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

        @@_self = nil
    end

    #
    # Ice::Application.
    #
    class Application
        def initialize(signalPolicy=HandleSignals)
            @@_signalPolicy = signalPolicy
        end

        def main(args, configFile=nil, initData=nil)
            if @@_communicator
                Ice::getProcessLogger().error($0 + ": only one instance of the Application class can be used")
                return false
            end
            if @@_signalPolicy == HandleSignals
                @@_ctrlCHandler = CtrlCHandler.new
            end

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
                initData.properties = Ice::createProperties(args, initData.properties)
                @@_appName = initData.properties.getPropertyWithDefault("Ice.ProgramName", @@_appName)
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
                if @@_signalPolicy == HandleSignals
                    Application::destroyOnInterrupt
                end

                status = run(args)
            rescue => ex
                Ice::getProcessLogger().error($!.inspect + "\n" + ex.backtrace.join("\n"))
                status = 1
            end

            #
            # Don't want any new interrupt and at this point (post-run),
            # it would not make sense to release a held signal to run
            # shutdown or destroy.
            #
            if @@_signalPolicy == HandleSignals
                Application::ignoreInterrupt
            end

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
                @@_communicator.destroy()
                @@_communicator = nil
            end

            if @@_signalPolicy == HandleSignals
                @@_ctrlCHandler.destroy()
                @@_ctrlCHandler = nil
            end

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
            if @@_signalPolicy == HandleSignals
                @@_mutex.synchronize {
                    if @@_ctrlCHandler.getCallback == @@_holdInterruptCallbackProc
                        @@_released = true
                        @@_condVar.signal
                    end
                    @@_ctrlCHandler.setCallback(@@_destroyOnInterruptCallbackProc)
                }
            else
                Ice::getProcessLogger().error(@@_appName + ": warning: interrupt method called on Application configured to not handle interrupts.")
            end
        end

        # No support for this since no server side in Ice for ruby.
        #def Application.shutdownOnInterrupt
        #end

        def Application.ignoreInterrupt
            if @@_signalPolicy == HandleSignals
                @@_mutex.synchronize {
                    if @@_ctrlCHandler.getCallback == @@_holdInterruptCallbackProc
                        @@_released = true
                        @@_condVar.signal
                    end
                    @@_ctrlCHandler.setCallback(nil)
                }
            else
                Ice::getProcessLogger().error(@@_appName + ": warning: interrupt method called on Application configured to not handle interrupts.")
            end
        end

        def Application.callbackOnInterrupt()
            if @@_signalPolicy == HandleSignals
                @@_mutex.synchronize {
                    if @@_ctrlCHandler.getCallback == @@_holdInterruptCallbackProc
                        @@_released = true
                        @@_condVar.signal
                    end
                    @@_ctrlCHandler.setCallback(@@_callbackOnInterruptCallbackProc)
                }
            else
                Ice::getProcessLogger().error(@@_appName + ": warning: interrupt method called on Application configured to not handle interrupts.")
            end
        end

        def Application.holdInterrupt
            if @@_signalPolicy == HandleSignals
                @@_mutex.synchronize {
                    if @@_ctrlCHandler.getCallback != @@_holdInterruptCallbackProc
                        @@_previousCallback = @@_ctrlCHandler.getCallback
                        @@_released = false
                        @@_ctrlCHandler.setCallback(@@_holdInterruptCallbackProc)
                    end
                    # else, we were already holding signals
                }
            else
                Ice::getProcessLogger().error(@@_appName + ": warning: interrupt method called on Application configured to not handle interrupts.")
            end
        end

        def Application.releaseInterrupt
            if @@_signalPolicy == HandleSignals
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
            else
                Ice::getProcessLogger().error(@@_appName + ": warning: interrupt method called on Application configured to not handle interrupts.")
            end
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

            @@_communicator.destroy()

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
                Ice::getProcessLogger().error($!.inspect + "\n" + @@_appName + " (while interrupting in response to signal " + sig + "):\n" + ex.backtrace.join("\n"))
            end
            @@_mutex.synchronize {
                @@_callbackInProcess = false
                @@_condVar.signal
            }
        end

        HandleSignals = 0
        NoSignalHandling = 1

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
        @@_signalPolicy = HandleSignals
    end

    #
    # Proxy comparison functions.
    #
    def Ice.proxyIdentityCompare(lhs, rhs)
        if (lhs && !lhs.is_a?(ObjectPrx)) || (rhs && !rhs.is_a?(ObjectPrx))
            raise TypeError, 'argument is not a proxy'
        end
        if lhs.nil? && rhs.nil?
            return 0
        elsif lhs.nil? && rhs
            return -1
        elsif lhs && rhs.nil?
            return 1
        else
            return lhs.ice_getIdentity() <=> rhs.ice_getIdentity()
        end
    end

    def Ice.proxyIdentityEqual(lhs, rhs)
        return proxyIdentityCompare(lhs, rhs) == 0
    end

    def Ice.proxyIdentityAndFacetCompare(lhs, rhs)
        n = proxyIdentityCompare(lhs, rhs)
        if n == 0 && lhs && rhs
            n = lhs.ice_getFacet() <=> rhs.ice_getFacet()
        end
        return n
    end

    def Ice.proxyIdentityAndFacetEqual(lhs, rhs)
        return proxyIdentityAndFacetCompare(lhs, rhs) == 0
    end

    Protocol_1_0 = ProtocolVersion.new(1, 0)
    Encoding_1_0 = EncodingVersion.new(1, 0)
    Encoding_1_1 = EncodingVersion.new(1, 1)
end
