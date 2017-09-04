%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef (Abstract) LocalException < Ice.Exception
    methods
        function obj = LocalException(id, msg)
            obj = obj@Ice.Exception(id, msg)
        end
    end
end
