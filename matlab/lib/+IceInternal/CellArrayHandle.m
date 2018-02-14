%{
**********************************************************************

Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

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
