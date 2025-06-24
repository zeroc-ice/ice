classdef ValueHolder < handle
    % Copyright (c) ZeroC, Inc.

    properties
        value
    end
    methods
        function set(obj, v)
            obj.value = v;
        end
    end
end
