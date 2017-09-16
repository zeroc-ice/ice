%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef II < Ice.InterfaceByValue
    methods
        function obj = II()
            obj = obj@Ice.InterfaceByValue(test.Ice.objects.Test.IPrx.ice_staticId());
        end
    end
end
