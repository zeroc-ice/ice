%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef Optional
    methods
        function obj = Optional(v)
            if nargin == 0
                obj.isSet = false
            else
                obj.value = v
            end
        end
        function obj = set.value(obj, v)
            if isempty(v)
                obj.value = v
                obj.isSet = false
            else
                obj.value = v
                obj.isSet = true
            end
        end
        function v = get.value(obj)
            if obj.isSet
                v = obj.value
            else
                throw(MException('ICE:IllegalStateException', 'Optional does not have a value'))
            end
        end
        function obj = clear(obj) % Use with v = v.clear()
            obj.value = []
            obj.isSet = false
        end
    end
    properties
        value
    end
    properties(SetAccess = private)
        isSet
    end
end
