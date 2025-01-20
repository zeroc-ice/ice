%
%  Copyright (c) ZeroC, Inc.
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
