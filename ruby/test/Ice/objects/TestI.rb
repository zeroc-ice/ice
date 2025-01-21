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
