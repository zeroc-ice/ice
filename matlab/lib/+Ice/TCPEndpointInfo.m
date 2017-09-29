%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef TCPEndpointInfo < Ice.IPEndpointInfo
    methods
        function obj = TCPEndpointInfo(underlying, timeout, compress, host, port, sourceAddress)
            if nargin == 0
                underlying = [];
                timeout = 0;
                compress = false;
                host = '';
                port = 0;
                sourceAddress = '';
            end
            obj = obj@Ice.IPEndpointInfo(Ice.TCPEndpointType.value, false, false, underlying, timeout, ...
                                         compress, host, port, sourceAddress);
        end
    end
end
