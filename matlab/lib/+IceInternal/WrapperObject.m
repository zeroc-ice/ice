%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef (Abstract) WrapperObject < handle
    methods
        function obj = WrapperObject(impl)
            obj.impl = impl;
        end
        function delete(obj)
            if ~isempty(obj.impl)
                Ice.Util.callMethod(obj, '_release');
            end
        end
    end
    properties(Hidden,SetAccess=protected)
        impl
    end
end
