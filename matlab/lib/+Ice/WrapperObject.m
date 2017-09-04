%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef (Abstract) WrapperObject < handle
    methods
        function self = WrapperObject(impl)
            self.impl = impl;
        end
        function delete(self)
            if ~isempty(self.impl)
                Ice.Util.callMethod(self, '_release');
            end
        end
    end
    properties(Hidden,SetAccess=protected)
        impl
    end
end
