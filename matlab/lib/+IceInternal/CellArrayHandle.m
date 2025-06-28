classdef (Hidden) CellArrayHandle < handle
    % Copyright (c) ZeroC, Inc.

    properties
        array
    end
    methods
        function set(obj, i, v)
            obj.array{i} = v;
        end
    end
end
