%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

classdef ValueHolder < handle
    properties
        value
    end
    methods
        function set(obj, v)
            obj.value = v;
        end
    end
end
