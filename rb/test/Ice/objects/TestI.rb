#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

class BI < Test::B
    def initialize
        @preMarshalInvoked = false
        @_postUnmarshalInvoked = false
    end

    def postUnmarshalInvoked(current=nil)
        return @_postUnmarshalInvoked
    end

    def ice_preMarshal
        @preMarshalInvoked = true
    end

    def ice_postUnmarshal
        @_postUnmarshalInvoked = true
    end
end

class CI < Test::C
    def initialize
        @preMarshalInvoked = false
        @_postUnmarshalInvoked = false
    end

    def postUnmarshalInvoked(current=nil)
        return @_postUnmarshalInvoked
    end

    def ice_preMarshal
        @preMarshalInvoked = true
    end

    def ice_postUnmarshal
        @_postUnmarshalInvoked = true
    end
end

#
# Re-open Test::D instead of specializing it.
#
class Test::D
    def initialize
        @preMarshalInvoked = false
        @_postUnmarshalInvoked = false
    end

    def postUnmarshalInvoked(current=nil)
        return @_postUnmarshalInvoked
    end

    def ice_preMarshal
        @preMarshalInvoked = true
    end

    def ice_postUnmarshal
        @_postUnmarshalInvoked = true
    end
end

