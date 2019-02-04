%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

classdef CB4I < LocalTest.CB4
    methods
        function ice_postUnmarshal(obj)
            obj.postUnmarshalInvoked = true;
        end
    end
    properties
        postUnmarshalInvoked = false
    end
end
