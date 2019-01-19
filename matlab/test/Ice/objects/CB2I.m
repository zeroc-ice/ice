%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

classdef CB2I < LocalTest.CB2
    methods
        function ice_postUnmarshal(obj)
            obj.postUnmarshalInvoked = true;
        end
    end
    properties
        postUnmarshalInvoked = false
    end
end
