%{
**********************************************************************

Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef CI < Test.C
    methods
        function ice_preMarshal(obj)
            obj.preMarshalInvoked = true;
        end

        function ice_postUnmarshal(obj)
            obj.postUnmarshalInvoked = true;
        end
    end
end
