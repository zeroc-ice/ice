%{
**********************************************************************

Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

**********************************************************************
%}

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
