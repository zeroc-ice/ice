#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

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
        if File::exist?(dir)
            return File::expand_path(dir)
        end

        #
        # In a source distribution, the "slice" directory is one level
        # higher.
        #
        dir = File::join(rbHome, "..", "slice")
        if File::exist?(dir)
            return File::expand_path(dir)
        end

        if RUBY_PLATFORM =~ /linux/i
            #
            # Check the default Linux location.
            #
            dir = File::join("/", "usr", "share", "ice", "slice")
            if File::exist?(dir)
                return dir
            end
        end

        return nil
    end

    #
    # Exceptions.
    #
    class Exception < ::StandardError
    end

    class LocalException < Exception
    end

    class UserException < Exception
        def ice_id
            to_s
        end

        def inspect
            return ::Ice::__stringifyException(self)
        end
    end

    #
    # Ice::Value
    #
    T_Value = Ice.__declareClass('::Ice::Object')

    #
    # Object proxy
    #
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

    T_Value.defineClass(Value, -1, false, nil, [])

    T_ObjectPrx.defineProxy(ObjectPrx, nil, [])

    #
    # UnknownSlicedValue.
    #
    class UnknownSlicedValue < Value
        def ice_id
            return @unknownTypeId
        end
    end
    T_UnknownSlicedValue = Ice.__declareClass('::Ice::UnknownSlicedValue')
    T_UnknownSlicedValue.defineClass(UnknownSlicedValue, -1, false, T_Value, [])

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
end

#
# Pull in other files.
#

require_relative 'Ice/BuiltinSequences.rb'
require_relative 'Ice/Context.rb'
require_relative 'Ice/ToStringMode.rb'
require_relative 'Ice/EndpointSelectionType.rb'
require_relative 'Ice/Connection.rb'
require_relative 'Ice/Version.rb'
require_relative 'Ice/Endpoint.rb'
require_relative 'Ice/EndpointTypes.rb'
require_relative 'Ice/LocalExceptions.rb'
require_relative 'Ice/OperationMode.rb'
require_relative 'Ice/Locator.rb'
require_relative 'Ice/Process.rb'
require_relative 'Ice/Router.rb'
require_relative 'Ice/Metrics.rb'
require_relative 'Ice/RemoteLogger.rb'

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
