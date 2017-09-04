%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef (Abstract) Exception < MException
    methods(Abstract)
        ice_id(obj)
    end
    methods
        function self = Exception(id, msg)
            self = self@MException(id, msg)
        end
    end
end
