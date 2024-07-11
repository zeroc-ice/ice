# Copyright (c) ZeroC, Inc.

module Ice

    class ConnectionClose
        include Comparable

        def initialize(name, value)
            @name = name
            @value = value
        end

        def ConnectionClose.from_int(val)
            @@_enumerators[val]
        end

        def to_s
            @name
        end

        def to_i
            @value
        end

        def <=>(other)
            other.is_a?(ConnectionClose) or raise ArgumentError, "value must be a ConnectionClose"
            @value <=> other.to_i
        end

        def hash
            @value.hash
        end

        def ConnectionClose.each(&block)
            @@_enumerators.each_value(&block)
        end

        Forcefully = ConnectionClose.new("Forcefully", 0)
        Gracefully = ConnectionClose.new("Gracefully", 1)
        GracefullyWithWait = ConnectionClose.new("GracefullyWithWait", 2)

        @@_enumerators = {0=>Forcefully, 1=>Gracefully, 2=>GracefullyWithWait}

        def ConnectionClose._enumerators
            @@_enumerators
        end

        private_class_method :new
    end
end
