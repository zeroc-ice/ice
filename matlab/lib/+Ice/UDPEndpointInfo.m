%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef UDPEndpointInfo < Ice.IPEndpointInfo
    methods
        function obj = UDPEndpointInfo(underlying, timeout, compress, host, port, sourceAddress, mcastInterface, ...
                                       mcastTtl)
            if nargin == 0
                underlying = [];
                timeout = 0;
                compress = false;
                host = '';
                port = 0;
                sourceAddress = '';
                mcastInterface = '';
                mcastTtl = 0;
            end
            obj = obj@Ice.IPEndpointInfo(Ice.UDPEndpointType.value, true, false, underlying, timeout, ...
                                         compress, host, port, sourceAddress);
            obj.mcastInterface = mcastInterface;
            obj.mcastTtl = mcastTtl;
        end
    end
    properties(SetAccess=private)
        mcastInterface char
        mcastTtl int32
    end
end
