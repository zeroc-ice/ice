%
% Copyright (c) ZeroC, Inc.
%

classdef CB8I < LocalTest.CB8
    methods
        function ice_postUnmarshal(obj)
            obj.postUnmarshalInvoked = true;
        end
    end
    properties
        postUnmarshalInvoked = false
    end
end
