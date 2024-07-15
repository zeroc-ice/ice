# Copyright (c) ZeroC, Inc.

module Ice

    class EndpointSelectionType
        include Comparable

        def initialize(name, value)
            @name = name
            @value = value
        end

        def self.from_int(val)
            @@_enumerators[val]
        end

        def to_s
            @name
        end

        def to_i
            @value
        end

        def <=>(other)
            other.is_a?(EndpointSelectionType) or raise ArgumentError, "value must be an EndpointSelectionType"
            @value <=> other.to_i
        end

        def hash
            @value.hash
        end

        def self.each(&block)
            @@_enumerators.each_value(&block)
        end

        Random = EndpointSelectionType.new("Random", 0)
        Ordered = EndpointSelectionType.new("Ordered", 1)

        @@_enumerators = {0=>Random, 1=>Ordered}

        def self._enumerators
            @@_enumerators
        end

        private_class_method :new
    end
end
