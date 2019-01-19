#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

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

class EI < Test::E
    def initialize
        super(1, "hello")
    end

    def checkValues(current=nil)
        return i == 1 && s == "hello"
    end
end

class FI < Test::F
    def initialize(e=nil)
        super(e, e)
    end

    def checkValues(current=nil)
        return e1 != nil && e1 == e2
    end
end
