%
%  Copyright (c) ZeroC, Inc.
%

classdef CB3I < LocalTest.CB3
    methods
        function ice_postUnmarshal(obj)
            obj.postUnmarshalInvoked = true;
        end
    end
    properties
        postUnmarshalInvoked = false
    end
end
