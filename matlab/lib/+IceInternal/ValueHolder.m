%{
**********************************************************************

Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

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
