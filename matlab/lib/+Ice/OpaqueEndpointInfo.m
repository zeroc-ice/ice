%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef OpaqueEndpointInfo < Ice.EndpointInfo
    methods
        function obj = OpaqueEndpointInfo(type, underlying, timeout, compress, rawEncoding, rawBytes)
            obj = obj@Ice.EndpointInfo(type, false, false, underlying, timeout, compress);
            obj.rawEncoding = rawEncoding;
            obj.rawBytes = rawBytes;
        end
    end
    properties(SetAccess=private)
        rawEncoding
        rawBytes uint8
    end
end
