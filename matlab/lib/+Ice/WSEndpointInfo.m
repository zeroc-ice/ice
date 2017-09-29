%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef WSEndpointInfo < Ice.EndpointInfo
    methods
        function obj = WSEndpointInfo(secure, underlying, timeout, compress, resource)
            if nargin == 0
                underlying = [];
                timeout = 0;
                compress = false;
                resource = '';
            end
            obj = obj@Ice.EndpointInfo(Ice.WSEndpointType.value, false, secure, underlying, timeout, compress);
            obj.resource = resource;
        end
    end
    properties(SetAccess=private)
        resource char
    end
end
