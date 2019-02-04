%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

classdef CB5I < LocalTest.CB5
    methods
        function ice_postUnmarshal(obj)
            obj.postUnmarshalInvoked = true;
        end
    end
    properties
        postUnmarshalInvoked = false
    end
end
