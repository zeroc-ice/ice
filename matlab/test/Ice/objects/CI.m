%{
**********************************************************************

Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

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
