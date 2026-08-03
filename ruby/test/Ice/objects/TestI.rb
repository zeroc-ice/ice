# Copyright (c) ZeroC, Inc.

class BI < Test::B
    def initialize
        @preMarshalInvoked = false
        @postUnmarshalInvoked = false
    end

    def ice_preMarshal
        @preMarshalInvoked = true
    end

    def ice_postUnmarshal
        @postUnmarshalInvoked = true
    end
end

class CI < Test::C
    def initialize
        @preMarshalInvoked = false
        @postUnmarshalInvoked = false
    end

    def ice_preMarshal
        @preMarshalInvoked = true
    end

    def ice_postUnmarshal
        @postUnmarshalInvoked = true
    end
end

#
# Re-open Test::D instead of specializing it.
#
class Test::D
    def initialize
        @preMarshalInvoked = false
        @postUnmarshalInvoked = false
    end

    def ice_preMarshal
        @preMarshalInvoked = true
    end

    def ice_postUnmarshal
        @postUnmarshalInvoked = true
    end
end

class CustomSliceLoader
    @@compactDuringUnmarshal = false

    def self.compactDuringUnmarshal=(value)
        @@compactDuringUnmarshal = value
    end

    def newInstance(typeId)
        # Compact the GC heap in the middle of unmarshaling the enclosing request results.
        GC.verify_compaction_references(expand_heap: true, toward: :empty) if @@compactDuringUnmarshal

        case typeId
        when '::Test::B'
            return BI.new
        when '::Test::C'
            return CI.new
        else
            return nil
        end
    end
end
