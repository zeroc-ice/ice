# encoding: utf-8
#
# Copyright (c) ZeroC, Inc.

module Ice
    class ToStringMode
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
            raise ArgumentError, "value must be a ToStringMode" unless other.is_a?(ToStringMode)
            @value <=> other.to_i
        end

        def hash
            @value.hash
        end

        def self.each(&block)
            @@_enumerators.each_value(&block)
        end

        Unicode = ToStringMode.new("Unicode", 0)
        ASCII = ToStringMode.new("ASCII", 1)
        Compat = ToStringMode.new("Compat", 2)

        @@_enumerators = { 0 => Unicode, 1 => ASCII, 2 => Compat }

        def self._enumerators
            @@_enumerators
        end

        private_class_method :new
    end
end
