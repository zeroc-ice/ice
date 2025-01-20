%
%  Copyright (c) ZeroC, Inc.
%

classdef CB1I < LocalTest.CB1
    methods
        function ice_postUnmarshal(obj)
            obj.postUnmarshalInvoked = true;
        end
    end
    properties
        postUnmarshalInvoked = false
    end
end
