%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef OpaqueEndpointInfo < Ice.EndpointInfo
    methods
        function obj = OpaqueEndpointInfo(underlying, timeout, compress, rawEncoding, rawBytes)
            if nargin == 0
                underlying = [];
                timeout = 0;
                compress = false;
                rawEncoding = [];
                rawBytes = 0;
            end
            obj = obj@Ice.EndpointInfo(Ice.OpaqueEndpointType.value, false, false, underlying, timeout, compress);
            obj.rawEncoding = rawEncoding;
            obj.rawBytes = rawBytes;
        end
    end
    properties(SetAccess=private)
        rawEncoding
        rawBytes uint8
    end
end
