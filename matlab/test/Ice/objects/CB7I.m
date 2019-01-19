%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

classdef CB7I < LocalTest.CB7
    methods
        function ice_postUnmarshal(obj)
            obj.postUnmarshalInvoked = true;
        end
    end
    properties
        postUnmarshalInvoked = false
    end
end
