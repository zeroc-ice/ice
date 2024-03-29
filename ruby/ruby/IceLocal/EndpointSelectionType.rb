# encoding: utf-8
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#
#
# Ice version 3.7.10
#
# <auto-generated>
#
# Generated from file `EndpointSelectionType.ice'
#
# Warning: do not edit this file.
#
# </auto-generated>
#

require 'Ice'

module ::Ice

    if not defined?(::Ice::EndpointSelectionType)
        class EndpointSelectionType
            include Comparable

            def initialize(name, value)
                @name = name
                @value = value
            end

            def EndpointSelectionType.from_int(val)
                @@_enumerators[val]
            end

            def to_s
                @name
            end

            def to_i
                @value
            end

            def <=>(other)
                other.is_a?(EndpointSelectionType) or raise ArgumentError, "value must be a EndpointSelectionType"
                @value <=> other.to_i
            end

            def hash
                @value.hash
            end

            def EndpointSelectionType.each(&block)
                @@_enumerators.each_value(&block)
            end

            Random = EndpointSelectionType.new("Random", 0)
            Ordered = EndpointSelectionType.new("Ordered", 1)

            @@_enumerators = {0=>Random, 1=>Ordered}

            def EndpointSelectionType._enumerators
                @@_enumerators
            end

            private_class_method :new
        end

        T_EndpointSelectionType = ::Ice::__defineEnum('::Ice::EndpointSelectionType', EndpointSelectionType, EndpointSelectionType::_enumerators)
    end
end
