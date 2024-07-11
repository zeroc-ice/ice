# Copyright (c) ZeroC, Inc.

module Ice

    class CompressBatch
        include Comparable

        def initialize(name, value)
            @name = name
            @value = value
        end

        def CompressBatch.from_int(val)
            @@_enumerators[val]
        end

        def to_s
            @name
        end

        def to_i
            @value
        end

        def <=>(other)
            other.is_a?(CompressBatch) or raise ArgumentError, "value must be a CompressBatch"
            @value <=> other.to_i
        end

        def hash
            @value.hash
        end

        def CompressBatch.each(&block)
            @@_enumerators.each_value(&block)
        end

        Yes = CompressBatch.new("Yes", 0)
        No = CompressBatch.new("No", 1)
        BasedOnProxy = CompressBatch.new("BasedOnProxy", 2)

        @@_enumerators = {0=>Yes, 1=>No, 2=>BasedOnProxy}

        def CompressBatch._enumerators
            @@_enumerators
        end

        private_class_method :new
    end
end
