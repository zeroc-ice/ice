%
%  Copyright (c) ZeroC, Inc.
%

classdef CB6I < LocalTest.CB6
    methods
        function ice_postUnmarshal(obj)
            obj.postUnmarshalInvoked = true;
        end
    end
    properties
        postUnmarshalInvoked = false
    end
end
