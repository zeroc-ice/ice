%
% Copyright (c) ZeroC, Inc.
%

classdef CellArrayHandle < handle
    properties
        array
    end
    methods
        function set(obj, i, v)
            obj.array{i} = v;
        end
    end
end
