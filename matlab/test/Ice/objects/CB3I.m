%
% Copyright (c) ZeroC, Inc. All rights reserved.
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
